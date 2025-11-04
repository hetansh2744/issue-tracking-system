#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "IssueTrackerController.h"

// ---------------- Mock Repository ----------------
class MockIssueRepository : public IssueRepository {
 public:
    MOCK_METHOD(Issue, saveIssue, (const Issue& issue), (override));
    MOCK_METHOD(Issue, getIssue, (int id), (override));
    MOCK_METHOD(bool, deleteIssue, (int id), (override));
    MOCK_METHOD(std::vector<Issue>, listIssues, (), (override));
    MOCK_METHOD(std::vector<Issue>, findIssues,
        (const std::string& userId), (override));

    MOCK_METHOD(Comment, saveComment, (const Comment& comment), (override));
    MOCK_METHOD(Comment, getComment, (int id), (override));
    MOCK_METHOD(bool, deleteComment, (int id), (override));

    MOCK_METHOD(User, saveUser, (const User& user), (override));
    MOCK_METHOD(User, getUser, (const std::string& id), (override));
    MOCK_METHOD(bool, deleteUser, (const std::string& id), (override));
    MOCK_METHOD(std::vector<User>, listAllUsers, (), (override));
};

// createIssue
TEST(IssueTrackerControllerTest, CreateIssueValid) {
    MockIssueRepository mockRepo;

    Issue newIssue("title", "desc", "user123");
    EXPECT_CALL(mockRepo, saveIssue(testing::_))
        .WillOnce(testing::Return(newIssue));

    IssueTrackerController controller(&mockRepo);
    Issue result = controller.createIssue("title", "desc", "user123");

    EXPECT_EQ(result.title, "title");
    EXPECT_EQ(result.description, "desc");
    EXPECT_EQ(result.assignedTo, "user123");
}

TEST(IssueTrackerControllerTest, CreateIssueInvalidEmptyFields) {
    MockIssueRepository mockRepo;

    IssueTrackerController controller(&mockRepo);
    Issue result = controller.createIssue("", "", "user123");

    EXPECT_EQ(result.title, "");
    EXPECT_EQ(result.description, "");
    EXPECT_EQ(result.assignedTo, "");
}

// updateIssueField
TEST(IssueTrackerControllerTest, UpdateIssueFieldTitleSuccess) {
    MockIssueRepository mockRepo;

    Issue existingIssue("old", "desc", "user");
    EXPECT_CALL(mockRepo, getIssue(1)).WillOnce(testing::Return(existingIssue));
    EXPECT_CALL(mockRepo, saveIssue(testing::_)).Times(testing::Exactly(1));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateIssueField(1, "title", "newTitle");

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, UpdateIssueFieldInvalidField) {
    MockIssueRepository mockRepo;

    Issue existingIssue("old", "desc", "user");
    EXPECT_CALL(mockRepo, getIssue(1)).WillOnce(testing::Return(existingIssue));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateIssueField(1, "invalid", "value");

    EXPECT_FALSE(result);
}

TEST(IssueTrackerControllerTest, UpdateIssueFieldThrows) {
    MockIssueRepository mockRepo;

    EXPECT_CALL(mockRepo, getIssue(1))
        .WillOnce(testing::Throw(std::out_of_range("Not found")));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateIssueField(1, "title", "newTitle");

    EXPECT_FALSE(result);
}

// assignUserToIssue
TEST(IssueTrackerControllerTest, AssignUserToIssueSuccess) {
    MockIssueRepository mockRepo;
    Issue existingIssue("title", "desc", "");

    EXPECT_CALL(mockRepo,
        getUser("user123")).WillOnce(testing::Return(User("user123")));
    EXPECT_CALL(mockRepo, getIssue(1)).WillOnce(testing::Return(existingIssue));
    EXPECT_CALL(mockRepo, saveIssue(testing::_)).Times(testing::Exactly(1));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.assignUserToIssue(1, "user123");

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, AssignUserToIssueThrows) {
    MockIssueRepository mockRepo;

    EXPECT_CALL(mockRepo, getUser("user123"))
        .WillOnce(testing::Throw(std::out_of_range("Not found")));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.assignUserToIssue(1, "user123");

    EXPECT_FALSE(result);
}

// unassignUserFromIssue
TEST(IssueTrackerControllerTest, UnassignUserFromIssueSuccess) {
    MockIssueRepository mockRepo;
    Issue existingIssue("title", "desc", "user123");

    EXPECT_CALL(mockRepo, getIssue(1)).WillOnce(testing::Return(existingIssue));
    EXPECT_CALL(mockRepo, saveIssue(testing::_)).Times(testing::Exactly(1));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.unassignUserFromIssue(1);

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, UnassignUserFromIssueThrows) {
    MockIssueRepository mockRepo;

    EXPECT_CALL(mockRepo, getIssue(1))
        .WillOnce(testing::Throw(std::out_of_range("Not found")));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.unassignUserFromIssue(1);

    EXPECT_FALSE(result);
}

// deleteIssue
TEST(IssueTrackerControllerTest, DeleteIssue) {
    MockIssueRepository mockRepo;
    EXPECT_CALL(mockRepo, deleteIssue(1)).WillOnce(testing::Return(true));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.deleteIssue(1);

    EXPECT_TRUE(result);
}

// listAllIssues
TEST(IssueTrackerControllerTest, ListAllIssues) {
    MockIssueRepository mockRepo;
    std::vector<Issue> issues = { Issue("t1",
        "d1", "u1"), Issue("t2","d2","u2") };
    EXPECT_CALL(mockRepo, listIssues()).WillOnce(testing::Return(issues));

    IssueTrackerController controller(&mockRepo);
    std::vector<Issue> result = controller.listAllIssues();

    EXPECT_EQ(result.size(), 2);
}

// findIssuesByUserId
TEST(IssueTrackerControllerTest, FindIssuesByUserId) {
    MockIssueRepository mockRepo;
    std::vector<Issue> issues = { Issue("t1", "d1", "u1") };
    EXPECT_CALL(mockRepo, findIssues("u1")).WillOnce(testing::Return(issues));

    IssueTrackerController controller(&mockRepo);
    std::vector<Issue> result = controller.findIssuesByUserId("u1");

    EXPECT_EQ(result.size(), 1);
}

// addCommentToIssue
TEST(IssueTrackerControllerTest, AddCommentToIssueSuccess) {
    MockIssueRepository mockRepo;
    Issue issue("title", "desc", "user");
    EXPECT_CALL(mockRepo, getIssue(1)).WillOnce(testing::Return(issue));
    EXPECT_CALL(mockRepo,
        getUser("author")).WillOnce(testing::Return(User("author")));
    Comment newComment(1,"author", "text");
    EXPECT_CALL(mockRepo,
        saveComment(testing::_)).WillOnce(testing::Return(newComment));
    EXPECT_CALL(mockRepo, saveIssue(testing::_)).Times(testing::Exactly(1));

    IssueTrackerController controller(&mockRepo);
    Comment result = controller.addCommentToIssue(1,
        "text", "author");

    EXPECT_EQ(result.issueId, 1);
    EXPECT_EQ(result.authorId, "author");
    EXPECT_EQ(result.text, "text");
}

TEST(IssueTrackerControllerTest, AddCommentToIssueEmptyText) {
    MockIssueRepository mockRepo;

    IssueTrackerController controller(&mockRepo);
    Comment result = controller.addCommentToIssue(1,
        "", "author");

    EXPECT_EQ(result.id, 0);
}

// updateComment
TEST(IssueTrackerControllerTest, UpdateCommentSuccess) {
    MockIssueRepository mockRepo;
    Comment comment(1,"author", "oldText");
    EXPECT_CALL(mockRepo, getComment(5)).WillOnce(testing::Return(comment));
    EXPECT_CALL(mockRepo, saveComment(testing::_)).Times(testing::Exactly(1));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateComment(5, "newText");

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, UpdateCommentThrows) {
    MockIssueRepository mockRepo;
    EXPECT_CALL(mockRepo,
        getComment(5)).WillOnce(testing::Throw(std::out_of_range("Not found")));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateComment(5, "text");

    EXPECT_FALSE(result);
}

// deleteComment
TEST(IssueTrackerControllerTest, DeleteCommentSuccess) {
    MockIssueRepository mockRepo;
    Comment comment(1,"author", "text");
    comment.id = 5;
    Issue issue("title", "desc", "user");
    issue.id = 1;
    issue.commentIds.push_back(5);

    EXPECT_CALL(mockRepo, getComment(5)).WillOnce(testing::Return(comment));
    EXPECT_CALL(mockRepo, deleteComment(5)).WillOnce(testing::Return(true));
    EXPECT_CALL(mockRepo, getIssue(1)).WillOnce(testing::Return(issue));
    EXPECT_CALL(mockRepo, saveIssue(testing::_)).Times(testing::Exactly(1));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.deleteComment(5);

    EXPECT_TRUE(result);
}

// createUser
TEST(IssueTrackerControllerTest, CreateUserSuccess) {
    MockIssueRepository mockRepo;
    User user("name", "123");
    EXPECT_CALL(mockRepo, saveUser(testing::_)).WillOnce(testing::Return(user));

    IssueTrackerController controller(&mockRepo);
    User result = controller.createUser("name");

    EXPECT_EQ(result.name, "name");
    EXPECT_EQ(result.id, "123");
}

TEST(IssueTrackerControllerTest, CreateUserEmptyName) {
    MockIssueRepository mockRepo;

    IssueTrackerController controller(&mockRepo);
    User result = controller.createUser("");

    EXPECT_EQ(result.name, "");
    EXPECT_EQ(result.id, "");
}

// removeUser
TEST(IssueTrackerControllerTest, RemoveUser) {
    MockIssueRepository mockRepo;
    EXPECT_CALL(mockRepo, deleteUser("123")).WillOnce(testing::Return(true));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.removeUser("123");

    EXPECT_TRUE(result);
}

// listAllUsers
TEST(IssueTrackerControllerTest, ListAllUsers) {
    MockIssueRepository mockRepo;
    std::vector<User> users = { User("u1", "id1"), User("u2", "id2") };
    EXPECT_CALL(mockRepo, listAllUsers()).WillOnce(testing::Return(users));

    IssueTrackerController controller(&mockRepo);
    std::vector<User> result = controller.listAllUsers();

    EXPECT_EQ(result.size(), 2);
}
