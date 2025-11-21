#ifndef SQLITE_ISSUE_REPOSITORY_HPP
#define SQLITE_ISSUE_REPOSITORY_HPP

#include <sqlite3.h>

#include <functional>
#include <string>
#include <vector>

#include "IssueRepository.hpp"

class SQLiteIssueRepository : public IssueRepository {
 public:
  explicit SQLiteIssueRepository(const std::string& dbPath);
  ~SQLiteIssueRepository() override;

  Issue getIssue(int issueId) const override;
  Issue saveIssue(const Issue& issue) override;
  bool deleteIssue(int issueId) override;
  std::vector<Issue> listIssues() const override;
  std::vector<Issue> findIssues(
      std::function<bool(const Issue&)> criteria) const override;

  Comment getComment(int issueId, int commentId) const override;
  std::vector<Comment> getAllComments(int issueId) const override;
  Comment saveComment(int issueId, const Comment& comment) override;
  bool deleteComment(int issueId, int commentId) override;

  User getUser(const std::string& userId) const override;
  User saveUser(const User& user) override;
  bool deleteUser(const std::string& userId) override;
  std::vector<User> listAllUsers() const override;

 private:
  sqlite3* db_{nullptr};

  void execOrThrow(const std::string& sql) const;
  void initializeSchema();
  bool exists(const std::string& sql,
              const std::function<void(sqlite3_stmt*)>& binder) const;
  void forEachRow(const std::string& sql,
                  const std::function<void(sqlite3_stmt*)>& binder,
                  const std::function<void(sqlite3_stmt*)>& onRow) const;
  Comment insertCommentRow(int issueId, const Comment& comment, int commentId);
  bool issueExists(int issueId) const;
  bool commentExists(int issueId, int commentId) const;
  int nextCommentIdForIssue(int issueId) const;
  std::vector<Comment> loadComments(int issueId) const;
};

#endif  // SQLITE_ISSUE_REPOSITORY_HPP
