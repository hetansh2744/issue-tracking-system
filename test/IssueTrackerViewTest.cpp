#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include "IssueTrackerView.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::HasSubstr;

class MockController : public IssueTrackerController {
 public:
  MOCK_METHOD(Issue, createIssue, (const std::string&, const std::string&, const std::string&), (override));
  MOCK_METHOD(bool, updateIssueField, (int, const std::string&, const std::string&), (override));
  MOCK_METHOD(bool, assignUserToIssue, (int, const std::string&), (override));
  MOCK_METHOD(bool, unassignUserFromIssue, (int), (override));
  MOCK_METHOD(bool, deleteIssue, (int), (override));
  MOCK_METHOD(std::vector<Issue>, listAllIssues, (), (override));
  MOCK_METHOD(std::vector<Issue>, listAllUnassignedIssues, (), (override));
  MOCK_METHOD(std::vector<Issue>, findIssuesByUserId, (const std::string&), (override));
  MOCK_METHOD(User, createUser, (const std::string&, const std::string&), (override));
  MOCK_METHOD(std::vector<User>, listAllUsers, (), (override));
  MOCK_METHOD(bool, removeUser, (const std::string&), (override));
  MOCK_METHOD(bool, updateUser, (const std::string&, const std::string&, const std::string&), (override));
  MOCK_METHOD(Issue, getIssue, (int), (override));
  MOCK_METHOD(std::vector<Comment>, getallComments, (int), (override));
  MOCK_METHOD(Comment, addCommentToIssue, (int, const std::string&, const std::string&), (override));
  MOCK_METHOD(bool, updateComment, (int, int, const std::string&), (override));
  MOCK_METHOD(bool, deleteComment, (int, int), (override));
};

// ---------------------------
// Class for redirecting IO
// ---------------------------
struct StreamRedirect {
  std::streambuf* old_cin;
  std::streambuf* old_cout;
  std::stringstream input;
  std::stringstream output;

  StreamRedirect(const std::string& in = "") {
    old_cin = std::cin.rdbuf(input.rdbuf());
    old_cout = std::cout.rdbuf(output.rdbuf());
    input.str(in);
  }

  ~StreamRedirect() {
    std::cin.rdbuf(old_cin);
    std::cout.rdbuf(old_cout);
  }

  std::string getOutput() const { return output.str(); }
};

TEST(IssueTrackerViewTest, GetValidIntAcceptsValidInput) {
  MockController mock;
  IssueTrackerView view(&mock);
  StreamRedirect sr("2\n");
  EXPECT_EQ(view.getvalidInt(3), 2);
  EXPECT_THAT(sr.getOutput(), HasSubstr("Input accepted"));
}

TEST(IssueTrackerViewTest, GetValidIntHandlesInvalidInput) {
  MockController mock;
  IssueTrackerView view(&mock);
  StreamRedirect sr("abc\n4\n2\n");
  EXPECT_EQ(view.getvalidInt(3), 2);
  EXPECT_THAT(sr.getOutput(), HasSubstr("Invalid"));
}

TEST(IssueTrackerViewTest, ListIssuesNoIssues) {
  MockController mock;
  EXPECT_CALL(mock, listAllIssues()).WillOnce(Return(std::vector<Issue>{}));
  IssueTrackerView view(&mock);
  StreamRedirect sr;
  view.listIssues();
  EXPECT_THAT(sr.getOutput(), HasSubstr("No issues found"));
}

TEST(IssueTrackerViewTest, ListUnassignedIssuesPrintsDetails) {
  MockController mock;
  Issue fake(1, "a1", "Bug", 123);
  EXPECT_CALL(mock, listAllUnassignedIssues()).WillOnce(Return(std::vector<Issue>{fake}));
  IssueTrackerView view(&mock);
  StreamRedirect sr;
  view.listUnassignedIssues();
  EXPECT_THAT(sr.getOutput(), HasSubstr("Unassigned Issues"));
  EXPECT_THAT(sr.getOutput(), HasSubstr("Bug"));
}

TEST(IssueTrackerViewTest, ListUsersEmpty) {
  MockController mock;
  EXPECT_CALL(mock, listAllUsers()).WillOnce(Return(std::vector<User>{}));
  IssueTrackerView view(&mock);
  StreamRedirect sr;
  view.listUsers();
  EXPECT_THAT(sr.getOutput(), HasSubstr("No users found"));
}

TEST(IssueTrackerViewTest, DeleteIssueSuccess) {
  MockController mock;
  Issue i(1, "a", "title", 1);
  EXPECT_CALL(mock, listAllIssues()).WillOnce(Return(std::vector<Issue>{i}));
  EXPECT_CALL(mock, deleteIssue(1)).WillOnce(Return(true));
  IssueTrackerView view(&mock);
  StreamRedirect sr("1\n");
  view.deleteIssue();
  EXPECT_THAT(sr.getOutput(), HasSubstr("Deleted successfully"));
}

TEST(IssueTrackerViewTest, FindIssuesByUserWorks) {
  MockController mock;
  Issue i(5, "x", "Crash bug", 10);
  EXPECT_CALL(mock, findIssuesByUserId("userA")).WillOnce(Return(std::vector<Issue>{i}));
  IssueTrackerView view(&mock);
  StreamRedirect sr("userA\n");
  view.findIssuesByUser();
  EXPECT_THAT(sr.getOutput(), HasSubstr("Crash bug"));
}

TEST(IssueTrackerViewTest, AssignUserSuccess) {
  MockController mock;
  User u("dev", "Developer");
  EXPECT_CALL(mock, listAllUsers()).WillOnce(Return(std::vector<User>{u}));
  EXPECT_CALL(mock, assignUserToIssue(_, _)).WillOnce(Return(true));
  EXPECT_CALL(mock, listAllIssues()).WillOnce(Return(std::vector<Issue>{Issue(1, "a", "t", 0)}));
  IssueTrackerView view(&mock);
  StreamRedirect sr("1\n1\n");
  view.assignUser();
  EXPECT_THAT(sr.getOutput(), HasSubstr("User assigned"));
}

TEST(IssueTrackerViewTest, UnassignUserSuccess) {
  MockController mock;
  EXPECT_CALL(mock, listAllIssues()).WillOnce(Return(std::vector<Issue>{Issue(7, "a", "b", 0)}));
  EXPECT_CALL(mock, unassignUserFromIssue(7)).WillOnce(Return(true));
  IssueTrackerView view(&mock);
  StreamRedirect sr("1\n");
  view.unassignUser();
  EXPECT_THAT(sr.getOutput(), HasSubstr("User unassigned"));
}

TEST(IssueTrackerViewTest, DisplayIssuePrintsComments) {
  MockController mock;
  Issue issue(1, "auth", "Title", 0);
  Comment c1(1, "auth", "Comment text", 0);
  EXPECT_CALL(mock, getIssue(1)).WillOnce(Return(issue));
  EXPECT_CALL(mock, getallComments(1)).WillOnce(Return(std::vector<Comment>{c1}));
  IssueTrackerView view(&mock);
  StreamRedirect sr;
  view.displayIssue(1);
  std::string out = sr.getOutput();
  EXPECT_THAT(out, HasSubstr("Title"));
  EXPECT_THAT(out, HasSubstr("Comment text"));
}

TEST(IssueTrackerViewTest, UpdateCommentPrintsSuccess) {
  MockController mock;
  Issue issue(1, "auth", "t", 0);
  EXPECT_CALL(mock, listAllIssues()).WillOnce(Return(std::vector<Issue>{issue}));
  EXPECT_CALL(mock, getIssue(1)).WillOnce(Return(issue));
  EXPECT_CALL(mock, getallComments(1)).WillOnce(Return(std::vector<Comment>{}));
  EXPECT_CALL(mock, updateComment(1, 1, "hi")).WillOnce(Return(true));
  IssueTrackerView view(&mock);
  StreamRedirect sr("1\n1\nhi\n");
  view.updateComment();
  EXPECT_THAT(sr.getOutput(), HasSubstr("Updated."));
}

TEST(IssueTrackerViewTest, DeleteCommentSuccess) {
  MockController mock;
  Issue issue(1, "a", "t", 0);
  EXPECT_CALL(mock, listAllIssues()).WillOnce(Return(std::vector<Issue>{issue}));
  EXPECT_CALL(mock, getIssue(1)).WillOnce(Return(issue));
  EXPECT_CALL(mock, getallComments(1)).WillOnce(Return(std::vector<Comment>{}));
  EXPECT_CALL(mock, deleteComment(1, 9)).WillOnce(Return(true));
  IssueTrackerView view(&mock);
  StreamRedirect sr("1\n9\n");
  view.deleteComment();
  EXPECT_THAT(sr.getOutput(), HasSubstr("Deleted."));
}

TEST(IssueTrackerViewTest, UpdateIssueSuccess) {
  MockController mock;
  Issue issue(1, "auth", "old", 0);
  EXPECT_CALL(mock, listAllIssues()).WillOnce(Return(std::vector<Issue>{issue}));
  EXPECT_CALL(mock, updateIssueField(1, "title", "newtitle")).WillOnce(Return(true));
  IssueTrackerView view(&mock);
  StreamRedirect sr("1\n1\nnewtitle\n");
  view.updateIssue();
  EXPECT_THAT(sr.getOutput(), HasSubstr("Updated successfully"));
}
