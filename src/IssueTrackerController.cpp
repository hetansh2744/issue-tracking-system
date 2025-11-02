#include "IssueTrackerController.h"

IssueTrackerController::IssueTrackerController(IssueRepository& repository)
    : repo(repository) {}

Issue IssueTrackerController::createIssue(const string& title,
    const string& desc, const string& assignedTo) {
    if (title.empty() || desc.empty()) {
        return Issue("", "", "");
    }
    Issue newIssue(title, desc, assignedTo);
     return repo->saveIssue(newIssue);
}

bool IssueTrackerController::updateIssueField(int id, const string& field,
const string& value) {
    Issue issue = repo->getIssue(id);

    if (field == "title") {
        issue.setTitle(value)
    } elseif (field == "description") {
        issue.setDescription(value)
    } elseif (field == "assignedTo") {
        issue.setassignedTo(value)
    } else {
        return false;
    }

    repo->saveIssue(issue) {}
}