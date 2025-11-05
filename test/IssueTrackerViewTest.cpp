#include "gmock/gmock.h"
#include "IssueTrackerController.hpp"
#include "IssueTrackerView.hpp"

class MockIssueTrackerController : public IssueTrackerController {
 public:
    MOCK_METHOD(Issue, createIssue, (const std::string&,
         const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, updateIssueField, (int, const std::string&,
         const std::string&), (override));
    MOCK_METHOD(bool, assignUserToIssue, (int,
         const std::string&), (override));
    MOCK_METHOD(bool, unassignUserFromIssue, (int), (override));
    MOCK_METHOD(bool, deleteIssue, (int), (override));
    MOCK_METHOD(std::vector<Issue>, listAllIssues, (), (override));
    MOCK_METHOD(std::vector<Issue>, listAllUnassignedIssues,
         (), (override));
    MOCK_METHOD(std::vector<Issue>, findIssuesByUserId,
         (const std::string&), (override));
    MOCK_METHOD(Comment, addCommentToIssue, (int,
         const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, updateComment, (int,
         const std::string&), (override));
    MOCK_METHOD(bool, deleteComment, (int), (override));
    MOCK_METHOD(User, createUser, (const std::string&,
         const std::string&), (override));
    MOCK_METHOD(std::vector<User>, listAllUsers, (), (override));
    MOCK_METHOD(bool, removeUser, (const std::string&), (override));
};

using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

class IssueTrackerViewTest : public ::testing::Test {
 protected:
    StrictMock<MockIssueTrackerController> mockController;
    IssueTrackerView* view;

    void SetUp() override {
        view = new IssueTrackerView(&mockController);
    }
};

TEST_F(IssueTrackerViewTest, CreateIssue_CallsControllerWithCorrectArgs) {
    std::istringstream input("Title\nDescription\nUser123\n");
    std::cin.rdbuf(input.rdbuf());

    Issue dummyIssue;
    EXPECT_CALL(mockController, createIssue("Title", "Description", "User123"))
        .WillOnce(Return(dummyIssue));

    view->createIssue();
}

TEST_F(IssueTrackerViewTest, CreateIssue_CallsControllerWithCorrectArgs) {
    std::istringstream input("Title\nDescription\nUser123\n");
    std::cin.rdbuf(input.rdbuf());

    Issue dummyIssue;
    EXPECT_CALL(mockController, createIssue("Title", "Description", "User123"))
        .WillOnce(Return(dummyIssue));

    view->createIssue();
}

TEST_F(IssueTrackerViewTest, AssignUser_CallsController) {
    std::istringstream input("1\nuser42\n");
    std::cin.rdbuf(input.rdbuf());

    EXPECT_CALL(mockController, assignUserToIssue(1, "user42"))
        .WillOnce(Return(true));

    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    view->assignUser();

    std::string out = output.str();
    EXPECT_NE(out.find("User assigned"), std::string::npos);
}

TEST_F(IssueTrackerViewTest, ListIssues_PrintsNoIssuesIfEmpty) {
    EXPECT_CALL(mockController, listAllIssues())
        .WillOnce(Return(std::vector<Issue>{}));

    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    view->listIssues();

    std::string out = output.str();
    EXPECT_NE(out.find("No issues found"), std::string::npos);
}

TEST_F(IssueTrackerViewTest, DeleteIssue_CallsControllerAndPrintsResult) {
    std::istringstream input("5\n");
    std::cin.rdbuf(input.rdbuf());

    EXPECT_CALL(mockController, deleteIssue(5))
        .WillOnce(Return(true));

    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    view->deleteIssue();

    std::string out = output.str();
    EXPECT_NE(out.find("Deleted successfully"), std::string::npos);
}
