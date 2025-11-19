#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <ctime>
#include <iostream>
#include <sstream>

#include "IssueTrackerView.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

class MockIssueTrackerController : public IssueTrackerController {
 public:
  explicit MockIssueTrackerController(IssueRepository* repo = nullptr)
      : IssueTrackerController(repo) {}

  MOCK_METHOD(Issue, createIssue,
              (const std::string& title, const std::string& description,
               const std::string& authorId),
              (override));
  MOCK_METHOD(bool, updateIssueField,
              (int issueId, const std::string& field,
               const std::string& value),
              (override));
  MOCK_METHOD(bool, assignUserToIssue, (int issueId, const std::string& userId),
              (override));
  MOCK_METHOD(bool, unassignUserFromIssue, (int issueId), (override));
  MOCK_METHOD(bool, deleteIssue, (int issueId), (override));
  MOCK_METHOD(Issue, getIssue, (const int issueId), (override));
  MOCK_METHOD(std::vector<Comment>, getallComments, (int issueId), (override));
  MOCK_METHOD(std::vector<Issue>, listAllIssues, (), (override));
  MOCK_METHOD(std::vector<Issue>, listAllUnassignedIssues, (), (override));
  MOCK_METHOD(std::vector<Issue>, findIssuesByUserId,
              (const std::string& userId), (override));
  MOCK_METHOD(Comment, addCommentToIssue,
              (int issueId, const std::string& text,
               const std::string& authorId),
              (override));
  MOCK_METHOD(bool, updateComment,
              (int issueId, int commentId, const std::string& text),
              (override));
  MOCK_METHOD(bool, deleteComment, (int issueId, int commentId), (override));
  MOCK_METHOD(User, createUser,
              (const std::string& name, const std::string& role), (override));
  MOCK_METHOD(std::vector<User>, listAllUsers, (), (override));
  MOCK_METHOD(bool, removeUser, (const std::string& userId), (override));
  MOCK_METHOD(bool, updateUser,
              (const std::string& userId, const std::string& field,
               const std::string& value),
              (override));
};

class IssueTrackerViewTest : public ::testing::Test {
 protected:
  void SetUp() override {
    mockController = std::make_unique<NiceMock<MockIssueTrackerController>>(
        nullptr);
    view = std::make_unique<IssueTrackerView>(mockController.get());
  }

  void TearDown() override {
    std::cin.rdbuf(originalCinBuffer);
    std::cout.rdbuf(originalCoutBuffer);
  }

  void simulateUserInput(const std::string& input) {
    inputStream.str(input);
    inputStream.clear();
    originalCinBuffer = std::cin.rdbuf();
    std::cin.rdbuf(inputStream.rdbuf());
  }

  void restoreCin() {
    if (originalCinBuffer) {
      std::cin.rdbuf(originalCinBuffer);
    }
  }

  std::string captureOutput(std::function<void()> func) {
    std::stringstream outputStream;
    originalCoutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(outputStream.rdbuf());

    func();

    std::cout.rdbuf(originalCoutBuffer);
    return outputStream.str();
  }

  std::unique_ptr<NiceMock<MockIssueTrackerController>> mockController;
  std::unique_ptr<IssueTrackerView> view;
  std::stringstream inputStream;
  std::streambuf* originalCinBuffer = nullptr;
  std::streambuf* originalCoutBuffer = nullptr;
};

TEST_F(IssueTrackerViewTest, DisplayMenuShowsAllOptions) {
  auto output = captureOutput([this]() {
    view->displayMenu();
  });

  EXPECT_THAT(output, testing::HasSubstr("Create Issue"));
  EXPECT_THAT(output, testing::HasSubstr("Update Issue Field"));
  EXPECT_THAT(output, testing::HasSubstr("Assign User to Issue"));
  EXPECT_THAT(output, testing::HasSubstr("Unassign User from Issue"));
  EXPECT_THAT(output, testing::HasSubstr("Delete Issue"));
  EXPECT_THAT(output, testing::HasSubstr("List All Issues"));
  EXPECT_THAT(output, testing::HasSubstr("Find Issues by User ID"));
  EXPECT_THAT(output, testing::HasSubstr("Add Comment to Issue"));
  EXPECT_THAT(output, testing::HasSubstr("Update Comment"));
  EXPECT_THAT(output, testing::HasSubstr("Delete Comment"));
  EXPECT_THAT(output, testing::HasSubstr("Create User"));
  EXPECT_THAT(output, testing::HasSubstr("List All Users"));
  EXPECT_THAT(output, testing::HasSubstr("Remove User"));
  EXPECT_THAT(output, testing::HasSubstr("Update User"));
  EXPECT_THAT(output, testing::HasSubstr("List Unassigned Issues"));
  EXPECT_THAT(output, testing::HasSubstr("Exit"));
}

TEST_F(IssueTrackerViewTest, GetValidIntValidInput) {
  simulateUserInput("3\n");
  int result = view->getvalidInt(5);
  restoreCin();
  EXPECT_EQ(result, 3);
}

TEST_F(IssueTrackerViewTest, GetValidIntInvalidThenValidInput) {
  simulateUserInput("10\n3\n");
  int result = view->getvalidInt(5);
  restoreCin();
  EXPECT_EQ(result, 3);
}

TEST_F(IssueTrackerViewTest, GetValidIntNonNumericThenValidInput) {
  simulateUserInput("abc\n2\n");
  int result = view->getvalidInt(5);
  restoreCin();
  EXPECT_EQ(result, 2);
}

TEST_F(IssueTrackerViewTest, GetUserIdWithExistingUsers) {
  std::vector<User> users = {User("user1", "Developer"),
                             User("user2", "Owner")};
  EXPECT_CALL(*mockController, listAllUsers())
      .WillOnce(Return(users));

  simulateUserInput("1\n");
  std::string result = view->getuserId();
  restoreCin();
  EXPECT_EQ(result, "user1");
}

TEST_F(IssueTrackerViewTest, GetIssueIdWithExistingIssues) {
  std::vector<Issue> issues = {Issue(1, "author1", "Issue 1"),
                               Issue(2, "author2", "Issue 2")};
  EXPECT_CALL(*mockController, listAllIssues())
      .WillOnce(Return(issues));

  simulateUserInput("1\n");
  int result = view->getissueId();
  restoreCin();
  EXPECT_EQ(result, 1);
}

TEST_F(IssueTrackerViewTest, GetIssueIdWithNoIssues) {
  std::vector<Issue> issues;
  EXPECT_CALL(*mockController, listAllIssues())
      .WillOnce(Return(issues));

  int result = view->getissueId();
  EXPECT_EQ(result, -1);
}

TEST_F(IssueTrackerViewTest, CreateIssueSuccess) {
  EXPECT_CALL(*mockController, listAllUsers())
      .WillOnce(Return(std::vector<User>{User("testuser", "Developer")}));

  Issue expectedIssue(1, "testuser", "Test Title");
  EXPECT_CALL(*mockController, createIssue("Test Title", "Test Description",
                                           "testuser"))
      .WillOnce(Return(expectedIssue));

  simulateUserInput("Test Title\nTest Description\n1\n");

  auto output = captureOutput([this]() {
    view->createIssue();
  });
  restoreCin();

  EXPECT_THAT(output, testing::HasSubstr("Issue assigned to user: testuser"));
}

TEST_F(IssueTrackerViewTest, UpdateIssueTitleSuccess) {
  std::vector<Issue> issues = {Issue(1, "author1", "Old Title")};
  EXPECT_CALL(*mockController, listAllIssues())
      .WillOnce(Return(issues));
  EXPECT_CALL(*mockController, updateIssueField(1, "title", "New Title"))
      .WillOnce(Return(true));

  simulateUserInput("1\n1\nNew Title\n");

  auto output = captureOutput([this]() {
    view->updateIssue();
  });
  restoreCin();

  EXPECT_THAT(output, testing::HasSubstr("Updated successfully"));
}

TEST_F(IssueTrackerViewTest, UpdateIssueDescriptionSuccess) {
  std::vector<Issue> issues = {Issue(1, "author1", "Test Issue")};
  EXPECT_CALL(*mockController, listAllIssues())
      .WillOnce(Return(issues));
  EXPECT_CALL(*mockController, updateIssueField(1, "description", "New Desc"))
      .WillOnce(Return(true));

  simulateUserInput("1\n2\nNew Desc\n");

  auto output = captureOutput([this]() {
    view->updateIssue();
  });
  restoreCin();

  EXPECT_THAT(output, testing::HasSubstr("Updated successfully"));
}

TEST_F(IssueTrackerViewTest, AssignUserSuccess) {
  std::vector<Issue> issues = {Issue(1, "author1", "Test Issue")};
  std::vector<User> users = {User("user1", "Developer")};

  EXPECT_CALL(*mockController, listAllIssues())
      .WillOnce(Return(issues));
  EXPECT_CALL(*mockController, listAllUsers())
      .WillOnce(Return(users));
  EXPECT_CALL(*mockController, assignUserToIssue(1, "user1"))
      .WillOnce(Return(true));

  simulateUserInput("1\n1\n");

  auto output = captureOutput([this]() {
    view->assignUser();
  });
  restoreCin();

  EXPECT_THAT(output, testing::HasSubstr("User assigned"));
}

TEST_F(IssueTrackerViewTest, UnassignUserSuccess) {
  std::vector<Issue> issues = {Issue(1, "author1", "Test Issue")};
  EXPECT_CALL(*mockController, listAllIssues())
      .WillOnce(Return(issues));
  EXPECT_CALL(*mockController, unassignUserFromIssue(1))
      .WillOnce(Return(true));

  simulateUserInput("1\n");

  auto output = captureOutput([this]() {
    view->unassignUser();
  });
  restoreCin();

  EXPECT_THAT(output, testing::HasSubstr("User unassigned"));
}

TEST_F(IssueTrackerViewTest, DeleteIssueSuccess) {
  std::vector<Issue> issues = {Issue(1, "author1", "Test Issue")};
  EXPECT_CALL(*mockController, listAllIssues())
      .WillOnce(Return(issues));
  EXPECT_CALL(*mockController, deleteIssue(1))
      .WillOnce(Return(true));

  simulateUserInput("1\n");

  auto output = captureOutput([this]() {
    view->deleteIssue();
  });
  restoreCin();

  EXPECT_THAT(output, testing::HasSubstr("Deleted successfully"));
}

TEST_F(IssueTrackerViewTest, ListIssuesWithResults) {
  std::vector<Issue> issues = {
      Issue(1, "author1", "Issue 1"),
      Issue(2, "author2", "Issue 2")};

  EXPECT_CALL(*mockController, listAllIssues())
      .WillOnce(Return(issues));

  auto output = captureOutput([this]() {
    view->listIssues();
  });

  EXPECT_THAT(output, testing::HasSubstr("All Issues"));
  EXPECT_THAT(output, testing::HasSubstr("Issue 1"));
  EXPECT_THAT(output, testing::HasSubstr("Issue 2"));
}

TEST_F(IssueTrackerViewTest, ListIssuesEmpty) {
  EXPECT_CALL(*mockController, listAllIssues())
      .WillOnce(Return(std::vector<Issue>{}));

  auto output = captureOutput([this]() {
    view->listIssues();
  });

  EXPECT_THAT(output, testing::HasSubstr("No issues found"));
}

TEST_F(IssueTrackerViewTest, ListUnassignedIssuesWithResults) {
  Issue unassignedIssue(1, "author1", "Unassigned");
  std::vector<Issue> issues = {unassignedIssue};

  EXPECT_CALL(*mockController, listAllUnassignedIssues())
      .WillOnce(Return(issues));

  auto output = captureOutput([this]() {
    view->listUnassignedIssues();
  });

  EXPECT_THAT(output, testing::HasSubstr("Unassigned Issues"));
  EXPECT_THAT(output, testing::HasSubstr("Unassigned"));
}

TEST_F(IssueTrackerViewTest, FindIssuesByUser) {
  std::vector<Issue> issues = {Issue(1, "user1", "User1 Issue")};
  EXPECT_CALL(*mockController, findIssuesByUserId("user1"))
      .WillOnce(Return(issues));

  simulateUserInput("user1\n");

  auto output = captureOutput([this]() {
    view->findIssuesByUser();
  });
  restoreCin();

  EXPECT_THAT(output, testing::HasSubstr("User1 Issue"));
}

TEST_F(IssueTrackerViewTest, CreateUserSuccess) {
  User expectedUser("newuser", "Developer");
  EXPECT_CALL(*mockController, createUser("newuser", "Developer"))
      .WillOnce(Return(expectedUser));

  simulateUserInput("newuser\n2\n");

  auto output = captureOutput([this]() {
    view->createUser();
  });
  restoreCin();

  EXPECT_THAT(output, testing::HasSubstr("User created: newuser"));
}

TEST_F(IssueTrackerViewTest, ListUsersWithResults) {
  std::vector<User> users = {
      User("user1", "Developer"),
      User("user2", "Owner")};

  EXPECT_CALL(*mockController, listAllUsers())
      .WillOnce(Return(users));

  auto output = captureOutput([this]() {
    view->listUsers();
  });

  EXPECT_THAT(output, testing::HasSubstr("All Users"));
  EXPECT_THAT(output, testing::HasSubstr("user1"));
  EXPECT_THAT(output, testing::HasSubstr("user2"));
}

TEST_F(IssueTrackerViewTest, RemoveUserSuccess) {
  EXPECT_CALL(*mockController, removeUser("olduser"))
      .WillOnce(Return(true));

  simulateUserInput("olduser\n");

  auto output = captureOutput([this]() {
    view->removeUser();
  });
  restoreCin();

  EXPECT_THAT(output, testing::HasSubstr("User removed"));
}

TEST_F(IssueTrackerViewTest, DisplayIssueShowsStoredTimestamp) {
  const Issue::TimePoint creationTs = 1'700'000'000'000;  // deterministic epoch
  Issue issue(42, "author1", "Detailed View", creationTs);
  issue.addComment(1);  // ensure comment sizing remains non-negative

  std::vector<Comment> comments = {
      Comment(1, "author1", "Initial description", creationTs)};

  EXPECT_CALL(*mockController, getIssue(42))
      .WillOnce(Return(issue));
  EXPECT_CALL(*mockController, getallComments(42))
      .WillOnce(Return(comments));

  auto output = captureOutput([this]() {
    view->displayIssue(42);
  });

  time_t expectedTime = static_cast<std::time_t>(creationTs / 1000);
  char expectedStr[26];
  ctime_r(&expectedTime, expectedStr);
  EXPECT_THAT(output,
    testing::HasSubstr(std::string("Created: ") + expectedStr));
}
