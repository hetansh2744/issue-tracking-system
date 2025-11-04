#include "IssueTrackerController.h"

IssueTrackerController::IssueTrackerController
(IssueRepository* repository) : repo(repository) {}

Issue IssueTrackerController::createIssue(const std::string& title,
const std::string& desc, const std::string& assignedTo) {
    std::cout << "running";
    if (title.empty() || desc.empty()) {
        return Issue("", "", "");
    }
    Issue newIssue(title, desc, assignedTo);
    return repo->saveIssue(newIssue);
}

bool IssueTrackerController::updateIssueField(int id,
                                              const std::string& field,
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
    } catch (const std::out_of_range& e) {
        return false;
    }
}

bool IssueTrackerController::assignUserToIssue(
    int issueId, const std::string& userId) {
    try {
        repo->getUser(userId);
        Issue issue = repo->getIssue(issueId);
        issue.setassignedTo(userId);
        repo->saveIssue(issue);
        return true;
    } catch (const std::out_of_range& e) {
        return false;
    }
}

bool IssueTrackerController::unassignUserFromIssue(int issueId) {
    try {
        Issue issue = repo->getIssue(issueId);
        issue.setassignedTo("");
        repo->saveIssue(issue);
        return true;
    } catch (const std::out_of_range& e) {
        return false;
    }
}

bool IssueTrackerController::deleteIssue(int issueId) {
    return repo->deleteIssue(issueId);
}

std::vector<Issue> IssueTrackerController::listAllIssues() {
    return repo->listIssues();
}

std::vector<Issue> IssueTrackerController::findIssuesByUserId(
    const std::string& userId) {
    return repo->findIssues(userId);
}

Comment IssueTrackerController::addCommentToIssue(int issueId,
                                                  const std::string& text,
                                                  const std::string& authorId) {
    try {
        if (text.empty()) {
            return Comment(0, "", "");
        }

        Issue issue = repo->getIssue(issueId);
        repo->getUser(authorId);

        Comment newComment(issueId, authorId, text);
        newComment.id = 0;
        Comment savedComment = repo->saveComment(newComment);

        issue.addComment(savedComment.id);
        repo->saveIssue(issue);

        return savedComment;
    } catch (const std::out_of_range& e) {
        return Comment(0, "", "");
    }
}

bool IssueTrackerController::updateComment(int commentId,
     const std::string& newText) {
    try {
        if (newText.empty()) {
            return false;
        }

        Comment comment = repo->getComment(commentId);
        comment.text = newText;
        repo->saveComment(comment);
        return true;
    } catch (const std::out_of_range& e) {
        return false;
    }
}

bool IssueTrackerController::deleteComment(int commentId) {
    try {
        Comment comment = repo->getComment(commentId);
        int issueId = comment.issueId;
        bool deleted = repo->deleteComment(commentId);
        if (deleted) {
            Issue issue = repo->getIssue(issueId);
            issue.removeComment(commentId);
            repo->saveIssue(issue);
            return true;
        }
        return false;
    } catch (const std::out_of_range& e) {
        return false;
    }
}

User IssueTrackerController::createUser(const std::string& name) {
    if (name.empty()) {
        return User("", "");
    }

    User newUser(name, "");
    return repo->saveUser(newUser);
}

bool IssueTrackerController::removeUser(const std::string& userId) {
    return repo->deleteUser(userId);
}

std::vector<User> IssueTrackerController::listAllUsers() {
    return repo->listAllUsers();
}
