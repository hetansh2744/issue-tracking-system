#include "IssueTrackerController.h"

IssueTrackerController::IssueTrackerController(IssueRepository& repository)
    : repo(repository) {}

Issue IssueTrackerController::createIssue(const string& title, const string& desc,
const string& assignedTo){ 
    if (title.empty() || desc.empty()) {
        return Issue("", "", "");
    }
    Issue newIssue(title,desc,assignedTo);
     return repo.saveIssue(newIssue);
    }
