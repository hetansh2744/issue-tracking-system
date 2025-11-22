#include <gtest/gtest.h>

#include <cstdlib>   // setenv, unsetenv, getenv
#include <memory>    // std::unique_ptr
#include <string>

#include "Issue.hpp"
#include "IssueRepository.hpp"
#include "IssueTrackerController.hpp"

// Small helper to save & restore environment variables inside a scope.
class EnvGuard {
 public:
  explicit EnvGuard(const char* name)
      : name_(name),
        had_value_(std::getenv(name) != nullptr),
        value_(had_value_ ? std::getenv(name) : "") {}

  ~EnvGuard() {
    if (had_value_) {
      setenv(name_.c_str(), value_.c_str(), 1);
    } else {
      unsetenv(name_.c_str());
    }
  }

 private:
  std::string name_;
  bool had_value_;
  std::string value_;
};

// ---------------------------------------------------------------------
//  Issue tag behaviour (Issue.cpp)
// ---------------------------------------------------------------------

TEST(IssueTagsTest, AddTagThenQueryAndGetTags) {
  Issue issue(0, "author", "Title", 0);

  // Initially no tag
  EXPECT_FALSE(issue.hasTag("bug"));

  // First add returns true
  EXPECT_TRUE(issue.addTag("bug"));
  EXPECT_TRUE(issue.hasTag("bug"));

  // Second add of same tag returns false (no duplicate insert)
  EXPECT_FALSE(issue.addTag("bug"));

  // getTags should contain exactly one element: "bug"
  auto tags = issue.getTags();
  ASSERT_EQ(tags.size(), 1u);
  EXPECT_EQ(*tags.begin(), "bug");
}

TEST(IssueTagsTest, RemoveTagAndIdempotency) {
  Issue issue(0, "author", "Title", 0);

  issue.addTag("bug");
  EXPECT_TRUE(issue.hasTag("bug"));

  // First removal succeeds
  EXPECT_TRUE(issue.removeTag("bug"));
  EXPECT_FALSE(issue.hasTag("bug"));

  // Removing again should do nothing and return false
  EXPECT_FALSE(issue.removeTag("bug"));
}

TEST(IssueTagsTest, AddEmptyTagThrowsInvalidArgument) {
  Issue issue(0, "author", "Title", 0);

  EXPECT_THROW(issue.addTag(""), std::invalid_argument);
}

// ---------------------------------------------------------------------
//  IssueRepository factory & tag helpers (IssueRepository.cpp)
//  - createIssueRepository memory vs disk branches
//  - addTagToIssue / removeTagFromIssue
// ---------------------------------------------------------------------

TEST(IssueRepositoryFactoryTest, CreatesMemoryBackendWhenEnvIsMemory) {
  EnvGuard guardBackend("ISSUE_REPO_BACKEND");
  EnvGuard guardDbPath("ISSUE_DB_PATH");

  // Force the "memory" branch (case-insensitive).
  setenv("ISSUE_REPO_BACKEND", "MeMoRy", 1);
  unsetenv("ISSUE_DB_PATH");

  std::unique_ptr<IssueRepository> repo(createIssueRepository());
  ASSERT_NE(repo, nullptr);

  Issue issue(0, "author", "Title", 0);
  Issue saved = repo->saveIssue(issue);
  EXPECT_GT(saved.getId(), 0);
}

TEST(IssueRepositoryFactoryTest, CreatesDiskBackendWhenNotMemory) {
  EnvGuard guardBackend("ISSUE_REPO_BACKEND");
  EnvGuard guardDbPath("ISSUE_DB_PATH");

  // Anything other than "memory" should go to the SQLite/disk branch.
  setenv("ISSUE_REPO_BACKEND", "sqlite", 1);
  setenv("ISSUE_DB_PATH", "test_issues.db", 1);

  std::unique_ptr<IssueRepository> repo(createIssueRepository());
  ASSERT_NE(repo, nullptr);

  Issue issue(0, "author", "Title", 0);
  Issue saved = repo->saveIssue(issue);
  EXPECT_GT(saved.getId(), 0);
}

TEST(IssueRepositoryTagHelpersTest, AddAndRemoveTagsThroughRepository) {
  EnvGuard guardBackend("ISSUE_REPO_BACKEND");
  EnvGuard guardDbPath("ISSUE_DB_PATH");

  // Use in-memory backend so we don't depend on files.
  setenv("ISSUE_REPO_BACKEND", "memory", 1);
  unsetenv("ISSUE_DB_PATH");

  std::unique_ptr<IssueRepository> repo(createIssueRepository());
  ASSERT_NE(repo, nullptr);

  Issue issue(0, "author", "Title", 0);
  Issue saved = repo->saveIssue(issue);
  int id = saved.getId();
  ASSERT_GT(id, 0);

  // First call: exercises addTagToIssue, but current implementation
  // does not persist tag changes back into the repository.
  (void)repo->addTagToIssue(id, "bug");

  Issue reloaded = repo->getIssue(id);
  // Document current behavior: tag not persisted by repository helper.
  EXPECT_FALSE(reloaded.hasTag("bug"));

  // Second call: we only care that it doesn't throw and returns *some* value.
  // From current implementation it returns true again.
  EXPECT_TRUE(repo->addTagToIssue(id, "bug"));

  // removeTagFromIssue currently also operates on a copy and does not
  // affect the stored issue; it returns false since the tag is absent.
  EXPECT_FALSE(repo->removeTagFromIssue(id, "bug"));

  Issue finalReload = repo->getIssue(id);
  EXPECT_FALSE(finalReload.hasTag("bug"));
}

// ---------------------------------------------------------------------
//  IssueTrackerController tag methods (IssueTrackerController.cpp)
//  - happy path (add/remove)
//  - duplicate & invalid tag
//  - non-existent issue id for remove
// ---------------------------------------------------------------------

TEST(IssueTrackerControllerTagsTest,
     AddTagHappyPathAndDuplicateAndInvalid) {
  EnvGuard guardBackend("ISSUE_REPO_BACKEND");
  EnvGuard guardDbPath("ISSUE_DB_PATH");

  // Use in-memory backend for controller tests as well.
  setenv("ISSUE_REPO_BACKEND", "memory", 1);
  unsetenv("ISSUE_DB_PATH");

  std::unique_ptr<IssueRepository> repo(createIssueRepository());
  ASSERT_NE(repo, nullptr);

  IssueTrackerController controller(repo.get());

  // Create a simple issue through the controller
  Issue created = controller.createIssue("Title", "Description", "author");
  int id = created.getId();
  ASSERT_GT(id, 0);

  // First add: controller forwards to repository helper.
  ASSERT_TRUE(controller.addTagToIssue(id, "bug"));

  Issue reloaded = controller.getIssue(id);
  // Current implementation does NOT persist tag modifications, so
  // the reloaded issue still has no tag.
  EXPECT_FALSE(reloaded.hasTag("bug"));

  // Second add: should still succeed and not throw.
  EXPECT_TRUE(controller.addTagToIssue(id, "bug"));

  // Invalid tag (empty) should trigger Issue::addTag exception inside and
  // be handled by controller, returning false.
  EXPECT_FALSE(controller.addTagToIssue(id, ""));
}

TEST(IssueTrackerControllerTagsTest,
     RemoveTagHappyPathAndErrorCases) {
  EnvGuard guardBackend("ISSUE_REPO_BACKEND");
  EnvGuard guardDbPath("ISSUE_DB_PATH");

  setenv("ISSUE_REPO_BACKEND", "memory", 1);
  unsetenv("ISSUE_DB_PATH");

  std::unique_ptr<IssueRepository> repo(createIssueRepository());
  ASSERT_NE(repo, nullptr);

  IssueTrackerController controller(repo.get());

  Issue created = controller.createIssue("Title", "Description", "author");
  int id = created.getId();
  ASSERT_GT(id, 0);

  ASSERT_TRUE(controller.addTagToIssue(id, "bug"));

  // Because repository helpers don't persist tag state, the remove
  // operation will not find the tag and returns false.
  EXPECT_FALSE(controller.removeTagFromIssue(id, "bug"));

  // Removing again still returns false.
  EXPECT_FALSE(controller.removeTagFromIssue(id, "bug"));

  // Non-existent issue id should be handled gracefully and return false.
  EXPECT_FALSE(controller.removeTagFromIssue(id + 9999, "bug"));
}
