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
    std::cout << "16. Exit\n";
    std::cout << "Select an option: ";
}

//Prompts user for input and keeps program
//running after user inputs
void IssueTrackerView::run() {
    int choice = -1;
    while (choice != 16) {
        displayMenu();
        int length_display = 16;
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

//prompts user to create an issue
//assigning title description and who
// it is assigned to
void IssueTrackerView::createIssue() {
    std::string title, desc, assignedTo;

    do {
        std::cout << "Enter title:\n";
        std::getline(std::cin, title);
        if (title.empty()) {
            std::cout << "Input error: Title cannot be empty. "
                      << "Please try again.\n";
        }
    } while (title.empty());
    std::cout << "Enter description:\n";
    std::getline(std::cin, desc);

    std::cout << "Select author of Issue";
    assignedTo = getuserId();
    std::cout << "Issue assigned to user: " << assignedTo << std::endl;

    Issue issue = controller->createIssue(title, desc, assignedTo);
}

//Allows User to change certain aspects of
//a given issue of their choice
void IssueTrackerView::updateIssue() {
    int id;
    std::string field, value;

    id = getissueId();
    int num_of_options = 2;

    std::cout << "Select a field to change\n1) Title\n" <<
    "2) Description\n";
    int userinput = getvalidInt(num_of_options);
    switch (userinput) {
        case 1: field = "title";
            std::cout << "Enter new value: ";
            std::getline(std::cin, value); break;
        case 2: field = "description";
            std::cout << "Enter new value: ";
            std::getline(std::cin, value); break;
    }

    bool success = controller->updateIssueField(id, field, value);
    std::cout << (success ? "Updated successfully.\n" : "Update failed.\n");
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

//prompts user to delete an issue from
// user id
void IssueTrackerView::deleteIssue() {
    int issueid;
    issueid = getissueId();

    bool success = controller->deleteIssue(issueid);
    std::cout << (success ? "Deleted successfully.\n" : "Delete failed.\n");
}

//prints a list of all issues in the system
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
    time_t now = time(0);
    if (issue.getCreatedAt() > 0) {
      now = static_cast<std::time_t>(issue.getCreatedAt() / 1000);
    }
    char timeStr[26]; // ctime_r requires a buffer of at least 26 bytes
    ctime_r(&now, timeStr);
    std::cout << "Created: " << timeStr;

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
        std::cout << "ID: " << i.getId() <<
         " | Title: " << i.getTitle() << "\n";
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
    std::cout << "1) Owner\n2) Developer\n3) Maintainer\n" <<
    "Enter role: ";

    int userinput = getvalidInt(num_of_roles);
    switch (userinput) {
        case 1: role = "Owner"; break;
        case 2: role = "Developer"; break;
        case 3: role = "Maintainer"; break;
    }

    User u = controller->createUser(name, role);
    if (u.getName().empty())
        std::cout << "Failed to create user.\n";
    else
        std::cout << "User created: " << u.getName() << "\n";
}

//prints a list of all users created
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

//prompts a user to be remove
void IssueTrackerView::removeUser() {
  std::string userId;
  std::cout << "Enter User ID to remove: ";
  std::getline(std::cin, userId);
  bool success = controller->removeUser(userId);
  std::cout << (success ? "User removed.\n" : "Failed to remove user.\n");
}

//update new user with new name or role
// from user input
void IssueTrackerView:: updateUser() {
  int choice;
  std::string newname;
  std::string oldname;
  std::string newrole;
  std::cout << "What would you like to update?" << std::endl;
  std::cout << "1: User name" << std::endl;
  std::cout << "2: User Role" << std::endl;
  std::cin >> choice;
  if (choice == 1) {
    std::cout << "Enter old Username: " << std::endl;
    std::cin >> oldname;
    std::cout << "Enter new username: " << std::endl;
    std:: cin >> newname;
    controller ->updateUser(oldname, "name", newname);
    controller ->removeUser(oldname);
  } else {
    std::string name;
    std::cout << "what is username" <<std::endl;
    std::cin >> name;
    int num_of_roles = 3;
    std::cout << "1) Owner\n2) Developer\n3) Maintainer\n" <<
    "Enter role: ";

    int userinput = getvalidInt(num_of_roles);
    switch (userinput) {
        case 1: newrole = "Owner"; break;
        case 2: newrole = "Developer"; break;
        case 3: newrole = "Maintainer"; break;
    }
    controller ->updateUser(name, "role", newrole);
  }
}

//User id issue handling
std::string IssueTrackerView::getuserId() {
  while (true) {
    std::vector<User> users = controller->listAllUsers();
    if (users.empty()) {
      std::cout << "\n--- All Users ---\n";
      std::cout << "No users found, please create a user\n";
      createUser();
      continue;
    }

    std::cout << "\n--- All Users ---\n";
    for (size_t i = 0; i < users.size(); ++i) {
      std::cout << (i + 1) << ") " << users[i].getName() << "\n";
    }

    int selection = getvalidInt(static_cast<int>(users.size()));
    if (selection == -1) {
      std::cout << "Unable to select a user. Please try again.\n";
      continue;
    }

    return users[selection - 1].getName();
  }
}

//Issue ID user handling
int IssueTrackerView::getissueId() {
  std::vector<Issue> issues = controller->listAllIssues();
  if (issues.empty()) {
    std::cout << "No issues found.\n";
    return -1;
  }

  int num_of_issues = 1;
  std::vector<int> issueids;

  std::cout << "\n--- All Issues ---\n";
  for (const auto& issue : issues) {
    std::cout << num_of_issues << "). ID: " << issue.getId() << " -- " <<
    "Title: " << issue.getTitle() << "\n";
    num_of_issues++;
    issueids.push_back(issue.getId());
  }

  num_of_issues -1;
  int userinput = getvalidInt(num_of_issues - 1);
  return issueids[userinput -1];
}

//error handling so user cant input unused ints
int IssueTrackerView::getvalidInt(int bound) {
    if (bound < 1) {
        std::cerr << "Error: The validation bound " <<
        "must be 1 or greater." << std::endl;
        return -1;
    }

    int selection;

    while (true) {
        std::cout << "Please enter an integer between 1 and " << bound << ": ";

        if (std::cin >> selection) {
            if (selection >= 1 && selection <= bound) {
                std::cout << "Input accepted: " << selection << "\n";
                std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(), '\n');
                return selection;
            } else {
                std::cout << "Input error: " << selection <<
                " is outside the valid range (1 to " << bound <<
                "). Please try again." << "\n";
            }

        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Input error: Invalid input." <<
            "Please enter a whole number."
                      << std::endl;
        }
    }
}

//displays a certian issue by id
void IssueTrackerView:: displayIssue(int id) {
  Issue iss = controller->getIssue(id);
  std::vector <Comment> comments = controller->getallComments(id);
    std::cout << "ID: " << iss.getId() << "\n";
    std::cout << "Author: " << iss.getAuthorId() << "\n";
    std::cout << "Title: " << iss.getTitle() << "\n";
    std::cout << "Amount of Comments: " << iss.getCommentIds().size()-1 << "\n";
    time_t now = time(0);
    if (iss.getCreatedAt() > 0) {
      now = static_cast<std::time_t>(iss.getCreatedAt() / 1000);
    }
    char timeStr[26]; // ctime_r requires a buffer of at least 26 bytes
    ctime_r(&now, timeStr);
    std::cout << "Created: " << timeStr;

    int i = 1;
    for (auto it : comments) {
      std::cout << i << it.getText() <<std::endl;
      i++;
    }
}

//add comments to an issue
void IssueTrackerView:: addComIssue() {
  int issueId;
  std::string text;
  std::string authorID;
  listIssues();
  issueId = getissueId();
  displayIssue(issueId);
  std::cout << std::endl;
  std::cout << "Comment text add here" << std::endl;
  std::getline(std::cin, text);
  authorID = getuserId();
  controller->addCommentToIssue(issueId, text, authorID);
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

//updates a certian comment from comment id
// and issue id
void IssueTrackerView::updateComment() {
    int id;
    std::string text;
    int issueid;
    issueid = getissueId();
    displayIssue(issueid);
    std::cout << "Enter Comment ID: ";
    std::cin >> id;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    do {
        std::cout << "Enter new text: ";
        std::getline(std::cin, text);
        if (text.empty()) {
            std::cout << "Input error: Comment text cannot be empty. "
                      << "Please try again.\n";
        }
    } while (text.empty());

    bool success = controller->updateComment(issueid, id, text);
    std::cout << (success ? "Updated.\n" : "Failed to update.\n");
}

//deletes a specific comment by comment id
//also displays all comments in the issue
void IssueTrackerView::deleteComment() {
    int issueid;
    int comid;
    listIssues();
    issueid = getissueId();
    controller->getIssue(issueid);
    displayIssue(issueid);
    std::cout << "Enter Comment ID: ";
    std::cin >> comid;
    bool success = controller->deleteComment(issueid, comid);
    std::cout << (success ? "Deleted.\n" : "Failed to delete.\n");
}
