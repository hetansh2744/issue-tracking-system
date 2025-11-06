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
// assigns a user to a specific issue by id
bool IssueTrackerController::assignUserToIssue(int issueId,
    const std::string& user_name) {
    try {
        repo->getUser(user_name);  // ensure user exists
        Issue issue = repo->getIssue(issueId);
        issue.assignTo(user_name);  // must exist in Issue.hpp
        repo->saveIssue(issue);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

// removes a user assignment from a specific issue
bool IssueTrackerController::unassignUserFromIssue(int issueId) {
    try {
        Issue issue = repo->getIssue(issueId);
        issue.unassign();  // must exist in Issue.hpp
        repo->saveIssue(issue);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

// deletes an issue by id
bool IssueTrackerController::deleteIssue(int issueId) {
    return repo->deleteIssue(issueId);
}

// retrieves all comments for a given issue
std::vector<Comment> IssueTrackerController::getallComments(
    int issueId) {
    return repo->getAllComments(issueId);
}

// retrieves a specific comment from an issue
Comment IssueTrackerController::getComment(
    int issueId, int commentId) {
    return repo->getComment(issueId, commentId);
}

// adds a comment to an issue, returns the saved comment
Comment IssueTrackerController::addCommentToIssue(int issueId,
    const std::string& text, const std::string& authorId) {
    try {
        if (text.empty() || authorId.empty()) {
            return Comment(0, "", "");
        }

        Issue issue = repo->getIssue(issueId);
        repo->getUser(authorId);

        Comment newComment(0, authorId, text, 0);
        Comment savedComment = repo->saveComment(issueId, newComment);

        issue.addComment(savedComment.getId());
        repo->saveIssue(issue);

        return savedComment;
    } catch (const std::out_of_range&) {
        return Comment(0, "", "");
    }
}

// updates the text of an existing comment
bool IssueTrackerController::updateComment(int issueId,
     int commentId, const std::string& newText) {
    try {
        Comment comment = repo->getComment(issueId,
            commentId);
        comment.setText(newText);
        repo->saveComment(issueId, comment);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

// deletes a comment from an issue and removes its reference
bool IssueTrackerController::deleteComment(int issueId, int commentId) {
    try {
        repo->getComment(issueId, commentId);

        bool deleted = repo->deleteComment(issueId, commentId);

        if (deleted) {
            Issue issue = repo->getIssue(issueId);
            issue.removeComment(commentId);
            repo->saveIssue(issue);
            return true;
        }
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

// creates a new user with a name and role
User IssueTrackerController::createUser(const std::string& name,
    const std::string& role) {
    if (name.empty() || role.empty()) {
        return User("", "");
    }
    User newUser(name, role);
    return repo->saveUser(newUser);
}

// updates a user’s name or role based on a specified field
bool IssueTrackerController::updateUser(const std::string& userId,
    const std::string& field, const std::string& value) {
    try {
        User userObj = repo->getUser(userId);
        if (field == "name") {
            userObj.setName(value);
        } else if (field == "role") {
            userObj.setRole(value);
        } else {
            return false;
        }
        repo->saveUser(userObj);
        return true;
    } catch (...) {
        return false;
    }
}

// removes a user from the repository
bool IssueTrackerController::removeUser(const std::string& user_name) {
    return repo->deleteUser(user_name);
}

// returns a list of all issues
std::vector<Issue> IssueTrackerController::listAllIssues() {
    return repo->listIssues();
}

// returns all issues that do not have an assigned user
std::vector<Issue> IssueTrackerController::listAllUnassignedIssues() {
    return repo->listAllUnassigned();
}

// finds all issues assigned to a specific user
std::vector<Issue> IssueTrackerController::findIssuesByUserId(
    const std::string& user_name) {
    return repo->findIssues(user_name);
}

// returns a list of all users
std::vector<User> IssueTrackerController::listAllUsers() {
    return repo->listAllUsers();
}
