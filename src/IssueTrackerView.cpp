#include "IssueTrackerView.hpp"

IssueTrackerView::IssueTrackerView(IssueTrackerController* controller)
    : controller(controller) {}

void IssueTrackerView::displayMenu() {
    std::cout << "\n=== Issue Tracker Menu ===\n";
    std::cout << "1. Create Issue\n";
    std::cout << "2. Update Issue Field\n";
    std::cout << "3. Assign User to Issue\n";
    std::cout << "4. Unassign User from Issue\n";
    std::cout << "5. Delete Issue\n";
    std::cout << "6. List All Issues\n";
    std::cout << "7. Find Issues by User ID\n";
    std::cout << "8. Add Comment to Issue\n";
    std::cout << "9. Update Comment\n";
    std::cout << "10. Delete Comment\n";
    std::cout << "11. Create User\n";
    std::cout << "12. List All Users\n";
    std::cout << "13. Remove User\n";
    std::cout << "14. Update User\n";
    std::cout << "15. List Unassigned Issues\n";
    //implement!

    std::cout << "0. Exit\n";
    std::cout << "Select an option: ";
}

void IssueTrackerView::run() {
    int choice = -1;
    while (choice != 0) {
        displayMenu();
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
        case 1: createIssue(); break;
        case 2: updateIssue(); break;
        case 3: assignUser(); break;
        case 4: unassignUser(); break;
        case 5: deleteIssue(); break;
        case 6: listIssues(); break;
        case 7: findIssuesByUser(); break;
        case 8: addComment(); break;
        case 9: updateComment(); break;
        case 10: deleteComment(); break;
        case 11: createUser(); break;
        case 12: listUsers(); break;
        case 13: removeUser(); break;
        case 14: updateUser(); break;
        case 15: listUnassignedIssues(); break;
        case 0: std::cout << "Goodbye!\n"; break;
        default: std::cout << "Invalid choice.\n"; break;
        }
    }
}

void IssueTrackerView::createIssue() {
    std::string title, desc, assignedTo;
    std::cout << "Enter title: ";
    std::getline(std::cin, title);
    std::cout << "Enter description: ";
    std::getline(std::cin, desc);
    std::cout << "Assign to (userId, optional): ";
    std::getline(std::cin, assignedTo);

    Issue issue = controller->createIssue(title, desc, assignedTo);
}

void IssueTrackerView::updateIssue() {
    int id;
    std::string field, value;
    std::cout << "Enter Issue ID: ";
    std::cin >> id;
    std::cin.ignore();
    std::cout << "Enter field to update (title/description/assignedTo): ";
    std::getline(std::cin, field);
    std::cout << "Enter new value: ";
    std::getline(std::cin, value);

    bool success = controller->updateIssueField(id, field, value);
    std::cout << (success ? "Updated successfully.\n" : "Update failed.\n");
}

void IssueTrackerView::assignUser() {
    int issueId;
    std::string userId;
    std::cout << "Enter Issue ID: ";
    std::cin >> issueId;
    std::cin.ignore();
    std::cout << "Enter User ID: ";
    std::getline(std::cin, userId);

    bool success = controller->assignUserToIssue(issueId, userId);
    std::cout << (success ? "User assigned.\n" : "Failed to assign.\n");
}

void IssueTrackerView::unassignUser() {
    int issueId;
    std::cout << "Enter Issue ID: ";
    std::cin >> issueId;

    bool success = controller->unassignUserFromIssue(issueId);
    std::cout << (success ? "User unassigned.\n" : "Failed to unassign.\n");
}

void IssueTrackerView::deleteIssue() {
    int id;
    std::cout << "Enter Issue ID to delete: ";
    std::cin >> id;

    bool success = controller->deleteIssue(id);
    std::cout << (success ? "Deleted successfully.\n" : "Delete failed.\n");
}

void IssueTrackerView::listIssues() {
  std::vector<Issue> issues = controller->listAllIssues();
  if (issues.empty()) {
    std::cout << "No issues found.\n";
    return;
  }

  std::cout << "\n--- All Issues ---\n";
  for (const auto& issue : issues) {
    std::cout << "ID: " << issue.getId() << "\n";
    std::cout << "Author: " << issue.getAuthorId() << "\n";
    std::cout << "Title: " << issue.getTitle() << "\n";

    if (issue.hasDescriptionComment()) {
      const Comment* desc =
          issue.findCommentById(issue.getDescriptionCommentId());
      if (desc) {
        std::cout << "Description: " << desc->getText() << "\n";
      } else {
        std::cout << "Description comment ID: "
                  << issue.getDescriptionCommentId() << "\n";
      }
    } else {
      std::cout << "Description: (none)\n";
    }

    if (issue.hasAssignee()) {
      std::cout << "Assigned To: " << issue.getAssignedTo() << "\n";
    } else {
      std::cout << "Assigned To: (unassigned)\n";
    }

    std::cout << "Comments:" << issue.getCommentIds().size() << "\n\n";
  }
}

void IssueTrackerView::listUnassignedIssues() {
  std::vector<Issue> issues = controller->listAllUnassignedIssues();
  if (issues.empty()) {
    std::cout << "No unassigned issues.\n";
    return;
  }

  std::cout << "\n--- Unassigned Issues ---\n";
  for (const auto& issue : issues) {
    std::cout << "ID: " << issue.getId() << "\n";
    std::cout << "Author: " << issue.getAuthorId() << "\n";
    std::cout << "Title: " << issue.getTitle() << "\n";
    std::cout << "Description Comment ID: ";
    if (issue.hasDescriptionComment()) {
      std::cout << issue.getDescriptionCommentId() << "\n";
    } else {
      std::cout << "(none)\n";
    }
    std::cout << "Comments: " << issue.getCommentIds().size() << "\n\n";
  }
}

void IssueTrackerView::findIssuesByUser() {
    std::string userId;
    std::cout << "Enter User ID: ";
    std::getline(std::cin, userId);

    std::vector<Issue> issues = controller->findIssuesByUserId(userId);
    for (const auto& i : issues) {
        std::cout << "ID: " << i.getId() <<
         " | Title: " << i.getTitle() << "\n";
    }
}

void IssueTrackerView::addComment() {
    int issueId;
    std::string text, authorId;
    std::cout << "Enter Issue ID: ";
    std::cin >> issueId;
    std::cin.ignore();
    std::cout << "Enter Author ID: ";
    std::getline(std::cin, authorId);
    std::cout << "Enter comment text: ";
    std::getline(std::cin, text);

    Comment c = controller->addCommentToIssue(issueId, text, authorId);
    if (c.getText().empty())
        std::cout << "Failed to add comment.\n";
    else
        std::cout << "Comment added successfully.\n";
}

void IssueTrackerView::updateComment() {
    int id;
    std::string text;
    std::cout << "Enter Comment ID: ";
    std::cin >> id;
    std::cin.ignore();
    std::cout << "Enter new text: ";
    std::getline(std::cin, text);

    bool success = controller->updateComment(id, text);
    std::cout << (success ? "Updated.\n" : "Failed to update.\n");
}

void IssueTrackerView::deleteComment() {
    int id;
    std::cout << "Enter Comment ID: ";
    std::cin >> id;

    bool success = controller->deleteComment(id);
    std::cout << (success ? "Deleted.\n" : "Failed to delete.\n");
}

void IssueTrackerView::createUser() {
    std::string name;
    std::string role;
    std::cout << "Enter username: ";
    std::getline(std::cin, name);
    std::cout << "Enter role: ";
    std::getline(std::cin, role);
    User u = controller->createUser(name, role);
    if (u.getName().empty())
        std::cout << "Failed to create user.\n";
    else
        std::cout << "User created: " << u.getName() << "\n";
}

void IssueTrackerView::listUsers() {
  std::vector<User> users = controller->listAllUsers();
  std::cout << "\n--- All Users ---\n";
  if (users.empty()) {
    std::cout << "No users found.\n";
    return;
  }
  for (auto user : users)
    std::cout << "Name: " << user.getName() << " | Role: " << user.getRole()
              << "\n";
}

void IssueTrackerView::removeUser() {
  std::string userId;
  std::cout << "Enter User ID to remove: ";
  std::getline(std::cin, userId);
  bool success = controller->removeUser(userId);
  std::cout << (success ? "User removed.\n" : "Failed to remove user.\n");
}

void IssueTrackerView:: updateUser() {
    int choice;
    std::cout <<"1: Assigned Issues " << std::endl;
    std::cout <<"2: " << std::endl;
    std::cout <<"3: " << std::endl;
    std::cout <<"4: " << std::endl;
    std::cin >> choice;
}
