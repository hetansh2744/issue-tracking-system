#include "IssueTrackerController.hpp"

#include <algorithm>
#include <exception>
#include <stdexcept>

IssueTrackerController::IssueTrackerController(IssueRepository* repository)
    : repo(repository) {}

Issue IssueTrackerController::createIssue(const std::string& title,
    const std::string& desc, const std::string& author_id) {
    if (title.empty() || desc.empty() || author_id.empty()) {
        return Issue(0, "", "");
    }

    // 1. Create the Issue (no description yet)
    Issue newIssue(0, author_id, title, 0);
    Issue savedIssue = repo->saveIssue(newIssue);

    // 2. Create the first Comment = Description
    Comment descComment(0, author_id, desc, 0);
    Comment savedComment = repo->saveComment(savedIssue.getId(), descComment);

    // 3. Link comment #1 as description
    savedIssue.setDescriptionCommentId(savedComment.getId());
    repo->saveIssue(savedIssue);

    return savedIssue;
}

Issue IssueTrackerController::getIssue(const int issueId) {
    return repo->getIssue(issueId);
}

bool IssueTrackerController::updateIssueField(int id, const std::string& field,
    const std::string& value) {
    try {
        Issue issue = repo->getIssue(id);

        if (field == "title") {
            issue.setTitle(value);
            repo->saveIssue(issue);
            return true;

        } else if (field == "description") {
            // Get the description comment
            const Comment* descComment =
            issue.findCommentById(issue.getDescriptionCommentId());

            if (!descComment) {
                // No description comment yet, so create one
                Comment newDesc(0, issue.getAuthorId(), value, 0);
                Comment savedDesc =
                    repo->saveComment(issue.getId(), newDesc);
                issue.setDescriptionCommentId(savedDesc.getId());
                repo->saveIssue(issue);
            } else {
                // Update existing description comment
                Comment editable = *descComment;
                editable.setText(value);
                repo->saveComment(issue.getId(), editable);
            }
            return true;

        } else {
            return false;
        }
    } catch (const std::out_of_range&) {
        return false;
    } catch (const std::invalid_argument&) {
        return false;
    }
}

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

bool IssueTrackerController::deleteIssue(int issueId) {
    return repo->deleteIssue(issueId);
}

std::vector<Comment> IssueTrackerController::getallComments(
    int issueId) {
    return repo->getAllComments(issueId);
}

Comment IssueTrackerController::getComment(
    int issueId, int commentId) {
    return repo->getComment(issueId, commentId);
}

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

bool IssueTrackerController::deleteComment(int issueId,
    int commentId) {
    try {
        Comment comment = repo->getComment(issueId,
            commentId);
        int issueId = comment.getId();

        bool deleted = repo->deleteComment(commentId);
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

User IssueTrackerController::createUser(const std::string& name,
    const std::string& role) {
    if (name.empty() || role.empty()) {
        return User("", "");
    }
    User newUser(name, role);
    return repo->saveUser(newUser);
}

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

bool IssueTrackerController::removeUser(const std::string& user_name) {
    return repo->deleteUser(user_name);
}

std::vector<Issue> IssueTrackerController::listAllIssues() {
    return repo->listIssues();
}

std::vector<Issue> IssueTrackerController::listAllUnassignedIssues() {
    return repo->listAllUnassigned();
}

std::vector<Issue> IssueTrackerController::findIssuesByUserId(
    const std::string& user_name) {
    return repo->findIssues(user_name);
}

std::vector<User> IssueTrackerController::listAllUsers() {
    return repo->listAllUsers();
}
