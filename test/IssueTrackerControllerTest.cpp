#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "IssueTrackerController.h"

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

TEST(IssueTrackerControllerTest, CreateIssueValid) {
    MockIssueRepository mockRepo;

    Issue newIssue("title", "desc", "user123");
    EXPECT_CALL(mockRepo, saveIssue(testing::_))
        .WillOnce(testing::Return(newIssue));

    IssueTrackerController controller(&mockRepo);
    Issue result = controller.createIssue("title",
        "desc", "user123");

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
