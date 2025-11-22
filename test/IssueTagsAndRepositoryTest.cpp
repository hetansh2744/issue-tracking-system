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
  EnvGuard(const char* name)
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

  // First tag add should return true
  EXPECT_TRUE(repo->addTagToIssue(id, "bug"));

  Issue reloaded = repo->getIssue(id);
  EXPECT_TRUE(reloaded.hasTag("bug"));

  // Second add of same tag should return false
  EXPECT_FALSE(repo->addTagToIssue(id, "bug"));

  // Remove tag returns true once, then false
  EXPECT_TRUE(repo->removeTagFromIssue(id, "bug"));
  Issue afterRemove = repo->getIssue(id);
  EXPECT_FALSE(afterRemove.hasTag("bug"));

  EXPECT_FALSE(repo->removeTagFromIssue(id, "bug"));
}

// ---------------------------------------------------------------------
//  IssueTrackerController tag methods (IssueTrackerController.cpp)
//  - happy path (add/remove)
//  - duplicate & invalid tag (exception path via Issue::addTag)
//  - non-existent issue id for remove (exception path via getIssue)
// ---------------------------------------------------------------------

TEST(IssueTrackerControllerTagsTest, AddTagHappyPathAndDuplicateAndInvalid) {
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

  // Happy path: first tag add returns true and is persisted
  EXPECT_TRUE(controller.addTagToIssue(id, "bug"));

  Issue reloaded = controller.getIssue(id);
  EXPECT_TRUE(reloaded.hasTag("bug"));

  // Duplicate tag: controller still returns false
  EXPECT_FALSE(controller.addTagToIssue(id, "bug"));

  // Invalid tag (empty) should trigger Issue::addTag exception and
  // be caught by controller, returning false.
  EXPECT_FALSE(controller.addTagToIssue(id, ""));
}

TEST(IssueTrackerControllerTagsTest, RemoveTagHappyPathAndErrorCases) {
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

  // Happy path: tag removed
  EXPECT_TRUE(controller.removeTagFromIssue(id, "bug"));

  Issue afterRemove = controller.getIssue(id);
  EXPECT_FALSE(afterRemove.hasTag("bug"));

  // Removing again should return false (no tag)
  EXPECT_FALSE(controller.removeTagFromIssue(id, "bug"));

  // Non-existent issue id should be handled gracefully and return false
  EXPECT_FALSE(controller.removeTagFromIssue(id + 9999, "bug"));
}
