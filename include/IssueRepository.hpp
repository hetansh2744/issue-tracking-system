#ifndef ISSUE_REPOSITORY_H_INCLUDED
#define ISSUE_REPOSITORY_H_INCLUDED

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "Comment.hpp"
#include "Issue.hpp"
#include "User.hpp"

class IssueRepository {
 public:
  // === Issues ===
  virtual Issue getIssue(int issueId) const = 0;
  virtual Issue saveIssue(const Issue& issue) = 0;
  virtual bool deleteIssue(int issueId) = 0;
  virtual std::vector<Issue> listIssues() const = 0;
  virtual std::vector<Issue> findIssues(
      std::function<bool(const Issue&)> criteria) const = 0;
  virtual std::vector<Issue> findIssues(const std::string& userId) const;
  virtual std::vector<Issue> listAllUnassigned() const;

  // == Comments ===
  virtual Comment getComment(int commentId, int issueId) const = 0;
  virtual std::vector<Comment> getAllComments(int issueId) const = 0;
  virtual Comment saveComment(int issueId, const Comment& comment) = 0;
  virtual bool deleteComment(int issueId, int commentId) = 0;
  virtual bool deleteComment(int commentId) = 0;

  // === Users ===
  virtual User getUser(const std::string& userId) const = 0;
  virtual User saveUser(const User& user) = 0;
  virtual bool deleteUser(const std::string& userId) = 0;
  virtual std::vector<User> listAllUsers() const = 0;

  virtual ~IssueRepository() = default;
};

IssueRepository* createIssueRepository();

#endif  // ISSUE_REPOSITORY_H_INCLUDED
