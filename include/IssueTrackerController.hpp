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

    virtual ~IssueTrackerController() = default;

    virtual Issue createIssue(const std::string& title,
        const std::string& desc, const std::string& assignedTo);

    Issue getIssue(const int issueId);

    virtual bool updateIssueField(int id, const std::string& field,
        const std::string& value);

    virtual bool assignUserToIssue(int issueId, const std::string& user);

    virtual bool unassignUserFromIssue(int issueId);

    virtual bool deleteIssue(int id);

    virtual std::vector<Comment> getallComments(int issueId);

    virtual Comment getComment(int issueId, int commentId);

    virtual Comment addCommentToIssue(int issueId, const std::string& text,
        const std::string& authorId);

    virtual bool updateComment(int issueId, int commentId,
        const std::string& newText);

    virtual bool deleteComment(int issueId, int commentId);

    virtual User createUser(const std::string& name, const std::string& roll);

    virtual bool updateUser(const std::string& user, const std::string& field,
        const std::string& value);

    virtual bool removeUser(const std::string& user_name);

    virtual std::vector<Issue> listAllIssues();

    virtual std::vector<Issue> listAllUnassignedIssues();

    virtual std::vector<Issue> findIssuesByUserId(
        const std::string& user_name);

    virtual std::vector<User> listAllUsers();
};

#endif // ISSUE_TRACKER_CONTROLLER_H.0
