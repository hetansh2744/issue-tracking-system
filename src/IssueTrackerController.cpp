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
    Issue issue = repo->getIssue(id);

    if (field == "title") {
        issue.setTitle(value)
    } elseif(field == "description") {
        issue.setDescription(value)
    } elseif(field == "assignedTo") {
        issue.setassignedTo(value)
    } else {
        return false;
    }

    repo->saveIssue(issue) {}
}
