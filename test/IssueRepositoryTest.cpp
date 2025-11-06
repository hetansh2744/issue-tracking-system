#include "gtest/gtest.h"
#include "IssueRepository.hpp"

IssueRepository* createIssueRepository();

namespace {

class InMemoryIssueRepositoryTest : public ::testing::Test {
 protected:
    std::unique_ptr<IssueRepository> repo = nullptr;
    User testUser{ "dummy", "reporter" };
    Issue issue1{ 0, "", "" };
    Issue issue2{ 0, "", "" };

    void SetUp() override {
        repo.reset(createIssueRepository());

        testUser = repo->saveUser(User("alice_rep", "Developer"));

        issue1 = repo->saveIssue(
            Issue(0, testUser.getName(), "Issue 1 Title", 0));
        issue2 = repo->saveIssue(Issue(0, "bob_rep", "Issue 2 Title", 0));

        Comment descComment = Comment(
            0, testUser.getName(), "Initial description for issue 1.", 0);
        Comment savedDesc = repo->saveComment(issue1.getId(), descComment);
        issue1.setDescriptionCommentId(savedDesc.getId());
        repo->saveIssue(issue1);
    }
};

}  // namespace

// ====================================================================
// USER TESTS
// ====================================================================

TEST_F(InMemoryIssueRepositoryTest, SaveNewUserAndGetUser) {
    ASSERT_EQ(testUser.getName(), "alice_rep");

    User newUser("charlie", "QA");
    User savedUser = repo->saveUser(newUser);

    User fetched = repo->getUser("charlie");
    EXPECT_EQ(fetched.getRole(), "QA");
}

TEST_F(InMemoryIssueRepositoryTest, UpdateExistingUser) {
    User fetched = repo->getUser(testUser.getName());
    fetched.setRole("Lead Dev");
    repo->saveUser(fetched);

    User updated = repo->getUser(testUser.getName());
    EXPECT_EQ(updated.getRole(), "Lead Dev");
}

TEST_F(InMemoryIssueRepositoryTest, GetNonExistentUserThrows) {
    EXPECT_THROW(repo->getUser("missing_user"), std::invalid_argument);
}

TEST_F(InMemoryIssueRepositoryTest, SaveUserEmptyNameThrows) {
    EXPECT_THROW(repo->saveUser(User("", "invalid")), std::invalid_argument);
}

TEST_F(InMemoryIssueRepositoryTest, ListUsers) {
    repo->saveUser(User("bob_rep", "Reporter"));

    std::vector<User> users = repo->listAllUsers();
    EXPECT_GE(users.size(), 2u);
    bool aliceFound = false;
    bool bobFound = false;
    for (const auto& user : users) {
        if (user.getName() == "alice_rep") aliceFound = true;
        if (user.getName() == "bob_rep") bobFound = true;
    }
    EXPECT_TRUE(aliceFound);
    EXPECT_TRUE(bobFound);
}

TEST_F(InMemoryIssueRepositoryTest, DeleteUser) {
    EXPECT_TRUE(repo->deleteUser(testUser.getName()));
    EXPECT_FALSE(repo->deleteUser("non_existent"));
    EXPECT_THROW(repo->getUser(testUser.getName()), std::invalid_argument);
}

// ====================================================================
// ISSUE TESTS
// ====================================================================

TEST_F(InMemoryIssueRepositoryTest, SaveNewIssueAndGetIssue) {
    ASSERT_GT(issue1.getId(), 0);

    Issue fetched = repo->getIssue(issue1.getId());
    EXPECT_TRUE(fetched.hasDescriptionComment());
    EXPECT_GE(fetched.getComments().size(), 1u);
}

TEST_F(InMemoryIssueRepositoryTest, SaveExistingIssueThrowsIfNonExistent) {
    Issue unpersistedIssue(999, "a", "b", 0);
    unpersistedIssue.setIdForPersistence(999);
    EXPECT_THROW(repo->saveIssue(unpersistedIssue), std::invalid_argument);
}

TEST_F(InMemoryIssueRepositoryTest, GetNonExistentIssueThrows) {
    EXPECT_THROW(repo->getIssue(999), std::invalid_argument);
}

TEST_F(InMemoryIssueRepositoryTest, ListIssues) {
    std::vector<Issue> issues = repo->listIssues();
    EXPECT_EQ(issues.size(), 2u);
    EXPECT_EQ(issues[0].getTitle(), "Issue 1 Title");
    EXPECT_EQ(issues[1].getTitle(), "Issue 2 Title");
}

TEST_F(InMemoryIssueRepositoryTest, DeleteIssue_RemovesComments) {
    Comment regularComment = Comment(0, "bob_rep", "A follow-up comment.", 0);
    Comment savedComment = repo->saveComment(issue2.getId(), regularComment);

    ASSERT_NO_THROW(repo->getComment(savedComment.getId(), issue2.getId()));

    EXPECT_TRUE(repo->deleteIssue(issue2.getId()));

    EXPECT_THROW(repo->getIssue(issue2.getId()), std::invalid_argument);

    EXPECT_FALSE(repo->deleteIssue(999));
}

TEST_F(InMemoryIssueRepositoryTest, FindIssuesByUserId) {
    Issue assigned = repo->getIssue(issue1.getId());
    assigned.assignTo(testUser.getName());
    repo->saveIssue(assigned);

    std::vector<Issue> assignedIssues = repo->findIssues(testUser.getName());
    ASSERT_EQ(assignedIssues.size(), 1u);
    EXPECT_EQ(assignedIssues[0].getId(), issue1.getId());
}

TEST_F(InMemoryIssueRepositoryTest, ListAllUnassigned) {
    ASSERT_EQ(repo->listAllUnassigned().size(), 2u);

    Issue assigned = repo->getIssue(issue1.getId());
    assigned.assignTo(testUser.getName());
    repo->saveIssue(assigned);

    std::vector<Issue> unassigned = repo->listAllUnassigned();
    ASSERT_EQ(unassigned.size(), 1u);
    EXPECT_EQ(unassigned[0].getId(), issue2.getId());
}

// ====================================================================
// COMMENT TESTS
// ====================================================================

TEST_F(InMemoryIssueRepositoryTest, SaveNewCommentAndGetAllComments) {
    Comment c2 = repo->saveComment(
        issue1.getId(), Comment(0, testUser.getName(), "Follow up", 0));
    ASSERT_GT(c2.getId(), 0);

    std::vector<Comment> allComments = repo->getAllComments(issue1.getId());
    ASSERT_EQ(allComments.size(), 2u);
    EXPECT_EQ(allComments[0].getText(), "Initial description for issue 1.");
    EXPECT_EQ(allComments[1].getText(), "Follow up");
}

TEST_F(InMemoryIssueRepositoryTest, SaveExistingCommentUpdatesText) {
    Comment originalDesc = repo->getComment(
        issue1.getDescriptionCommentId(), issue1.getId());
    originalDesc.setText("Updated description text.");

    Comment updatedDesc = repo->saveComment(issue1.getId(), originalDesc);

    Comment fetched = repo->getComment(updatedDesc.getId(), issue1.getId());
    EXPECT_EQ(fetched.getText(), "Updated description text.");
    EXPECT_EQ(repo->getAllComments(issue1.getId()).size(), 1u);
}

TEST_F(InMemoryIssueRepositoryTest, SaveCommentNonExistentIssueThrows) {
    Comment c(0, "a", "b", 0);
    EXPECT_THROW(repo->saveComment(999, c), std::invalid_argument);
}

TEST_F(InMemoryIssueRepositoryTest, GetCommentNonExistentCommentIdThrows) {
    EXPECT_THROW(repo->getComment(999, issue1.getId()), std::invalid_argument);
}

TEST_F(InMemoryIssueRepositoryTest, DeleteComment_TwoArg) {
    int descId = issue1.getDescriptionCommentId();

    EXPECT_TRUE(repo->deleteComment(issue1.getId(), descId));

    EXPECT_THROW(repo->getComment(descId, issue1.getId()),
                 std::invalid_argument);

    EXPECT_FALSE(repo->deleteComment(issue1.getId(), 999));
}

TEST_F(InMemoryIssueRepositoryTest, DeleteComment_OneArg) {
    Comment c2 = repo->saveComment(
        issue1.getId(), Comment(0, testUser.getName(), "Test Comment", 0));
    int commentId = c2.getId();

    EXPECT_TRUE(repo->deleteComment(commentId));

    EXPECT_THROW(repo->getComment(commentId, issue1.getId()),
                 std::invalid_argument);

    EXPECT_THROW(repo->deleteComment(999), std::invalid_argument);
}

TEST_F(InMemoryIssueRepositoryTest, HydrateIssue_HandlesMissingComment) {
    Comment c = repo->saveComment(
        issue1.getId(), Comment(0, testUser.getName(), "To be orphaned", 0));

    // We assume the type InMemoryIssueRepository is available via linking now.
    InMemoryIssueRepository* impl =
        dynamic_cast<InMemoryIssueRepository*>(repo.get());
    ASSERT_TRUE(impl != nullptr);

    // We access the underlying map directly for this specific test case
    impl->comments_.erase(c.getId());

    Issue fetched = repo->getIssue(issue1.getId());
    EXPECT_EQ(fetched.getComments().size(), 1u);
} // namespace
