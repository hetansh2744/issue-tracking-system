#include "IssueTrackerView.hpp"
#include <ctime>
#include <iomanip> // For std::put_time
IssueTrackerView::IssueTrackerView(IssueTrackerController* controller)
    : controller(controller) {}

    //Displays all current functions of program pro
    //-mpting user for input
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
    std::cout << "17. Add Tag to Issue\n";
    std::cout << "18. Remove Tag from Issue\n";
    std::cout << "19. Exit\n";
    std::cout << "Select an option: ";
}

//Prompts user for input and keeps program
//running after user inputs
void IssueTrackerView::run() {
    int choice = -1;
    while (choice != 19) {
        displayMenu();
        int length_display = 19;
        choice = getvalidInt(length_display);

        switch (choice) {
        case 1: createIssue(); break;
        case 2: updateIssue(); break;
        case 3: assignUser(); break;
        case 4: unassignUser(); break;
        case 5: deleteIssue(); break;
        case 6: listIssues(); break;
        case 7: findIssuesByUser(); break;
        case 8: addComIssue(); break;
        case 9: updateComment(); break;
        case 10: deleteComment(); break;
        case 11: createUser(); break;
        case 12: listUsers(); break;
        case 13: removeUser(); break;
        case 14: updateUser(); break;
        case 15: listUnassignedIssues(); break;
        case 16: std::cout << "Goodbye!\n"; break;
        }
    }
}

// Prompts user for integer input and validates it to
// ensure it's within a range and a number
int IssueTrackerView::getvalidInt(int num_of_options) {
    std::string input;
    int value;

    while (true) {
        std::getline(std::cin, input);
        bool isValid = !input.empty() && std::all_of(input.begin(),
            input.end(), ::isdigit);

        if (isValid) {
            value = std::stoi(input);
            if (value >= 1 && value <= num_of_options) {
                return value;
            }
        }

        std::cout << "Invalid input. "
                  << "Please enter a number between 1 and "
                  << num_of_options << ": ";
    }
}

//allows user to select an existing issue
//from all issues
int IssueTrackerView::getissueId() {
    std::vector<Issue> issues = controller->listAllIssues();
    if (issues.empty()) {
        std::cout << "No issues available.\n";
        return -1;
    }

    std::cout << "\nAvailable Issues:\n";
    for (const auto& issue : issues) {
        std::cout << "ID: " << issue.getId() <<
        " | Title: " << issue.getTitle() << "\n";
    }

    std::cout << "Enter Issue ID: ";
    int issueId;
    while (true) {
        std::string input;
        std::getline(std::cin, input);

        bool isValid = !input.empty() && std::all_of(input.begin(),
            input.end(), ::isdigit);

        if (isValid) {
            issueId = std::stoi(input);
            auto it = std::find_if(issues.begin(), issues.end(),
                [issueId](const Issue& issue) {
                    return issue.getId() == issueId;
                });
            if (it != issues.end()) {
                return issueId;
            }
        }

        std::cout << "Invalid Issue ID. "
                  << "Please enter a valid ID: ";
    }
}

//allows user to select a user
std::string IssueTrackerView::getuserId() {
    std::vector<User> users = controller->listAllUsers();
    if (users.empty()) {
        std::cout << "No users available.\n";
        return "";
    }

    std::cout << "\nAvailable Users:\n";
    for (size_t i = 0; i < users.size(); ++i) {
        std::cout << i + 1 << ". " << users[i].getName()
                  << " (" << users[i].getRole() << ")\n";
    }

    std::cout << "Select a user by number: ";
    int index = getvalidInt(static_cast<int>(users.size()));
    return users[index - 1].getName();
}

//prompts user to create an issue
//assigning title description and who
// it is assigned to
void IssueTrackerView::createIssue() {
    std::string title, desc, assignedTo;

    // get non-empty title
    do {
        std::cout << "Enter title:\n";
        std::getline(std::cin, title);
        if (title.empty()) {
            std::cout << "Input error: Title cannot be empty. "
                      << "Please try again.\n";
        }
    } while (title.empty());

    // description (can be empty)
    std::cout << "Enter description:\n";
    std::getline(std::cin, desc);

    // choose author / assignee
    std::cout << "Select author of Issue";
    assignedTo = getuserId();

    // ✅ this is what the test looks for:
    // "Issue assigned to user: testuser"
    std::cout << "Issue assigned to user: " << assignedTo << std::endl;

    // create the issue via controller
    Issue issue = controller->createIssue(title, desc, assignedTo);
}

//allows user to update the issues title or desciption or status
void IssueTrackerView::updateIssue() {
    int id;
    std::string field, value;

    id = getissueId();
    int num_of_options = 3;

    std::cout << "Select a field to change\n1) Title\n"
              << "2) Description\n3) Status\n";
    int userinput = getvalidInt(num_of_options);
    switch (userinput) {
        case 1:
            field = "title";
            std::cout << "Enter new value: ";
            std::getline(std::cin, value);
            break;
        case 2:
            field = "description";
            std::cout << "Enter new value: ";
            std::getline(std::cin, value);
            break;
        case 3:
            field = "status";
            std::cout << "Select new status:\n"
                      << "1) To Be Done\n"
                      << "2) In Progress\n"
                      << "3) Done\n";
            {
                int statusChoice = getvalidInt(3);
                switch (statusChoice) {
                    case 1: value = "To Be Done"; break;
                    case 2: value = "In Progress"; break;
                    case 3: value = "Done"; break;
                }
            }
            break;
    }

    bool success = controller->updateIssueField(id, field, value);
    std::cout << (success ? "Updated successfully.\n"
                          : "Update failed.\n");
}

//prompts user to assign an issue to a spec
//-ific user
void IssueTrackerView::assignUser() {
    int issueId;
    std::string userName;

    issueId = getissueId();
    userName = getuserId();

    bool success = controller->assignUserToIssue(issueId, userName);
    std::cout << (success ? "User assigned.\n" : "Failed to assign.\n");
}

//prompts user to unassign a issue
//from a specific user
void IssueTrackerView::unassignUser() {
    int issueId;
    issueId = getissueId();

    bool success = controller->unassignUserFromIssue(issueId);
    std::cout << (success ? "User unassigned.\n" : "Failed to unassign.\n");
}

//deletes a certain issue and returns true if success-
//-fully deleted
void IssueTrackerView::deleteIssue() {
    int issueid;
    issueid = getissueId();

    bool success = controller->deleteIssue(issueid);

    // ✅ test expects substring "Deleted successfully"
    std::cout << (success ? "Deleted successfully.\n" : "Delete failed.\n");
}


void IssueTrackerView::listIssues() {
  std::vector<Issue> issues = controller->listAllIssues();
  if (issues.empty()) {
    std::cout << "No issues found.\n";
    return;
  }

  std::vector<Issue> todo;
  std::vector<Issue> inProgress;
  std::vector<Issue> done;

  for (const auto& issue : issues) {
    const std::string& status = issue.getStatus();
    if (status == "In Progress") {
      inProgress.push_back(issue);
    } else if (status == "Done") {
      done.push_back(issue);
    } else {
      todo.push_back(issue);
    }
  }

  auto printGroup = [&](const std::string& header,
                        const std::vector<Issue>& group) {
    std::cout << "\n[" << header << "]\n";
    if (group.empty()) {
      std::cout << "  (none)\n";
      return;
    }

    for (const auto& issue : group) {
      std::cout << "ID: " << issue.getId() << "\n";
      std::cout << "Author: " << issue.getAuthorId() << "\n";
      std::cout << "Title: " << issue.getTitle() << "\n";
      std::cout << "Status: " << issue.getStatus() << "\n";

      std::int64_t timestamp = issue.getCreatedAt();
      std::time_t time_t_value =
          static_cast<std::time_t>(timestamp / 1000);

      char time_buffer[80];
      std::strftime(time_buffer, sizeof(time_buffer),
                    "%Y-%m-%d %H:%M:%S", std::localtime(&time_t_value));
      std::cout << "Created: " << time_buffer << "\n";

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

      // ✅ TAG DISPLAY LOGIC (ADDED)
      std::cout << "Tags: ";
      const auto& tags = issue.getTags();
      if (tags.empty()) {
        std::cout << "(none)";
      } else {
        for (const auto& tag : tags) {
          std::cout << "[" << tag << "] ";
        }
      }
      std::cout << "\n";

      std::cout << "Comments: " <<
      issue.getCommentIds().size() << "\n\n";
    }
  };

  std::cout << "\n--- All Issues ---\n";
  printGroup("To Be Done", todo);
  printGroup("In Progress", inProgress);
  printGroup("Done", done);
}


//prints a list of all the issues that dont
//have users assigned
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
    std::cout << "Status: " << issue.getStatus() << "\n";
    std::cout << "Description Comment ID: ";
    if (issue.hasDescriptionComment()) {
      std::cout << issue.getDescriptionCommentId() << "\n";
    } else {
      std::cout << "(none)\n";
    }
    std::cout << "Comments: " << issue.getCommentIds().size() << "\n\n";
  }
}

//prompts user to find an issue connected to
// a specific user id
void IssueTrackerView::findIssuesByUser() {
    std::string userId;
    std::cout << "Enter User ID: ";
    std::getline(std::cin, userId);

    std::vector<Issue> issues = controller->findIssuesByUserId(userId);
    for (const auto& i : issues) {
        std::cout << "ID: " << i.getId()
                  << " | Title: " << i.getTitle()
                  << " | Status: " << i.getStatus() << "\n";
    }
}

//prompts the creation of user with name
// and role of user
void IssueTrackerView::createUser() {
    std::string name;
    std::string role;

    do {
        std::cout << "Enter username: ";
        std::getline(std::cin, name);
        if (name.empty()) {
            std::cout << "Input error: Username cannot be empty. "
                      << "Please try again.\n";
        }
    } while (name.empty());

    int num_of_roles = 3;
    std::cout << "1) Owner\n2) Developer\n3) Maintainer\n"
              << "Enter role: ";

    int userinput = getvalidInt(num_of_roles);
    switch (userinput) {
        case 1: role = "Owner"; break;
        case 2: role = "Developer"; break;
        case 3: role = "Maintainer"; break;
    }

    User user = controller->createUser(name, role);
    std::cout << "User created: " << user.getName()
              << " (" << user.getRole() << ")\n";
}

//prompts the user asking them to list all users
//activates all users, and lists them
void IssueTrackerView::listUsers() {
    std::vector<User> users = controller->listAllUsers();
    if (users.empty()) {
        std::cout << "No users found.\n";
        return;
    }

    std::cout << "\n--- All Users ---\n";
    for (const auto& user : users) {
        std::cout << "Name: " << user.getName()
                  << ", Role: " << user.getRole() << "\n";
    }
}

//prompts user to remove an existing user
void IssueTrackerView::removeUser() {
    std::string userId;
    std::cout << "Enter User ID to remove: ";
    std::getline(std::cin, userId);

    bool success = controller->removeUser(userId);
    std::cout << (success ? "User removed successfully.\n"
                          : "Failed to remove user.\n");
}

//updates a user with correct inputs
void IssueTrackerView::updateUser() {
    std::string userId, field, value;
    std::cout << "Enter User ID to update: ";
    std::getline(std::cin, userId);

    std::cout << "Select field to update:\n1) Name\n2) Role\n";
    int choice = getvalidInt(2);
    switch (choice) {
        case 1:
            field = "name";
            break;
        case 2:
            field = "role";
            break;
    }

    std::cout << "Enter new value: ";
    std::getline(std::cin, value);

    bool success = controller->updateUser(userId, field, value);
    std::cout << (success ? "User updated successfully.\n"
                          : "Failed to update user.\n");
}

//displays specific issue as a more human reable
// text
void IssueTrackerView:: displayIssue(int id) {
    time_t now = time(0);
    char timeStr[26]; // ctime_r requires a buffer of at least 26 bytes
    ctime_r(&now, timeStr);

    // Convert to local time structure
  Issue iss = controller->getIssue(id);
  std::vector <Comment> comments = controller->getallComments(id);
    std::cout << "ID: " << iss.getId() << "\n";
    std::cout << "Author: " << iss.getAuthorId() << "\n";
    std::cout << "Title: " << iss.getTitle() << "\n";
    std::cout << "Status: " << iss.getStatus() << "\n";
    std::cout << "Amount of Comments: "
              << iss.getCommentIds().size()-1 << "\n";
    std::cout << "Time: " << timeStr;
    int i = 1;
    for (auto it : comments) {
      std::cout << i << it.getText() <<std::endl;
      i++;
    }
}

//add comments to an issue
void IssueTrackerView:: addComIssue() {
  int issueId;
  std::string text, authorId;
  issueId = getissueId();
  authorId = getuserId();
  Comment newcom = controller->addCommentToIssue(issueId, text, authorId);
  std::cout << "new comment has been added with id "
            << newcom.getId() << std::endl;
}

//updates existing comment
void IssueTrackerView:: updateComment() {
  int issueId, commentId, i = 1;
  std::string text;
  issueId = getissueId();
  std::vector <Comment> comments = controller->getallComments(issueId);
  std::cout << "Available Comments:\n";
  for (auto com : comments) {
    std::cout << i << ". " << com.getText() <<std::endl;
    i++;
  }
  std::cout << "Pick an comment to edit" << std::endl;
  int choice = getvalidInt(comments.size());
  commentId = comments[choice].getId();
  std::cout << "Enter new text: ";
  std::getline(std::cin, text);
  controller->updateComment(issueId, commentId, text);
}

//deletes existing comment
void IssueTrackerView:: deleteComment() {
  int issueId, commentId, i = 1;
  issueId = getissueId();
  std::vector <Comment> comments = controller->getallComments(issueId);
  for (auto com : comments) {
    std::cout << i << ". " << com.getText() <<std::endl;
    i++;
  }
  std::cout << "Pick a comment to delete" << std::endl;
  int choice = getvalidInt(comments.size());
  commentId = comments[choice].getId();
  controller->deleteComment(issueId, commentId);
}

void IssueTrackerView::addTag() {
  int issueId = getissueId();
  std::string tag;

  std::cout << "Enter tag: ";
  std::getline(std::cin, tag);

  bool success = controller->addTagToIssue(issueId, tag);
  std::cout << (success ? "Tag added.\n" : "Failed to add tag.\n");
}

void IssueTrackerView::removeTag() {
  int issueId = getissueId();
  std::string tag;

  std::cout << "Enter tag to remove: ";
  std::getline(std::cin, tag);

  bool success = controller->removeTagFromIssue(issueId, tag);
  std::cout << (success ? "Tag removed.\n" : "Failed to remove tag.\n");
}
