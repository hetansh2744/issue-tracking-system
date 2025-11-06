#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <cctype>

#include "IssueTrackerController.hpp"

class IssueTrackerView {
  private:
    IssueTrackerController* controller;
  public:
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
    void addComIssue();
    std::string getuserId();
    int getissueId();
    int getvalidInt(int bound);
 public:
    void displayIssue(int id);
    // Constructor: takes a pointer to the controller
    explicit IssueTrackerView(IssueTrackerController* controller);
    void run();
};
