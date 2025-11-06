#include "gtest/gtest.h"

#include <memory>
#include <string>

#include "Comment.hpp"
#include "Issue.hpp"
#include "IssueRepository.hpp"
#include "IssueTrackerController.hpp"
#include "User.hpp"

namespace {

class ControllerWithRepoTest : public ::testing::Test {
 protected:
  std::unique_ptr<IssueRepository> repo;
  std::unique_ptr<IssueTrackerController> controller;

  void SetUp() override {
    repo.reset(createIssueRepository());
    controller = std::make_unique<IssueTrackerController>(repo.get());
  }
};

}  // namespace

TEST_F(ControllerWithRepoTest, FullWorkflowCoversControllerBranches) {
  User author = controller->createUser("author", "reporter");
  User assignee = controller->createUser("assignee", "developer");
  User commenter = controller->createUser("commenter", "qa");
  User temp = controller->createUser("temp", "intern");

  EXPECT_TRUE(controller->createUser("", "role").getName().empty());
  EXPECT_TRUE(controller->createUser("", "").getName().empty());

  EXPECT_TRUE(controller->updateUser("assignee", "role", "lead"));
  EXPECT_FALSE(controller->updateUser("assignee", "invalid", "value"));
  EXPECT_FALSE(controller->updateUser("ghost", "role", "value"));

  EXPECT_TRUE(controller->removeUser("temp"));
  EXPECT_FALSE(controller->removeUser("missing"));

  Issue created =
      controller->createIssue("Login Bug", "Initial desc", author.getName());
  ASSERT_GT(created.getId(), 0);

  Issue persisted = repo->getIssue(created.getId());
  ASSERT_TRUE(persisted.hasDescriptionComment());
  const int firstDescId = persisted.getDescriptionCommentId();

  EXPECT_TRUE(controller->updateIssueField(
      created.getId(), "title", "Login Bug (updated)"));

  EXPECT_TRUE(controller->updateIssueField(
      created.getId(), "description", "Updated desc"));
  Issue afterFirstDescriptionUpdate = repo->getIssue(created.getId());
  const int secondDescId =
      afterFirstDescriptionUpdate.getDescriptionCommentId();
  EXPECT_NE(firstDescId, secondDescId);

  Comment cachedDesc = repo->getComment(secondDescId);
  afterFirstDescriptionUpdate.addComment(cachedDesc);
  repo->saveIssue(afterFirstDescriptionUpdate);
  EXPECT_TRUE(controller->updateIssueField(
      created.getId(), "description", "Cached desc updated"));

  EXPECT_FALSE(
      controller->updateIssueField(created.getId(), "unknownField", "value"));

  EXPECT_TRUE(
      controller->assignUserToIssue(created.getId(), assignee.getName()));
  EXPECT_EQ(repo->getIssue(created.getId()).getAssignedTo(), assignee.getName());
  EXPECT_THROW(controller->assignUserToIssue(created.getId(), "ghost"),
      std::invalid_argument);
  EXPECT_THROW(
      controller->assignUserToIssue(999, assignee.getName()),
      std::invalid_argument);

  EXPECT_TRUE(controller->unassignUserFromIssue(created.getId()));
  EXPECT_FALSE(repo->getIssue(created.getId()).hasAssignee());
  EXPECT_THROW(controller->unassignUserFromIssue(999),
      std::invalid_argument);

  Comment added = controller->addCommentToIssue(
      created.getId(), "Follow up", commenter.getName());
  ASSERT_GT(added.getId(), 0);
  EXPECT_THROW(
      controller->addCommentToIssue(created.getId(), "",
          commenter.getName()),
      std::invalid_argument);
  EXPECT_THROW(
      controller->addCommentToIssue(999, "Invalid issue",
          commenter.getName()),
      std::invalid_argument);
  EXPECT_THROW(
      controller->addCommentToIssue(created.getId(), "Unknown user", "ghost"),
      std::invalid_argument);

  EXPECT_TRUE(controller->updateComment(added.getId(), "Edited follow up"));
  EXPECT_EQ(repo->getComment(added.getId()).getText(), "Edited follow up");
  EXPECT_THROW(controller->updateComment(added.getId(), ""),
      std::invalid_argument);
  EXPECT_THROW(controller->updateComment(999, "No comment"),
      std::invalid_argument);

  EXPECT_TRUE(controller->deleteComment(firstDescId));
  EXPECT_THROW(controller->deleteComment(firstDescId),
      std::invalid_argument);
  EXPECT_THROW(controller->deleteComment(999),
      std::invalid_argument);

  auto allIssues = controller->listAllIssues();
  ASSERT_EQ(allIssues.size(), 1u);
  EXPECT_TRUE(controller->findIssuesByUserId(assignee.getName()).empty());
  auto initialUnassigned = controller->listAllUnassignedIssues();
  ASSERT_EQ(initialUnassigned.size(), 1u);

  EXPECT_TRUE(
      controller->assignUserToIssue(created.getId(), assignee.getName()));
  auto assignedIssues = controller->findIssuesByUserId(assignee.getName());
  ASSERT_EQ(assignedIssues.size(), 1u);
  controller->unassignUserFromIssue(created.getId());
  auto unassignedIssues = controller->listAllUnassignedIssues();
  ASSERT_EQ(unassignedIssues.size(), 1u);
  EXPECT_EQ(unassignedIssues[0].getId(), created.getId());

  auto users = controller->listAllUsers();
  EXPECT_GE(users.size(), 3u);

  auto titleMatches = repo->findIssues([](const Issue& issue) {
    return issue.getTitle().find("Login") != std::string::npos;
  });
  EXPECT_EQ(titleMatches.size(), 1u);

  EXPECT_TRUE(controller->deleteIssue(created.getId()));
  EXPECT_FALSE(controller->deleteIssue(created.getId()));
}
