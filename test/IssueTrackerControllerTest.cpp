#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "IssueTrackerController.hpp"

using ::testing::Throw;

class MockIssueRepository : public IssueRepository {
 public:
    MOCK_METHOD(Issue, saveIssue, (const Issue& issue), (override));
    MOCK_METHOD(Issue, getIssue, (int id), (const, override));
    MOCK_METHOD(bool, deleteIssue, (int id), (override));
    MOCK_METHOD(std::vector<Issue>, listIssues, (), (const, override));
    MOCK_METHOD(std::vector<Issue>, findIssues,
        (std::function<bool(const Issue&)> criteria), (const, override));
    MOCK_METHOD(std::vector<Issue>, findIssues,
        (const std::string& userId), (const, override));
    MOCK_METHOD(std::vector<Issue>, listAllUnassigned, (), (const, override));

    MOCK_METHOD(Comment, saveComment, (int issueId,
        const Comment& comment), (override));
    MOCK_METHOD(Comment, getComment, (int issueId,
        int commentId), (const, override));
    MOCK_METHOD(bool, deleteComment, (int issueId, int commentId), (override));
    MOCK_METHOD(std::vector<Comment>,
        getAllComments, (int issueId), (const, override));
    MOCK_METHOD(bool, deleteComment, (int commentId), (override));

    MOCK_METHOD(User, saveUser, (const User& user), (override));
    MOCK_METHOD(User, getUser, (const std::string& id), (const, override));
    MOCK_METHOD(bool, deleteUser, (const std::string& id), (override));
    MOCK_METHOD(std::vector<User>, listAllUsers, (), (const, override));
};

TEST(IssueTrackerControllerTest, CreateIssueValid) {
    MockIssueRepository mockRepo;
    Issue persistedIssue(1, "user123", "title", 0);
    Comment descComment(1, "user123", "desc", 0);

    testing::InSequence seq;
    EXPECT_CALL(mockRepo, saveIssue(testing::_))
        .WillOnce(testing::Return(persistedIssue));
    EXPECT_CALL(mockRepo, saveComment(1, testing::_))
        .WillOnce(testing::Return(descComment));
    EXPECT_CALL(mockRepo, saveIssue(testing::_))
        .WillOnce(testing::Return(persistedIssue));

    IssueTrackerController controller(&mockRepo);
    Issue result = controller.createIssue("title", "desc", "user123");

    EXPECT_EQ(result.getAuthorId(), "user123");
}

TEST(IssueTrackerControllerTest, UpdateIssueFieldTitleSuccess) {
    MockIssueRepository mockRepo;
    Issue existingIssue(1, "user", "old", 0);

    EXPECT_CALL(mockRepo, getIssue(1))
        .WillOnce(testing::Return(existingIssue));
    EXPECT_CALL(mockRepo, saveIssue(testing::_))
        .Times(1);

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateIssueField(1, "title", "newTitle");

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, UpdateIssueFieldDescriptionSuccess) {
    MockIssueRepository mockRepo;
    Issue existingIssue(1, "user", "title", 0);

    EXPECT_CALL(mockRepo, getIssue(1))
        .WillOnce(testing::Return(existingIssue));
    EXPECT_CALL(mockRepo, saveComment(1, testing::_))
        .WillOnce(testing::Return(Comment(1, "user", "newDesc", 0)));
    EXPECT_CALL(mockRepo, saveIssue(testing::_))
        .Times(1);

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateIssueField(1, "description", "newDesc");

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, UpdateIssueFieldInvalidField) {
    MockIssueRepository mockRepo;
    Issue existingIssue(1, "user", "title", 0);

    EXPECT_CALL(mockRepo, getIssue(1))
        .WillOnce(testing::Return(existingIssue));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateIssueField(1, "invalid", "value");

    EXPECT_FALSE(result);
}

TEST(IssueTrackerControllerTest, UpdateIssueFieldThrows) {
    MockIssueRepository mockRepo;

    EXPECT_CALL(mockRepo, getIssue(1))
        .WillOnce(testing::Throw(std::invalid_argument("Not found")));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateIssueField(1, "title", "newTitle");

    EXPECT_FALSE(result);
}

TEST(IssueTrackerControllerTest, AssignUserToIssueSuccess) {
    MockIssueRepository mockRepo;
    Issue existingIssue(1, "author", "title", 0);
    User user("name", "role");

    EXPECT_CALL(mockRepo, getUser("user123"))
        .WillOnce(testing::Return(user));
    EXPECT_CALL(mockRepo, getIssue(1))
        .WillOnce(testing::Return(existingIssue));
    EXPECT_CALL(mockRepo, saveIssue(testing::_))
        .Times(1);

    IssueTrackerController controller(&mockRepo);
    bool result = controller.assignUserToIssue(1, "user123");

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, AssignUserToIssueThrows) {
    MockIssueRepository mockRepo;

    EXPECT_CALL(mockRepo, getUser("user123"))
        .WillOnce(testing::Throw(std::out_of_range("Not found")));

    IssueTrackerController controller(&mockRepo);
    EXPECT_NO_THROW({
        bool result = controller.assignUserToIssue(1, "user123");
        EXPECT_FALSE(result);
    });
}

TEST(IssueTrackerControllerTest, UnassignUserFromIssueSuccess) {
    MockIssueRepository mockRepo;
    Issue existingIssue(1, "author", "title", 0);

    EXPECT_CALL(mockRepo, getIssue(1))
        .WillOnce(testing::Return(existingIssue));
    EXPECT_CALL(mockRepo, saveIssue(testing::_))
        .Times(1);

    IssueTrackerController controller(&mockRepo);
    bool result = controller.unassignUserFromIssue(1);

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, UnassignUserFromIssueThrows) {
    MockIssueRepository mockRepo;

    EXPECT_CALL(mockRepo, getIssue(1))
        .WillOnce(testing::Throw(std::out_of_range("Not found")));

    IssueTrackerController controller(&mockRepo);
    EXPECT_NO_THROW({
        bool result = controller.unassignUserFromIssue(1);
        EXPECT_FALSE(result);
    });
}

TEST(IssueTrackerControllerTest, DeleteIssue) {
    MockIssueRepository mockRepo;

    EXPECT_CALL(mockRepo, deleteIssue(1))
        .WillOnce(testing::Return(true));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.deleteIssue(1);

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, ListAllIssues) {
    MockIssueRepository mockRepo;
    std::vector<Issue> issues = { Issue(1, "u1", "t1", 0),
        Issue(2, "u2", "t2", 0) };

    EXPECT_CALL(mockRepo, listIssues())
        .WillOnce(testing::Return(issues));

    IssueTrackerController controller(&mockRepo);
    std::vector<Issue> result = controller.listAllIssues();

    EXPECT_EQ(result.size(), 2);
}

TEST(IssueTrackerControllerTest, FindIssuesByUserId) {
    MockIssueRepository mockRepo;
    std::vector<Issue> issues = { Issue(1, "u1", "t1", 0) };

    EXPECT_CALL(mockRepo, findIssues("u1"))
        .WillOnce(testing::Return(issues));

    IssueTrackerController controller(&mockRepo);
    std::vector<Issue> result = controller.findIssuesByUserId("u1");

    EXPECT_EQ(result.size(), 1);
}

TEST(IssueTrackerControllerTest, AddCommentToIssueSuccess) {
    MockIssueRepository mockRepo;
    Issue issue(1, "author", "title", 0);
    User user("author", "role");
    Comment comment(1, "author", "text", 1);

    EXPECT_CALL(mockRepo, getIssue(1))
        .WillOnce(testing::Return(issue));
    EXPECT_CALL(mockRepo, getUser("author"))
        .WillOnce(testing::Return(user));
    EXPECT_CALL(mockRepo, saveComment(1, testing::_))
        .WillOnce(testing::Return(comment));
    EXPECT_CALL(mockRepo, saveIssue(testing::_))
        .Times(1);

    IssueTrackerController controller(&mockRepo);
    Comment result = controller.addCommentToIssue(1, "text", "author");

    EXPECT_EQ(result.getAuthor(), "author");
}

TEST(IssueTrackerControllerTest, UpdateCommentSuccess) {
    MockIssueRepository mockRepo;
    Comment comment(1, "author", "oldText", 1);

    EXPECT_CALL(mockRepo, getComment(1, 1))
        .WillOnce(testing::Return(comment));
    EXPECT_CALL(mockRepo, saveComment(1, testing::_))
        .Times(1);

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateComment(1, 1, "newText");

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, UpdateCommentThrows) {
    MockIssueRepository mockRepo;

    EXPECT_CALL(mockRepo, getComment(1, 1))
        .WillOnce(testing::Throw(std::out_of_range("Not found")));

    IssueTrackerController controller(&mockRepo);
    EXPECT_NO_THROW({
        bool result = controller.updateComment(1, 1, "text");
        EXPECT_FALSE(result);
    });
}

TEST(IssueTrackerControllerTest, DeleteCommentSuccess) {
    MockIssueRepository mockRepo;
    Comment comment(5, "author", "text", 1);
    Issue issue(1, "author", "title", 0);

    EXPECT_CALL(mockRepo, getComment(1, 5))
        .WillOnce(testing::Return(comment));
    EXPECT_CALL(mockRepo, deleteComment(1, 5))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(mockRepo, getIssue(1))
        .WillOnce(testing::Return(issue));
    EXPECT_CALL(mockRepo, saveIssue(testing::_))
        .Times(1);

    IssueTrackerController controller(&mockRepo);
    bool result = controller.deleteComment(1, 5);

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, DeleteCommentThrows) {
    MockIssueRepository mockRepo;

    EXPECT_CALL(mockRepo, getComment(1, 5))
        .WillOnce(testing::Throw(std::out_of_range("Not found")));

    IssueTrackerController controller(&mockRepo);
    EXPECT_NO_THROW({
        bool result = controller.deleteComment(1, 5);
        EXPECT_FALSE(result);
    });
}

TEST(IssueTrackerControllerTest, CreateUserSuccess) {
    MockIssueRepository mockRepo;
    User user("name", "role");

    EXPECT_CALL(mockRepo, saveUser(testing::_))
        .WillOnce(testing::Return(user));

    IssueTrackerController controller(&mockRepo);
    User result = controller.createUser("name", "role");

    EXPECT_EQ(result.getName(), "name");
}

TEST(IssueTrackerControllerTest, CreateUserEmptyName) {
    MockIssueRepository mockRepo;
    IssueTrackerController controller(&mockRepo);
    User result = controller.createUser("", "");

    EXPECT_EQ(result.getName(), "");
}

TEST(IssueTrackerControllerTest, UpdateUserSuccess) {
    MockIssueRepository mockRepo;
    User user("oldName", "role");

    EXPECT_CALL(mockRepo, getUser("user123"))
        .WillOnce(testing::Return(user));
    EXPECT_CALL(mockRepo, saveUser(testing::_))
        .WillOnce(testing::Return(User("newName", "role")));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateUser("user123", "name", "newName");

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, UpdateUserInvalidField) {
    MockIssueRepository mockRepo;
    User user("name", "role");

    EXPECT_CALL(mockRepo, getUser("user123"))
        .WillOnce(testing::Return(user));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateUser("user123", "invalid", "value");

    EXPECT_FALSE(result);
}

TEST(IssueTrackerControllerTest, UpdateUserThrows) {
    MockIssueRepository mockRepo;

    EXPECT_CALL(mockRepo, getUser("user123"))
        .WillOnce(testing::Throw(std::invalid_argument("Not found")));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.updateUser("user123", "name", "newName");

    EXPECT_FALSE(result);
}

TEST(IssueTrackerControllerTest, RemoveUser) {
    MockIssueRepository mockRepo;

    EXPECT_CALL(mockRepo, deleteUser("123"))
        .WillOnce(testing::Return(true));

    IssueTrackerController controller(&mockRepo);
    bool result = controller.removeUser("123");

    EXPECT_TRUE(result);
}

TEST(IssueTrackerControllerTest, ListAllUsers) {
    MockIssueRepository mockRepo;
    std::vector<User> users = { User("u1", "r1"), User("u2", "r2") };

    EXPECT_CALL(mockRepo, listAllUsers())
        .WillOnce(testing::Return(users));

    IssueTrackerController controller(&mockRepo);
    std::vector<User> result = controller.listAllUsers();

    EXPECT_EQ(result.size(), 2);
}

TEST(IssueTrackerControllerTest, ListAllUnassignedIssues) {
    MockIssueRepository mockRepo;
    std::vector<Issue> issues = { Issue(1, "u1", "t1", 0) };

    EXPECT_CALL(mockRepo, listAllUnassigned())
        .WillOnce(testing::Return(issues));

    IssueTrackerController controller(&mockRepo);
    std::vector<Issue> result = controller.listAllUnassignedIssues();

    EXPECT_EQ(result.size(), 1);
}
