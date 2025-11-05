#ifndef ISSUE_TRACKER_CONTROLLER_H
#define ISSUE_TRACKER_CONTROLLER_H

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "IssueRepository.hpp"
#include "User.hpp"
#include "Issue.hpp"
#include "Comment.hpp"

class IssueTrackerController {
 private:
IssueRepository* repo;

 public:
    IssueTrackerController(IssueRepository* repository);

    Issue createIssue(const std::string& title, const std::string& desc,
    const std::string& assignedTo);

    bool updateIssueField(int id, const std::string& field,
    const std::string& value);

    bool assignUserToIssue(int issueId, const std::string& user);

    bool unassignUserFromIssue(int issueId);

    bool deleteIssue(int id);

    Comment addCommentToIssue(int issueId, const std::string& text,
    const std::string& authorId);

    bool updateComment(int commentId,
        const std::string& newText);

    bool deleteComment(int commentId);

    User createUser(const std::string& name, const std::string& roll);

    bool updateUser(const std::string& user, const std::string& field,
    const std::string& value);

    bool removeUser(const std::string& user_name);

    std::vector<Issue> listAllIssues();

    std::vector<Issue> listAllUnassignedIssues();

    std::vector<Issue> findIssuesByUserId(const std::string& user_name);

    std::vector<User> listAllUsers();
};

#endif // ISSUE_TRACKER_CONTROLLER_H.0
