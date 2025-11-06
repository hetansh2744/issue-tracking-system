#include "IssueTrackerView.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define private public
#undef private

using ::testing::_;
using ::testing::HasSubstr;
using ::testing::Return;
using ::testing::StrictMock;

class MockIssueTrackerController : public IssueTrackerController {
 public:
  MockIssueTrackerController() : IssueTrackerController(nullptr) {}

  MOCK_METHOD(Issue, createIssue,
      (const std::string&, const std::string&, const std::string&), (override));
  MOCK_METHOD(bool, updateIssueField,
      (int, const std::string&, const std::string&), (override));
  MOCK_METHOD(bool, assignUserToIssue, (int, const std::string&), (override));
  MOCK_METHOD(bool, unassignUserFromIssue, (int), (override));
  MOCK_METHOD(bool, deleteIssue, (int), (override));
  MOCK_METHOD(std::vector<Issue>, listAllIssues, (), (override));
  MOCK_METHOD(std::vector<Issue>, listAllUnassignedIssues, (), (override));
  MOCK_METHOD(std::vector<Issue>, findIssuesByUserId,
      (const std::string&), (override));
  MOCK_METHOD(Comment, addCommentToIssue,
      (int, const std::string&, const std::string&), (override));

  MOCK_METHOD(bool, updateComment,
      (int, int, const std::string&), (override));

  MOCK_METHOD(bool, deleteComment,
      (int, int), (override));

  MOCK_METHOD(User, createUser,
      (const std::string&, const std::string&), (override));
  MOCK_METHOD(std::vector<User>, listAllUsers, (), (override));
  MOCK_METHOD(bool, removeUser, (const std::string&), (override));
};

class ConsoleCapture {
 public:
  explicit ConsoleCapture(const std::string& input = "")
      : input_(input), cin_backup_(nullptr), cout_backup_(std::cout.rdbuf()) {
    std::cout.rdbuf(output_.rdbuf());
    if (!input.empty()) {
      cin_backup_ = std::cin.rdbuf(input_.rdbuf());
    }
  }

  ~ConsoleCapture() {
    std::cout.rdbuf(cout_backup_);
    if (cin_backup_ != nullptr) {
      std::cin.rdbuf(cin_backup_);
    }
  }

  std::string str() const { return output_.str(); }

 private:
  std::istringstream input_;
  std::ostringstream output_;
  std::streambuf* cin_backup_;
  std::streambuf* cout_backup_;
};

class IssueTrackerViewTest : public ::testing::Test {
 protected:
  StrictMock<MockIssueTrackerController> mockController;
  std::unique_ptr<IssueTrackerView> view;

  void SetUp() override {
    view = std::make_unique<IssueTrackerView>(&mockController);
  }

  std::string runWithInput(const std::string& input,
      void (IssueTrackerView::*action)()) {
    ConsoleCapture capture(input);
    (view.get()->*action)();
    return capture.str();
  }

  std::string run(void (IssueTrackerView::*action)()) {
    return runWithInput("", action);
  }
};


TEST_F(IssueTrackerViewTest, AssignUser_PrintsSuccessMessage) {
    EXPECT_CALL(mockController, listAllIssues())
        .WillOnce(Return(std::vector<Issue>{Issue(1, "reporter", "bug", 0)}));
    EXPECT_CALL(mockController, listAllUsers())
        .WillOnce(Return(std::vector<User>{User("user42", "dev")}));

    EXPECT_CALL(mockController, assignUserToIssue(1, "user42"))
      .WillOnce(Return(true));

    const std::string output = runWithInput("1\n1\n",
      &IssueTrackerView::assignUser);

    EXPECT_THAT(output, HasSubstr("User assigned"));
}

TEST_F(IssueTrackerViewTest, ListIssues_PrintsDetails) {
  Issue withDescription(1, "author", "Title1", 0);
  Comment description(2, "author", "Detailed desc", 0);
  withDescription.addComment(description);
  withDescription.setDescriptionCommentId(description.getId());
  withDescription.assignTo("dev");

  Issue second(3, "reporter", "Title2", 0);
  second.setDescriptionCommentId(99);

  EXPECT_CALL(mockController, listAllIssues())
      .WillOnce(Return(std::vector<Issue>{withDescription, second}));

  const std::string output = run(&IssueTrackerView::listIssues);

  EXPECT_THAT(output, HasSubstr("Title1"));
  EXPECT_THAT(output, HasSubstr("Detailed desc"));
  EXPECT_THAT(output, HasSubstr("Assigned To: dev"));
  EXPECT_THAT(output, HasSubstr("Description comment ID: 99"));
  EXPECT_THAT(output, HasSubstr("(unassigned)"));
}

TEST_F(IssueTrackerViewTest, ListUnassignedIssues_PrintsMessageWhenEmpty) {
  EXPECT_CALL(mockController, listAllUnassignedIssues())
      .WillOnce(Return(std::vector<Issue>{}));

  const std::string output = run(&IssueTrackerView::listUnassignedIssues);

  EXPECT_THAT(output, HasSubstr("No unassigned issues"));
}

TEST_F(IssueTrackerViewTest, DeleteIssue_PrintsSuccessMessage) {
    EXPECT_CALL(mockController, listAllIssues())
        .WillOnce(Return(std::vector<Issue>{Issue(5, "reporter", "bug", 0)}));

    EXPECT_CALL(mockController, deleteIssue(5))
      .WillOnce(Return(true));

    const std::string output = runWithInput("1\n",
      &IssueTrackerView::deleteIssue);

    EXPECT_THAT(output, HasSubstr("Deleted successfully"));
}

TEST_F(IssueTrackerViewTest, FindIssuesByUser_PrintsMatches) {
  Issue first(1, "reporter", "First", 0);
  Issue second(2, "reporter", "Second", 0);

  EXPECT_CALL(mockController, findIssuesByUserId("user777"))
      .WillOnce(Return(std::vector<Issue>{first, second}));

  const std::string output = runWithInput("user777\n",
      &IssueTrackerView::findIssuesByUser);

  EXPECT_THAT(output, HasSubstr("ID: 1 | Title: First"));
  EXPECT_THAT(output, HasSubstr("ID: 2 | Title: Second"));
}

TEST_F(IssueTrackerViewTest, AddComment_PrintsSuccessMessage) {
    EXPECT_CALL(mockController, listAllIssues())
        .WillOnce(Return(std::vector<Issue>{Issue(9, "reporter", "bug", 0)}));
    EXPECT_CALL(mockController, listAllUsers())
        .WillOnce(Return(std::vector<User>{User("author", "dev")}));

    EXPECT_CALL(mockController,
      addCommentToIssue(9, "Great job", "author"))
      .WillOnce(Return(Comment(3, "author", "Great job", 0)));

    const std::string output = runWithInput("1\nauthor\nGreat job\n",
      &IssueTrackerView::addComment);

    EXPECT_THAT(output, HasSubstr("Comment added successfully"));
}

TEST_F(IssueTrackerViewTest, CreateUser_PrintsSuccessMessage) {
  EXPECT_CALL(mockController, createUser("alice", "Developer"))
      .WillOnce(Return(User("alice", "Developer")));

  const std::string output = runWithInput("alice\nDeveloper\n",
      &IssueTrackerView::createUser);

  EXPECT_THAT(output, HasSubstr("User created: alice"));
}

TEST_F(IssueTrackerViewTest, ListUsers_PrintsAllUsers) {
  EXPECT_CALL(mockController, listAllUsers())
      .WillOnce(Return(std::vector<User>{
          User("anna", "lead"), User("bob", "qa")}));

  const std::string output = run(&IssueTrackerView::listUsers);

  EXPECT_THAT(output, HasSubstr("anna"));
  EXPECT_THAT(output, HasSubstr("bob"));
}

TEST_F(IssueTrackerViewTest, UpdateIssue_PrintsSuccessMessage) {
    EXPECT_CALL(mockController, listAllIssues())
        .WillOnce(Return(std::vector<Issue>{Issue(7, "reporter", "old", 0)}));

    EXPECT_CALL(mockController,
      updateIssueField(7, "title", "New title"))
      .WillOnce(Return(true));

    const std::string output = runWithInput("1\n1\nNew title\n",
      &IssueTrackerView::updateIssue);

    EXPECT_THAT(output, HasSubstr("Updated successfully"));
}

TEST_F(IssueTrackerViewTest, UnassignUser_PrintsSuccessMessage) {
    EXPECT_CALL(mockController, listAllIssues())
        .WillOnce(Return(std::vector<Issue>{Issue(11, "reporter", "bug", 0)}));

    EXPECT_CALL(mockController, unassignUserFromIssue(11))
      .WillOnce(Return(true));

    const std::string output = runWithInput("1\n",
      &IssueTrackerView::unassignUser);

    EXPECT_THAT(output, HasSubstr("User unassigned"));
}

TEST_F(IssueTrackerViewTest, Run_ShowsMenuAndExits) {
    const std::string output = runWithInput("16\n", &IssueTrackerView::run);

    EXPECT_THAT(output, HasSubstr("Issue Tracker Menu"));
    EXPECT_THAT(output, HasSubstr("Goodbye!"));
}
