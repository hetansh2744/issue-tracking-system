#ifndef TEXT_BASED_ITS_SQLITEISSUEREPOSITORY_HPP_
#define TEXT_BASED_ITS_SQLITEISSUEREPOSITORY_HPP_

#include <functional>
#include <string>
#include <vector>

#include "IssueRepository.hpp"
#include "sqlite3.h"

// Concrete IssueRepository implementation backed by SQLite.
class SQLiteIssueRepository : public IssueRepository {
 public:
  explicit SQLiteIssueRepository(const std::string& dbPath);
  ~SQLiteIssueRepository() override;

  // ---- Issue operations ----
  Issue getIssue(int issueId) const override;
  Issue saveIssue(const Issue& issue) override;
  bool deleteIssue(int issueId) override;

  std::vector<Issue> listIssues() const override;

  // Generic predicate-based search (used by some tests/tools).
  std::vector<Issue> findIssues(
      std::function<bool(const Issue&)> criteria) const override;

  // Milestone helpers:
  //  - all issues with no assignee
  std::vector<Issue> listAllUnassigned() const override;
  //  - all issues assigned to a particular user
  std::vector<Issue> findIssues(
      const std::string& userId) const override;

  // ---- Comment operations ----
  Comment getComment(int issueId, int commentId) const override;
  std::vector<Comment> getAllComments(int issueId) const override;
  Comment saveComment(int issueId, const Comment& comment) override;
  bool deleteComment(int issueId, int commentId) override;

  // ---- User operations ----
  User getUser(const std::string& userId) const override;
  User saveUser(const User& user) override;
  bool deleteUser(const std::string& userId) override;
  std::vector<User> listAllUsers() const override;

  // ---- Tag operations ----
  bool addTagToIssue(int issueId, const std::string& tag) override;
  bool removeTagFromIssue(int issueId, const std::string& tag) override;

 private:
  sqlite3* db_;

  void execOrThrow(const std::string& sql) const;
  void initializeSchema();

  bool exists(const std::string& sql,
              const std::function<void(sqlite3_stmt*)>& binder) const;

  void forEachRow(const std::string& sql,
                  const std::function<void(sqlite3_stmt*)>& binder,
                  const std::function<void(sqlite3_stmt*)>& onRow) const;

  Comment insertCommentRow(int issueId,
                           const Comment& comment,
                           int commentId);

  bool issueExists(int issueId) const;
  bool commentExists(int issueId, int commentId) const;
  int nextCommentIdForIssue(int issueId) const;
  std::vector<Comment> loadComments(int issueId) const;
};

#endif  // TEXT_BASED_ITS_SQLITEISSUEREPOSITORY_HPP_
