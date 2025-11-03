#ifndef ISSUEREPO_H
#define ISSUEREPO_H

#include <string>
#include <vector>
#include <stdexcept> 
#include "Entities.h" 

/**
 * @brief Abstract interface for the persistence layer (Model).
 * * All concrete repository implementations must inherit from this class.
 */
class IssueRepository {
 public:

    virtual ~IssueRepository() = default;

    virtual Issue saveIssue(const Issue& issue) = 0;
    virtual Issue getIssue(int id) = 0;
    virtual std::vector<Issue> listIssues() = 0;
    virtual bool deleteIssue(int id) = 0;

    virtual std::vector<Issue> findIssues(const std::string& userId) = 0;

    virtual Comment saveComment(const Comment& comment) = 0;
    virtual Comment getComment(int id) = 0;
    virtual bool deleteComment(int id) = 0;
    
    // --- User Operations ---
    virtual User saveUser(const User& user) = 0;
    virtual User getUser(const std::string& id) = 0;
    virtual std::vector<User> listAllUsers() = 0;
    virtual bool deleteUser(const std::string& id) = 0;
};

#endif // ISSUEREPO_H
