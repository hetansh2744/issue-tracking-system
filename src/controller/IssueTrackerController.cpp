#include "IssueTrackerController.hpp"

#include <algorithm>
#include <exception>
#include <stdexcept>

IssueTrackerController::IssueTrackerController(IssueRepository* repository)
    : repo(repository) {}

Issue IssueTrackerController::createIssue(const std::string& title,
    const std::string& desc, const std::string& author_id) {
    if (title.empty() || author_id.empty()) {
        return Issue();
    }

    // 1. Create the Issue (no description yet)
    Issue newIssue(0, author_id, title, 0);
    Issue savedIssue = repo->saveIssue(newIssue);

    // 2. Create the first Comment = Description
    if (!desc.empty()) {
        Comment descComment(0, author_id, desc, 0);
        Comment savedComment = repo->saveComment(savedIssue.getId(),
            descComment);

        // 3. Link comment #1 as description
        savedIssue.setDescriptionCommentId(savedComment.getId());
        repo->saveIssue(savedIssue);
    }

    return savedIssue;
}

//returns the issue from IR
Issue IssueTrackerController::getIssue(const int issueId) {
    return repo->getIssue(issueId);
}

//Updates the a certian issue and sends updated value to view
bool IssueTrackerController::updateIssueField(int id,
    const std::string& field,
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

            if (descComment) {
                // Update existing description comment
                Comment updated = *descComment;
                updated.setText(value);
                repo->saveComment(issue.getId(), updated);
            } else {
                // Create a new description comment
                Comment newDesc(0, issue.getAuthorId(), value, 0);
                Comment saved = repo->saveComment(issue.getId(), newDesc);
                issue.setDescriptionCommentId(saved.getId());
                repo->saveIssue(issue);
            }
            return true;

        } else if (field == "status") {
            issue.setStatus(value);
            repo->saveIssue(issue);
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

//Assigns a created user to a specific username
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
    } catch (const std::invalid_argument&) {
        return false;
    }
}

//unassigns a user
bool IssueTrackerController::unassignUserFromIssue(int issueId) {
    try {
        Issue issue = repo->getIssue(issueId);
        issue.unassign();
        repo->saveIssue(issue);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

//deletes a certain issue and returns true if success-
//-fully deleted
bool IssueTrackerController::deleteIssue(int issueId) {
    return repo->deleteIssue(issueId);
}

//returns all comments from a issue id
std::vector<Comment> IssueTrackerController::getallComments(
    int issueId) {
    return repo->getAllComments(issueId);
}

//allows the view to display the comments
Comment IssueTrackerController::getComment(
    int issueId, int commentId) {
    return repo->getComment(issueId, commentId);
}

//adds comments to certain issues and makes user inut the au-
//-thor of comments
Comment IssueTrackerController::addCommentToIssue(int issueId,
    const std::string& text, const std::string& authorId) {
    try {
        if (text.empty() || authorId.empty()) {
            // invalid input → return placeholder comment
            return Comment();
        }

        // make sure the issue exists
        Issue issue = repo->getIssue(issueId);

        // ✅ this satisfies EXPECT_CALL(mockRepo, getUser("author"))
        repo->getUser(authorId);

        // create and save the new comment
        Comment newComment(-1, authorId, text, 0);
        Comment savedComment = repo->saveComment(issueId, newComment);

        // link comment id to issue and save issue
        issue.addComment(savedComment.getId());
        repo->saveIssue(issue);

        return savedComment;
    } catch (const std::out_of_range&) {
        // issue or user not found → behave gracefully
        return Comment();
    }
}

//allows the view to update a comment
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

//deletes comments by issue id and what comment ids
//are in the issue
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

//takes input from view and creates a new user
User IssueTrackerController::createUser(const std::string& name,
    const std::string& role) {
    if (name.empty() || role.empty()) {
        return User("", "");
    }
    User newUser(name, role);
    return repo->saveUser(newUser);
}

//allows the view to use cin to access what/how needs to be -
//-changed and what new value is
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

//returns a list of all issues
std::vector<Issue> IssueTrackerController::listAllIssues() {
    return repo->listIssues();
}

//returns a list of issues to view that dont have a user id
std::vector<Issue> IssueTrackerController::listAllUnassignedIssues() {
    return repo->listAllUnassigned();
}

//reuturns issues by username inputted
//allows view to access issues by userid
std::vector<Issue> IssueTrackerController::findIssuesByUserId(
    const std::string& user_name) {

    std::string target = user_name;
    std::transform(target.begin(), target.end(),
    target.begin(), ::tolower);

    return repo->findIssues([&](const Issue& issue) {
        std::string author = issue.getAuthorId();
        std::transform(author.begin(), author.end(),
        author.begin(), ::tolower);
        return author == target;
    });
}


//lists all the users created
std::vector<User> IssueTrackerController::listAllUsers() {
    return repo->listAllUsers();
}

bool IssueTrackerController::addTagToIssue(
    int issueId, const std::string& tag) {
  try {
    return repo->addTagToIssue(issueId, tag);
  } catch (...) {
    return false;
  }
}

bool IssueTrackerController::removeTagFromIssue(
    int issueId, const std::string& tag) {
  try {
    return repo->removeTagFromIssue(issueId, tag);
  } catch (...) {
    return false;
  }
}
