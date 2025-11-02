#include <iostream>
#include <string>
#include <vector>
#include "IssueRepository.h"
#include "Issue.h"
#include "User.h"

class IssueTrackerController {
 private:
IssueRepository& repo;
 public:
    IssueTrackerController(IssueRepository& repository);

    Issue createIssue(const std::string& title, const std::string& desc, const std::string& assignedTo);

    bool updateIssueField(int id, const std::string& field, const std::string& value);

    bool assignUserToIssue(int issueId, const std::string& userId);

    bool unassignUserFromIssue(int issueId);

    bool deleteIssue(int id);

    Comment addCommentToIssue(int issueId, const std::string& text, const std::string& authorId);

    bool updateComment(int commentId, const std::string& newText);

    bool deleteComment(int commentId);

    User createUser(const std::string& name);

    bool removeUser(const std::string& id);

    std::vector<Issue> listAllIssues();

    std::vector<Issue> findIssuesByUserId(const std::string& userId);

    std::vector<User> listAllUsers();
};

#endif