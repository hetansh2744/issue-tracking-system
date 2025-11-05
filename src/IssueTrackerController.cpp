#include "IssueTrackerController.hpp"

IssueTrackerController::IssueTrackerController(IssueRepository* repository)
    : repo(repository) {}

Issue IssueTrackerController::createIssue(const std::string& title,
    const std::string& desc, const std::string& assignedTo) {
    if (title.empty() || desc.empty()) {
        return Issue(0, "", "");
    }
    Issue newIssue(0, title, desc);
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
        issue.assignTo(value);
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
    const std::string& user_name) {
    try {
        // 1. Check existense of user
        repo->getUser(user_name);

        // 2. Get Issue
        Issue issue = repo->getIssue(issueId);

        // 3. Update field
        issue.assignTo(user_name);

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
        issue.unassign();

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
        Comment newComment(0, authorId, text);

        // 4.Save Comment
        Comment savedComment = repo->saveComment(newComment);

        // 5. Update Issue
        issue.addComment(savedComment.getId);
        repo->saveIssue(issue);

        return savedComment;
    } catch (const std::out_of_range& e) {
        // Issue or User not found
        return Comment(0, "", "");
    }
}

bool IssueTrackerController::updateComment(int issueId, int commentId,
    const std::string& newText) {
    try {
        Comment comment = repo->getComment(issueId, commentId);
        comment.setText(newText);
        repo->saveComment(Icomment);
        return true;
    } catch (const std::out_of_range& e) {
        // Comment not found
        return false;
    }
}

bool IssueTrackerController::deleteComment(int issueId, int commentId) {
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

User IssueTrackerController::createUser(const std::string& name,
    const std::string& roll) {
    // 1. Validate
    if (name.empty() || roll.empty()) {
        return User("", "");
    }
    User newUser(name, roll);
    return repo->saveUser(newUser);
}

bool IssueTrackerController::updateUser(const std::string& user_name,
    const std::string& field, const std::string& value) {
  try {
    User user = repo->getUser(user_name);
    if (field == "name") user.setName(value);
    else if (field == "role") user.setRole(value);
    else return false;
    repo->saveUser(user);
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

std::vector<Issue> IssueTrackerController::findIssuesByUserId(
    const std::string& user_name) {
    return repo->findIssues(user_name);
}

std::vector<User> IssueTrackerController::listAllUsers() {
    return repo->listAllUsers();
}
