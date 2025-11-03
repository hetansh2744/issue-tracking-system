#include "IssueTrackerController.h"

IssueTrackerController::IssueTrackerController(IssueRepository* repository)
    : repo(repository) {}

Issue IssueTrackerController::createIssue(const std::string& title,
    const std::string& desc, const std::string& assignedTo) {
    if (title.empty() || desc.empty()) {
        return Issue("", "", "");
    }
    Issue newIssue(title, desc, assignedTo);
     return repo->saveIssue(newIssue);
}

bool IssueTrackerController::updateIssueField(int id, const std::string& field,
const std::string& value) {
    try {
    Issue issue = repo->getIssue(id);

    if (field == "title") {
        issue.setTitle(value);
    } else if (field == "description") {
        issue.setDescription(value);
    } else if (field == "assignedTo") {
        issue.setassignedTo(value);
    } else {
        return false;
    }

    repo->saveIssue(issue);
    return true;
} catch (const out_of_range& e) {
        return false;
}

bool IssueTrackerController::assignUserToIssue(int issueId, const string& userId) {
    try {
        // 1. Check existense of user
        repo->getUser(userId);

        // 2. Get Issue
        Issue issue = repo->getIssue(issueId);
        
        // 3. Update field
        issue.setassignedTo(userId);

        // 4. saveIssue
        repo->saveIssue(issue);
        return true;

    } catch (const out_of_range& e) {
        // Issue or User not found
        return false;
    }
}
