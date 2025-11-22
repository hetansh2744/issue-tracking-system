#include "IssueTrackerView.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace {

int readIntChoice() {
  int choice{};
  while (true) {
    if (std::cin >> choice) {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      return choice;
    }
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Invalid input. Please enter a number: ";
  }
}

std::string readLine(const std::string& prompt) {
  std::cout << prompt;
  std::string line;
  std::getline(std::cin, line);
  return line;
}

}  // namespace

IssueTrackerView::IssueTrackerView(IssueTrackerController* controller_in)
    : controller(controller_in) {}

void IssueTrackerView::displayMenu() {
  std::cout << "\n===== Issue Tracker Menu =====\n";
  std::cout << " 1) Create issue\n";
  std::cout << " 2) Update issue (title/description/status)\n";
  std::cout << " 3) Assign user to issue\n";
  std::cout << " 4) Unassign user from issue\n";
  std::cout << " 5) Delete issue\n";
  std::cout << " 6) List all issues\n";
  std::cout << " 7) List unassigned issues\n";
  std::cout << " 8) Find issues by user\n";
  std::cout << " 9) Add comment to issue\n";
  std::cout << "10) Update comment\n";
  std::cout << "11) Delete comment\n";
  std::cout << "12) Create user\n";
  std::cout << "13) List users\n";
  std::cout << "14) Remove user\n";
  std::cout << "15) Update user\n";
  std::cout << "16) Add tag to issue\n";
  std::cout << "17) Remove tag from issue\n";
  std::cout << "18) View issues by status\n";
  std::cout << " 0) Exit\n";
}

bool IssueTrackerView::ensureIssuesAvailable(const std::string& actionName) {
  auto issues = controller->listAllIssues();
  if (!issues.empty()) {
    return true;
  }

  std::cout << "There are currently no issues. "
               "You must create an issue before you can "
            << actionName << ".\n";
  std::cout << "Would you like to create a new issue now? (y/n): ";
  char c{};
  std::cin >> c;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  if (c == 'y' || c == 'Y') {
    createIssue();
    issues = controller->listAllIssues();
    return !issues.empty();
  }
  return false;
}

void IssueTrackerView::createIssue() {
  std::string title;
  std::string desc;
  std::string authorId;

  std::cout << "=== Create Issue ===\n";
  std::cout << "Enter title: ";
  std::getline(std::cin, title);
  if (title.empty()) {
    std::cout << "Title must not be empty. Aborting.\n";
    return;
  }

  std::cout << "Enter description (optional, can be empty): ";
  std::getline(std::cin, desc);

  std::cout << "Enter author user id: ";
  std::getline(std::cin, authorId);
  if (authorId.empty()) {
    std::cout << "Author id must not be empty. Aborting.\n";
    return;
  }

  Issue issue = controller->createIssue(title, desc, authorId);
  if (issue.getId() <= 0) {
    std::cout << "Failed to create issue. "
                 "Ensure title and author id are valid.\n";
    return;
  }

  std::cout << "Issue created with id " << issue.getId() << ".\n";

  // Optional initial status
  std::string status;
  std::cout << "Initial status "
               "(To Be Done / In Progress / Done) "
               "[leave empty for default 'To Be Done']: ";
  std::getline(std::cin, status);
  if (!status.empty()) {
    if (!controller->updateIssueField(issue.getId(), "status", status)) {
      std::cout << "Warning: failed to set initial status; "
                   "keeping default.\n";
    } else {
      std::cout << "Status set.\n";
    }
  }
}

void IssueTrackerView::updateIssue() {
  if (!ensureIssuesAvailable("update an issue")) {
    return;
  }

  std::cout << "=== Update Issue ===\n";
  int id = getissueId();
  if (id <= 0) {
    std::cout << "Invalid issue id.\n";
    return;
  }

  std::cout << "What would you like to update?\n";
  std::cout << " 1) Title\n";
  std::cout << " 2) Description\n";
  std::cout << " 3) Status\n";
  std::cout << "Choice: ";
  int choice = readIntChoice();

  std::string field;
  if (choice == 1) {
    field = "title";
  } else if (choice == 2) {
    field = "description";
  } else if (choice == 3) {
    field = "status";
  } else {
    std::cout << "Unknown choice.\n";
    return;
  }

  std::string value;
  if (field == "description") {
    std::cout << "Enter new description: ";
  } else if (field == "status") {
    std::cout << "Enter new status (To Be Done / In Progress / Done): ";
  } else {
    std::cout << "Enter new title: ";
  }
  std::getline(std::cin, value);

  if (!controller->updateIssueField(id, field, value)) {
    std::cout << "Failed to update issue.\n";
  } else {
    std::cout << "Issue updated.\n";
  }
}

void IssueTrackerView::assignUser() {
  if (!ensureIssuesAvailable("assign a user")) {
    return;
  }

  std::cout << "=== Assign User to Issue ===\n";
  int issueId = getissueId();
  if (issueId <= 0) {
    std::cout << "Invalid issue id.\n";
    return;
  }

  std::string userId = getuserId();
  if (userId.empty()) {
    std::cout << "No user selected.\n";
    return;
  }

  if (controller->assignUserToIssue(issueId, userId)) {
    std::cout << "User assigned.\n";
  } else {
    std::cout << "Failed to assign user. "
                 "Ensure both issue and user exist.\n";
  }
}

void IssueTrackerView::unassignUser() {
  if (!ensureIssuesAvailable("unassign a user")) {
    return;
  }

  std::cout << "=== Unassign User from Issue ===\n";
  int issueId = getissueId();
  if (issueId <= 0) {
    std::cout << "Invalid issue id.\n";
    return;
  }

  if (controller->unassignUserFromIssue(issueId)) {
    std::cout << "User unassigned.\n";
  } else {
    std::cout << "Failed to unassign user.\n";
  }
}

void IssueTrackerView::deleteIssue() {
  if (!ensureIssuesAvailable("delete an issue")) {
    return;
  }

  std::cout << "=== Delete Issue ===\n";
  int issueId = getissueId();
  if (issueId <= 0) {
    std::cout << "Invalid issue id.\n";
    return;
  }

  if (controller->deleteIssue(issueId)) {
    std::cout << "Issue deleted.\n";
  } else {
    std::cout << "Failed to delete issue.\n";
  }
}

void IssueTrackerView::listIssues() {
  std::cout << "=== All Issues ===\n";
  auto issues = controller->listAllIssues();
  if (issues.empty()) {
    std::cout << "No issues found.\n";
    return;
  }

  std::cout << "ID  | Title                      | Status        "
               "| Assignee\n";
  std::cout << "----+----------------------------+---------------"
               "+----------------\n";

  for (const auto& issue : issues) {
    std::string title = issue.getTitle();
    if (title.size() > 26) {
      title = title.substr(0, 23) + "...";
    }
    std::string status = issue.getStatus();
    std::string assignee = issue.hasAssignee() ? issue.getAssignedTo() : "-";

    std::cout << issue.getId() << "   | "
              << title
              << std::string(
                     28 - static_cast<int>(
                              std::min<std::size_t>(title.size(), 28)),
                     ' ')
              << "| " << status
              << std::string(
                     15 - static_cast<int>(
                              std::min<std::size_t>(status.size(), 15)),
                     ' ')
              << "| " << assignee << "\n";
  }
}

void IssueTrackerView::listUnassignedIssues() {
  std::cout << "=== Unassigned Issues ===\n";
  auto issues = controller->listAllUnassignedIssues();
  if (issues.empty()) {
    std::cout << "No unassigned issues.\n";
    return;
  }
  for (const auto& issue : issues) {
    std::cout << "Id: " << issue.getId()
              << " | Title: " << issue.getTitle()
              << " | Status: " << issue.getStatus() << "\n";
  }
}

void IssueTrackerView::findIssuesByUser() {
  std::cout << "=== Find Issues by User ===\n";
  std::string userId = getuserId();
  if (userId.empty()) {
    std::cout << "No user selected.\n";
    return;
  }

  auto issues = controller->findIssuesByUserId(userId);
  if (issues.empty()) {
    std::cout << "No issues found for user " << userId << ".\n";
    return;
  }

  for (const auto& issue : issues) {
    std::cout << "Id: " << issue.getId()
              << " | Title: " << issue.getTitle()
              << " | Status: " << issue.getStatus() << "\n";
  }
}

void IssueTrackerView::addComment() {
  if (!ensureIssuesAvailable("add a comment")) {
    return;
  }

  std::cout << "=== Add Comment to Issue ===\n";
  int issueId = getissueId();
  if (issueId <= 0) {
    std::cout << "Invalid issue id.\n";
    return;
  }

  std::string authorId = getuserId();
  if (authorId.empty()) {
    std::cout << "No user selected as author.\n";
    return;
  }

  std::string text = readLine("Enter comment text: ");
  if (text.empty()) {
    std::cout << "Comment text must not be empty.\n";
    return;
  }

  Comment c = controller->addCommentToIssue(issueId, text, authorId);
  if (c.getId() <= 0) {
    std::cout << "Failed to create comment.\n";
  } else {
    std::cout << "Comment created with id " << c.getId() << ".\n";
  }
}

void IssueTrackerView::updateComment() {
  if (!ensureIssuesAvailable("update a comment")) {
    return;
  }

  std::cout << "=== Update Comment ===\n";
  int issueId = getissueId();
  if (issueId <= 0) {
    std::cout << "Invalid issue id.\n";
    return;
  }

  auto comments = controller->getallComments(issueId);
  if (comments.empty()) {
    std::cout << "No comments for this issue.\n";
    return;
  }

  std::cout << "Comments:\n";
  for (const auto& c : comments) {
    std::cout << "  Id: " << c.getId()
              << " | Author: " << c.getAuthor()
              << " | Text: " << c.getText() << "\n";
  }

  std::cout << "Enter comment id to update: ";
  int commentId = readIntChoice();
  std::string newText = readLine("Enter new comment text: ");
  if (!controller->updateComment(issueId, commentId, newText)) {
    std::cout << "Failed to update comment.\n";
  } else {
    std::cout << "Comment updated.\n";
  }
}

void IssueTrackerView::deleteComment() {
  if (!ensureIssuesAvailable("delete a comment")) {
    return;
  }

  std::cout << "=== Delete Comment ===\n";
  int issueId = getissueId();
  if (issueId <= 0) {
    std::cout << "Invalid issue id.\n";
    return;
  }

  auto comments = controller->getallComments(issueId);
  if (comments.empty()) {
    std::cout << "No comments for this issue.\n";
    return;
  }

  std::cout << "Comments:\n";
  for (const auto& c : comments) {
    std::cout << "  Id: " << c.getId()
              << " | Author: " << c.getAuthor()
              << " | Text: " << c.getText() << "\n";
  }

  std::cout << "Enter comment id to delete: ";
  int commentId = readIntChoice();
  if (!controller->deleteComment(issueId, commentId)) {
    std::cout << "Failed to delete comment.\n";
  } else {
    std::cout << "Comment deleted.\n";
  }
}

void IssueTrackerView::createUser() {
  std::cout << "=== Create User ===\n";
  std::string name = readLine("Enter user name/id: ");
  std::string role = readLine("Enter role: ");

  User u = controller->createUser(name, role);
  if (u.getName().empty()) {
    std::cout << "Failed to create user.\n";
  } else {
    std::cout << "User created: " << u.getName()
              << " (role: " << u.getRole() << ")\n";
  }
}

void IssueTrackerView::listUsers() {
  std::cout << "=== All Users ===\n";
  auto users = controller->listAllUsers();
  if (users.empty()) {
    std::cout << "No users found.\n";
    return;
  }

  for (const auto& u : users) {
    std::cout << "Name: " << u.getName()
              << " | Role: " << u.getRole() << "\n";
  }
}

void IssueTrackerView::removeUser() {
  std::cout << "=== Remove User ===\n";
  std::string userId = getuserId();
  if (userId.empty()) {
    std::cout << "No user selected.\n";
    return;
  }

  if (controller->removeUser(userId)) {
    std::cout << "User removed.\n";
  } else {
    std::cout << "Failed to remove user.\n";
  }
}

void IssueTrackerView::updateUser() {
  std::cout << "=== Update User ===\n";
  std::string userId = getuserId();
  if (userId.empty()) {
    std::cout << "No user selected.\n";
    return;
  }

  std::cout << "What would you like to update?\n";
  std::cout << " 1) Name\n";
  std::cout << " 2) Role\n";
  std::cout << "Choice: ";
  int choice = readIntChoice();

  std::string field;
  if (choice == 1) {
    field = "name";
  } else if (choice == 2) {
    field = "role";
  } else {
    std::cout << "Unknown choice.\n";
    return;
  }

  std::string value = readLine("Enter new value: ");
  if (!controller->updateUser(userId, field, value)) {
    std::cout << "Failed to update user.\n";
  } else {
    std::cout << "User updated.\n";
  }
}

void IssueTrackerView::addComIssue() {
  if (!ensureIssuesAvailable("add a description comment")) {
    return;
  }

  std::cout << "=== Add Description Comment ===\n";
  int issueId = getissueId();
  if (issueId <= 0) {
    std::cout << "Invalid issue id.\n";
    return;
  }

  std::string desc = readLine("Enter description text: ");

  if (!controller->updateIssueField(issueId, "description", desc)) {
    std::cout << "Failed to update description.\n";
  } else {
    std::cout << "Description updated.\n";
  }
}

std::string IssueTrackerView::getuserId() {
  auto users = controller->listAllUsers();
  if (users.empty()) {
    std::cout << "No users exist yet.\n";
    std::cout << "You need at least one user. "
                 "Create a user first.\n";
    return "";
  }

  std::cout << "Select a user:\n";
  for (std::size_t i = 0; i < users.size(); ++i) {
    std::cout << " " << (i + 1) << ") "
              << users[i].getName()
              << " (role: " << users[i].getRole() << ")\n";
  }
  std::cout << "Choice (1-" << users.size() << "): ";
  int idx = getvalidInt(static_cast<int>(users.size()));
  if (idx <= 0 || static_cast<std::size_t>(idx) > users.size()) {
    return "";
  }
  return users[static_cast<std::size_t>(idx - 1)].getName();
}

int IssueTrackerView::getissueId() {
  auto issues = controller->listAllIssues();
  if (issues.empty()) {
    std::cout << "No issues available.\n";
    return -1;
  }

  std::cout << "Select an issue:\n";
  for (std::size_t i = 0; i < issues.size(); ++i) {
    std::cout << " " << (i + 1) << ") "
              << "Id: " << issues[i].getId()
              << " | Title: " << issues[i].getTitle()
              << " | Status: " << issues[i].getStatus() << "\n";
  }
  std::cout << "Choice (1-" << issues.size() << "): ";
  int idx = getvalidInt(static_cast<int>(issues.size()));
  if (idx <= 0 || static_cast<std::size_t>(idx) > issues.size()) {
    return -1;
  }
  return issues[static_cast<std::size_t>(idx - 1)].getId();
}

int IssueTrackerView::getvalidInt(int bound) {
  while (true) {
    int value{};
    if (std::cin >> value) {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      if (value >= 1 && value <= bound) {
        return value;
      }
      std::cout << "Please enter a number between 1 and "
                << bound << ": ";
    } else {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "Invalid input. Please enter a number: ";
    }
  }
}

std::vector<int> IssueTrackerView::displayIssue(int id) {
  std::vector<int> commentIdsShown;
  try {
    Issue issue = controller->getIssue(id);
    std::cout << "=== Issue Details ===\n";
    std::cout << "Id: " << issue.getId() << "\n";
    std::cout << "Title: " << issue.getTitle() << "\n";
    std::cout << "Author: " << issue.getAuthorId() << "\n";
    std::cout << "Status: " << issue.getStatus() << "\n";
    if (issue.hasAssignee()) {
      std::cout << "Assignee: " << issue.getAssignedTo() << "\n";
    } else {
      std::cout << "Assignee: (unassigned)\n";
    }

    auto tags = issue.getTags();
    if (!tags.empty()) {
      std::cout << "Tags: ";
      bool first = true;
      for (const auto& tag : tags) {
        if (!first) {
          std::cout << ", ";
        }
        std::cout << tag;
        first = false;
      }
      std::cout << "\n";
    }

    auto comments = controller->getallComments(id);
    if (!comments.empty()) {
      std::cout << "\nComments:\n";
      int displayIndex = 1;
      for (const auto& c : comments) {
        std::cout << "  [" << displayIndex << "] "
                  << "Id: " << c.getId()
                  << " | Author: " << c.getAuthor()
                  << " | " << c.getText() << "\n";
        commentIdsShown.push_back(c.getId());
        ++displayIndex;
      }
    } else {
      std::cout << "No comments.\n";
    }
  } catch (const std::out_of_range&) {
    std::cout << "Issue not found.\n";
  }
  return commentIdsShown;
}

void IssueTrackerView::addTag() {
  if (!ensureIssuesAvailable("add a tag")) {
    return;
  }

  std::cout << "=== Add Tag to Issue ===\n";
  int issueId = getissueId();
  if (issueId <= 0) {
    std::cout << "Invalid issue id.\n";
    return;
  }

  std::string tag = readLine("Enter tag: ");
  if (tag.empty()) {
    std::cout << "Tag must not be empty.\n";
    return;
  }

  if (controller->addTagToIssue(issueId, tag)) {
    std::cout << "Tag added.\n";
  } else {
    std::cout << "Failed to add tag.\n";
  }
}

void IssueTrackerView::removeTag() {
  if (!ensureIssuesAvailable("remove a tag")) {
    return;
  }

  std::cout << "=== Remove Tag from Issue ===\n";
  int issueId = getissueId();
  if (issueId <= 0) {
    std::cout << "Invalid issue id.\n";
    return;
  }

  Issue issue = controller->getIssue(issueId);
  auto tags = issue.getTags();
  if (tags.empty()) {
    std::cout << "This issue has no tags.\n";
    return;
  }

  std::vector<std::string> tagList(tags.begin(), tags.end());
  std::cout << "Existing tags:\n";
  for (std::size_t i = 0; i < tagList.size(); ++i) {
    std::cout << " " << (i + 1) << ") " << tagList[i] << "\n";
  }
  std::cout << "Choice (1-" << tagList.size() << "): ";
  int idx = getvalidInt(static_cast<int>(tagList.size()));
  if (idx <= 0 || static_cast<std::size_t>(idx) > tagList.size()) {
    std::cout << "Invalid choice.\n";
    return;
  }
  const std::string& tagToRemove = tagList[static_cast<std::size_t>(idx - 1)];
  if (controller->removeTagFromIssue(issueId, tagToRemove)) {
    std::cout << "Tag removed.\n";
  } else {
    std::cout << "Failed to remove tag.\n";
  }
}

void IssueTrackerView::viewIssuesByStatus() {
  std::cout << "=== Issues by Status ===\n";
  auto issues = controller->listAllIssues();
  if (issues.empty()) {
    std::cout << "No issues found.\n";
    return;
  }

  std::vector<Issue> todo;
  std::vector<Issue> inProgress;
  std::vector<Issue> done;
  std::vector<Issue> other;

  for (const auto& issue : issues) {
    const std::string status = issue.getStatus();
    if (status == "To Be Done") {
      todo.push_back(issue);
    } else if (status == "In Progress") {
      inProgress.push_back(issue);
    } else if (status == "Done") {
      done.push_back(issue);
    } else {
      other.push_back(issue);
    }
  }

  std::cout << "Summary:\n";
  std::cout << "  To Be Done : " << todo.size() << " issue(s)\n";
  std::cout << "  In Progress: " << inProgress.size() << " issue(s)\n";
  std::cout << "  Done       : " << done.size() << " issue(s)\n";
  if (!other.empty()) {
    std::cout << "  Other      : " << other.size() << " issue(s)\n";
  }

  auto printGroup = [](const std::string& label,
                       const std::vector<Issue>& group) {
    if (group.empty()) {
      return;
    }
    std::cout << "\n" << label << ":\n";
    for (const auto& issue : group) {
      std::cout << "  Id: " << issue.getId()
                << " | Title: " << issue.getTitle();
      if (issue.hasAssignee()) {
        std::cout << " | Assignee: " << issue.getAssignedTo();
      }
      std::cout << "\n";
    }
  };

  printGroup("To Be Done", todo);
  printGroup("In Progress", inProgress);
  printGroup("Done", done);
  printGroup("Other / Unknown", other);
}

void IssueTrackerView::run() {
  bool done = false;
  while (!done) {
    displayMenu();
    std::cout << "Enter choice: ";
    int choice = readIntChoice();
    switch (choice) {
      case 1:
        createIssue();
        break;
      case 2:
        updateIssue();
        break;
      case 3:
        assignUser();
        break;
      case 4:
        unassignUser();
        break;
      case 5:
        deleteIssue();
        break;
      case 6:
        listIssues();
        break;
      case 7:
        listUnassignedIssues();
        break;
      case 8:
        findIssuesByUser();
        break;
      case 9:
        addComment();
        break;
      case 10:
        updateComment();
        break;
      case 11:
        deleteComment();
        break;
      case 12:
        createUser();
        break;
      case 13:
        listUsers();
        break;
      case 14:
        removeUser();
        break;
      case 15:
        updateUser();
        break;
      case 16:
        addTag();
        break;
      case 17:
        removeTag();
        break;
      case 18:
        viewIssuesByStatus();
        break;
      case 0:
        done = true;
        std::cout << "Exiting Issue Tracker.\n";
        break;
      default:
        std::cout << "Unknown choice. Try again.\n";
        break;
    }
  }
}
