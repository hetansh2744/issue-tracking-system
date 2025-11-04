#include "IssueTrackerController.h"

IssueTrackerController::IssueTrackerController(IssueRepository* repository)
    : repo(repository) {}

Issue IssueTrackerController::createIssue(const std::string& title,
    const std::string& desc, const std::string& assignedTo) {
        std::cout<< "running " ;
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
    } catch (const std::out_of_range& e) {
        return false;
    }
}
bool IssueTrackerController::assignUserToIssue(int issueId,
    const std::string& userId) {
    try {
        // 1. Check existense of user
        repo->getUser(userId);

        // 2. Get Issue
        Issue issue = repo->getIssue(issueId);

        // 3. Update field
        issue.setassignedTo(userId);

        // 4. Save Issue
        repo->saveIssue(issue);
        return true;
    } catch (const std::out_of_range& e) {
        // Issue or User not found
        return false;
    }
}

bool IssueTrackerController::unassignUserFromIssue(int issueId) {
    try {
        // 1. Get Issue
        Issue issue = repo->getIssue(issueId);
        // 2. Update field (set to empty)
        issue.setassignedTo("");

        // 3. Save Issue
        repo->saveIssue(issue);
        return true;
    } catch (const std::out_of_range& e) {
        // Issue not found
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
    const std::string& text, const std::string& authorId) {
    try {
        // 1. Validate
        if (text.empty()) {
            return Comment(0, "", "");
        }

        // 2. Validate Issue and User existence
        Issue issue = repo->getIssue(issueId);
        repo->getUser(authorId);

        // 3. Create Comment
        Comment newComment(issueId, authorId, text);
        newComment.id = 0; // Signals to repository that its a new comment

        // 4.Save Comment
        Comment savedComment = repo->saveComment(newComment);

        // 5. Update Issue
        issue.addComment(savedComment.id);
        repo->saveIssue(issue);

        return savedComment;
    } catch (const std::out_of_range& e) {
        // Issue or User not found
        return Comment(0, "", "");
    }
}

bool IssueTrackerController::updateComment(int commentId,
    const std::string& newText) {
    try {
        // 1. Validate
        if (newText.empty()) {
            return false;
        }

        // 2. Get Comment
        Comment comment = repo->getComment(commentId);

        // 3. Update
        comment.text = newText;

        // 4. Save Comment
        repo->saveComment(comment);
        return true;
    } catch (const std::out_of_range& e) {
        // Comment not found
        return false;
    }
}

bool IssueTrackerController::deleteComment(int commentId) {
    try {
        // 1. Get the comment to find its associated issueId
        Comment comment = repo->getComment(commentId);
        int issueId = comment.issueId;

        // 2. Delete the comment from the repository
        bool deleted = repo->deleteComment(commentId);

        if (deleted) {
            // 3. Update the associated Issue to remove the comment ID
            Issue issue = repo->getIssue(issueId);
            issue.removeComment(commentId);
            repo->saveIssue(issue);
            return true;
        }
        return false;
    } catch (const std::out_of_range& e) {
        // Comment or Issue not found
        return false;
    }
}

User IssueTrackerController::createUser(const std::string& name) {
    // 1. Validate
    if (name.empty()) {
        return User("", "");
    }

    // 2. Create User
    User newUser(name, ""); // Creates with blank ID so repo can assign

    // 3. Save User
    return repo->saveUser(newUser);
}

bool IssueTrackerController::removeUser(const std::string& userId) {
    return repo->deleteUser(userId);
}

std::vector<User> IssueTrackerController::listAllUsers() {
    return repo->listAllUsers();
}
