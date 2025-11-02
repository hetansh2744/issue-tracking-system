#ifndef MOCK_ISSUE_REPOSITORY_H
#define MOCK_ISSUE_REPOSITORY_H

#include <gmock/gmock.h> 
#include <string>
#include <vector>
#include <ctime>
#include <stdexcept>

class User {
public:
    std::string id;
    std::string name;
    std::vector<int> assignedIssueIDs;

    User(std::string n = "", std::string i = "") : name(n), id(i) {}
};

class Comment {
public:
    int id = 0;
    int issueId = 0;
    std::string authorId;
    std::string text;
    std::string timestamp;

    Comment(int iId = 0, std::string aId = "", std::string t = "", int i = 0) 
        : id(i), issueId(iId), authorId(aId), text(t) {
        timestamp = std::to_string(std::time(0)); 
    }
};

class Issue {
public:
    int id = 0;
    std::string title;
    std::string assignedTo;
    std::string description;
    std::vector<int> commentIDs;

    Issue(std::string t = "", std::string d = "", std::string a = "", int i = 0)
        : id(i), title(t), assignedTo(a), description(d) {}

    void addComment(int commentId) {
        commentIDs.push_back(commentId);
    }
    void removeComment(int commentId) {
    }
};

/**
 * @brief Abstract Base Class for testing.
 * * methods throw std::out_of_range if an ID is not found.
 */
class IssueRepository {
public:
    virtual ~IssueRepository() = default;

    virtual Issue getIssue(int id) = 0;
    virtual Issue saveIssue(const Issue& issue) = 0;
    virtual bool deleteIssue(int id) = 0;
    virtual std::vector<Issue> listIssues() = 0;
    virtual std::vector<Issue> findIssues(const std::string& userId) = 0;
    virtual Comment getComment(int id) = 0;
    virtual Comment saveComment(const Comment& comment) = 0;
    virtual bool deleteComment(int id) = 0;

    virtual User getUser(const std::string& id) = 0;
    virtual User saveUser(const User& user) = 0;
    virtual bool deleteUser(const std::string& id) = 0;
    virtual std::vector<User> listAllUsers() = 0;
};

/**
 * @brief Mock implementation of the IssueRepository for testing the Controller.
 */
class MockIssueRepository : public IssueRepository {
public:
    // Mock for saveIssue: Used by createIssue
    MOCK_METHOD1(saveIssue, Issue(const Issue& issue));
    MOCK_METHOD1(getIssue, Issue(int id));
    MOCK_METHOD1(deleteIssue, bool(int id));
    MOCK_METHOD0(listIssues, std::vector<Issue>());
    MOCK_METHOD1(findIssues, std::vector<Issue>(const std::string& userId));

    MOCK_METHOD1(getComment, Comment(int id));
    MOCK_METHOD1(saveComment, Comment(const Comment& comment));
    MOCK_METHOD1(deleteComment, bool(int id));
    MOCK_METHOD1(getUser, User(const std::string& id));
    MOCK_METHOD1(saveUser, User(const User& user));
    MOCK_METHOD0(listAllUsers, std::vector<User>());
    MOCK_METHOD1(deleteUser, bool(const std::string& id));
};

#endif