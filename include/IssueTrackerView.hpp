#include <iostream>
#include <string>
#include <vector>

#include "IssueTrackerController.hpp"
class IssueTrackerView {
 private:
    IssueTrackerController* controller;
    // Helper functions for menu options
    void displayMenu();
    void createIssue();
    void updateIssue();
    void assignUser();
    void unassignUser();
    void deleteIssue();
    void listIssues();
    void listUnassignedIssues();
    void findIssuesByUser();
    void addComment();
    void updateComment();
    void deleteComment();
    void createUser();
    void listUsers();
    void removeUser();
    void updateUser();
 public:
    // Constructor: takes a pointer to the controller
    explicit IssueTrackerView(IssueTrackerController* controller);
    void run();
};
