#include "SQLiteIssueRepository.hpp"

#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>
#include <ctime>

namespace {
class SqliteStmt {
 public:
  SqliteStmt(sqlite3* db, const std::string& sql) : stmt_(nullptr) {
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt_, nullptr) != SQLITE_OK) {
      throw std::runtime_error(sqlite3_errmsg(db));
    }
  }
  ~SqliteStmt() {
    if (stmt_) {
      sqlite3_finalize(stmt_);
    }
  }

  sqlite3_stmt* get() const { return stmt_; }

  SqliteStmt(const SqliteStmt&) = delete;
  SqliteStmt& operator=(const SqliteStmt&) = delete;

 private:
  sqlite3_stmt* stmt_;
};

std::string columnText(sqlite3_stmt* stmt, int index) {
  const unsigned char* text = sqlite3_column_text(stmt, index);
  return text ? reinterpret_cast<const char*>(text) : std::string();
}

Comment::TimePoint currentTimeMillis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}
}  // namespace

SQLiteIssueRepository::SQLiteIssueRepository(const std::string& dbPath) {
  if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK) {
    throw std::runtime_error("Failed to open SQLite database: " + dbPath);
  }
  execOrThrow("PRAGMA foreign_keys = ON;");
  initializeSchema();
}

SQLiteIssueRepository::~SQLiteIssueRepository() {
  if (db_ != nullptr) {
    sqlite3_close(db_);
    db_ = nullptr;
  }
}

void SQLiteIssueRepository::execOrThrow(const std::string& sql) const {
  char* errMsg = nullptr;
  if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
    std::string message = errMsg ? errMsg : "Unknown SQLite error";
    sqlite3_free(errMsg);
    throw std::runtime_error(message);
  }
}

void SQLiteIssueRepository::initializeSchema() {
  const char* statements[] = {
      "CREATE TABLE IF NOT EXISTS issues (id INTEGER PRIMARY KEY "
      "AUTOINCREMENT, author_id TEXT NOT NULL, title TEXT NOT NULL, "
      "description_comment_id INTEGER NOT NULL DEFAULT -1, assigned_to TEXT, "
      "created_at INTEGER DEFAULT 0);",

      "CREATE TABLE IF NOT EXISTS comments (id INTEGER NOT NULL, "
      "issue_id INTEGER NOT NULL, author_id TEXT NOT NULL, text TEXT NOT NULL, "
      "timestamp INTEGER DEFAULT 0, PRIMARY KEY(issue_id, id), "
      "FOREIGN KEY(issue_id) REFERENCES issues(id) ON DELETE CASCADE);",

      "CREATE TABLE IF NOT EXISTS users (name TEXT PRIMARY KEY, role TEXT NOT "
      "NULL);",
      "CREATE TABLE IF NOT EXISTS issue_tags ("
      " issue_id INTEGER NOT NULL,"
      " tag TEXT NOT NULL,"
      " PRIMARY KEY(issue_id, tag),"
      " FOREIGN KEY(issue_id) REFERENCES issues(id) ON DELETE CASCADE"
      ");",

      "CREATE INDEX IF NOT EXISTS idx_comments_issue ON comments(issue_id);"
  };

  for (const char* sql : statements) {
    execOrThrow(sql);
  }
}

bool SQLiteIssueRepository::exists(
    const std::string& sql,
    const std::function<void(sqlite3_stmt*)>& binder) const {
  SqliteStmt stmt(db_, sql);
  if (binder) {
    binder(stmt.get());
  }
  int rc = sqlite3_step(stmt.get());
  if (rc == SQLITE_ROW) {
    return true;
  }
  if (rc == SQLITE_DONE) {
    return false;
  }
  throw std::runtime_error("Failed to execute exists query");
}

void SQLiteIssueRepository::forEachRow(
    const std::string& sql, const std::function<void(sqlite3_stmt*)>& binder,
    const std::function<void(sqlite3_stmt*)>& onRow) const {
  SqliteStmt stmt(db_, sql);
  if (binder) {
    binder(stmt.get());
  }

  while (true) {
    int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_ROW) {
      onRow(stmt.get());
    } else if (rc == SQLITE_DONE) {
      break;
    } else {
      throw std::runtime_error("Failed to read rows");
    }
  }
}

Comment SQLiteIssueRepository::insertCommentRow(int issueId,
                                                const Comment& comment,
                                                int commentId) {
  Comment stored = comment;
  if (stored.getTimeStamp() == 0) {
    stored.setTimeStamp(currentTimeMillis());
  }
  SqliteStmt stmt(
      db_,
      "INSERT INTO comments (id, issue_id, author_id, text, timestamp) "
      "VALUES (?, ?, ?, ?, ?);");
  sqlite3_bind_int(stmt.get(), 1, commentId);
  sqlite3_bind_int(stmt.get(), 2, issueId);
  sqlite3_bind_text(stmt.get(), 3, stored.getAuthor().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 4, stored.getText().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt.get(), 5, stored.getTimeStamp());

  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to insert comment");
  }

  if (!stored.hasPersistentId()) {
    stored.setIdForPersistence(commentId);
  }
  return stored;
}

bool SQLiteIssueRepository::issueExists(int issueId) const {
  return exists(
      "SELECT 1 FROM issues WHERE id = ? LIMIT 1;",
      [issueId](sqlite3_stmt* stmt) { sqlite3_bind_int(stmt, 1, issueId); });
}

bool SQLiteIssueRepository::commentExists(int issueId, int commentId) const {
  return exists("SELECT 1 FROM comments WHERE issue_id = ? AND id = ? LIMIT 1;",
                [issueId, commentId](sqlite3_stmt* stmt) {
                  sqlite3_bind_int(stmt, 1, issueId);
                  sqlite3_bind_int(stmt, 2, commentId);
                });
}

int SQLiteIssueRepository::nextCommentIdForIssue(int issueId) const {
  SqliteStmt stmt(
      db_, "SELECT COALESCE(MAX(id), -1) FROM comments WHERE issue_id = ?;");
  sqlite3_bind_int(stmt.get(), 1, issueId);
  if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
    throw std::runtime_error("Failed to compute next comment id");
  }
  int maxId = sqlite3_column_int(stmt.get(), 0);
  return maxId + 1;
}

std::vector<Comment> SQLiteIssueRepository::loadComments(int issueId) const {
  std::vector<Comment> comments;
  forEachRow(
      "SELECT id, author_id, text, timestamp FROM comments WHERE issue_id = ? "
      "ORDER BY id ASC;",
      [issueId](sqlite3_stmt* stmt) { sqlite3_bind_int(stmt, 1, issueId); },
      [&comments](sqlite3_stmt* stmt) {
        comments.emplace_back(sqlite3_column_int(stmt, 0), columnText(stmt, 1),
                              columnText(stmt, 2),
                              sqlite3_column_int64(stmt, 3));
      });
  return comments;
}

Issue SQLiteIssueRepository::getIssue(int issueId) const {
  SqliteStmt stmt(
      db_,
      "SELECT id, author_id, title, description_comment_id, assigned_to, "
      "created_at FROM issues WHERE id = ? LIMIT 1;");
  sqlite3_bind_int(stmt.get(), 1, issueId);

  if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
    throw std::invalid_argument("Issue with given ID does not exist");
  }

  Issue issue(sqlite3_column_int(stmt.get(), 0), columnText(stmt.get(), 1),
              columnText(stmt.get(), 2), sqlite3_column_int64(stmt.get(), 5));

  const int descriptionId = sqlite3_column_int(stmt.get(), 3);
  const std::string assigned = columnText(stmt.get(), 4);
  if (!assigned.empty()) {
    issue.assignTo(assigned);
  }
  if (descriptionId >= 0) {
    issue.setDescriptionCommentId(descriptionId);
  }
  for (const auto& comment : loadComments(issueId)) {
    issue.addComment(comment);
  }

  forEachRow(
      "SELECT tag FROM issue_tags WHERE issue_id = ?;",
      [issueId](sqlite3_stmt* stmt) { sqlite3_bind_int(stmt, 1, issueId); },
      [&issue](sqlite3_stmt* stmt) {
        std::string tag = columnText(stmt, 0);
        if (!tag.empty()) {
          issue.addTag(tag);
        }
      });
  return issue;
}

Issue SQLiteIssueRepository::saveIssue(const Issue& issue) {
  Issue stored = issue;

  if (!stored.hasPersistentId()) {
    if (stored.getTimestamp() == 0) {
      stored.setTimestamp(currentTimeMillis());
    }
    SqliteStmt stmt(
        db_,
        "INSERT INTO issues (author_id, title, description_comment_id, "
        "assigned_to, created_at) VALUES (?, ?, ?, ?, ?);");
    sqlite3_bind_text(stmt.get(), 1, stored.getAuthorId().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 2, stored.getTitle().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt.get(), 3, stored.getDescriptionCommentId());
    if (stored.hasAssignee()) {
      sqlite3_bind_text(stmt.get(), 4, stored.getAssignedTo().c_str(), -1,
                        SQLITE_TRANSIENT);
    } else {
      sqlite3_bind_null(stmt.get(), 4);
    }
    sqlite3_bind_int64(stmt.get(), 5, stored.getTimestamp());

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
      throw std::runtime_error("Failed to insert issue");
    }

    int newId = static_cast<int>(sqlite3_last_insert_rowid(db_));
    stored.setIdForPersistence(newId);
  } else {
    if (!issueExists(stored.getId())) {
      throw std::invalid_argument("Issue with given ID does not exist: " +
                                  std::to_string(stored.getId()));
    }

    if (stored.getTimestamp() == 0) {
      stored.setTimestamp(currentTimeMillis());
    }
    SqliteStmt stmt(db_,
                    "UPDATE issues SET author_id = ?, title = ?, "
                    "description_comment_id = ?, assigned_to = ?, "
                    "created_at = ? WHERE id = ?;");
    sqlite3_bind_text(stmt.get(), 1, stored.getAuthorId().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 2, stored.getTitle().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt.get(), 3, stored.getDescriptionCommentId());
    if (stored.hasAssignee()) {
      sqlite3_bind_text(stmt.get(), 4, stored.getAssignedTo().c_str(), -1,
                        SQLITE_TRANSIENT);
    } else {
      sqlite3_bind_null(stmt.get(), 4);
    }
    sqlite3_bind_int64(stmt.get(), 5, stored.getTimestamp());
    sqlite3_bind_int(stmt.get(), 6, stored.getId());

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
      throw std::runtime_error("Failed to update issue");
    }
  }

  {
    SqliteStmt stmt(db_, "DELETE FROM issue_tags WHERE issue_id = ?;");
    sqlite3_bind_int(stmt.get(), 1, stored.getId());
    sqlite3_step(stmt.get());
  }

  for (const auto& tag : stored.getTags()) {
    SqliteStmt stmt(db_,
                    "INSERT INTO issue_tags (issue_id, tag) VALUES (?, ?);");
    sqlite3_bind_int(stmt.get(), 1, stored.getId());
    sqlite3_bind_text(stmt.get(), 2, tag.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt.get());
  }

  return getIssue(stored.getId());
}

bool SQLiteIssueRepository::deleteIssue(int issueId) {
  SqliteStmt stmt(db_, "DELETE FROM issues WHERE id = ?;");
  sqlite3_bind_int(stmt.get(), 1, issueId);
  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to delete issue");
  }
  return sqlite3_changes(db_) > 0;
}

std::vector<Issue> SQLiteIssueRepository::listIssues() const {
  std::vector<Issue> issues;
  forEachRow("SELECT id FROM issues ORDER BY id ASC;", {},
             [this, &issues](sqlite3_stmt* stmt) {
               int id = sqlite3_column_int(stmt, 0);
               issues.push_back(getIssue(id));
             });
  return issues;
}

std::vector<Issue> SQLiteIssueRepository::findIssues(
    std::function<bool(const Issue&)> criteria) const {
  std::vector<Issue> all = listIssues();
  std::vector<Issue> filtered;
  for (Issue& issue : all) {
    if (criteria(issue)) {
      filtered.push_back(issue);
    }
  }
  return filtered;
}

Comment SQLiteIssueRepository::getComment(int issueId, int commentId) const {
  SqliteStmt stmt(db_,
                  "SELECT id, author_id, text, timestamp FROM comments "
                  "WHERE issue_id = ? AND id = ? LIMIT 1;");
  sqlite3_bind_int(stmt.get(), 1, issueId);
  sqlite3_bind_int(stmt.get(), 2, commentId);

  if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
    throw std::invalid_argument("Comment does not belong to the given issue");
  }

  int id = sqlite3_column_int(stmt.get(), 0);
  std::string author = columnText(stmt.get(), 1);
  std::string text = columnText(stmt.get(), 2);
  Comment::TimePoint ts = sqlite3_column_int64(stmt.get(), 3);
  return Comment(id, author, text, ts);
}

std::vector<Comment> SQLiteIssueRepository::getAllComments(int issueId) const {
  if (!issueExists(issueId)) {
    throw std::invalid_argument("Issue with given ID does not exist");
  }
  return loadComments(issueId);
}

Comment SQLiteIssueRepository::saveComment(int issueId,
                                           const Comment& comment) {
  if (!issueExists(issueId)) {
    throw std::invalid_argument("Issue with given ID does not exist");
  }

  if (!comment.hasPersistentId()) {
    const int newId = nextCommentIdForIssue(issueId);
    return insertCommentRow(issueId, comment, newId);
  }

  const int commentId = comment.getId();
  if (!commentExists(issueId, commentId)) {
    if (commentId == 0) {
      return insertCommentRow(issueId, comment, 0);
    }
    throw std::invalid_argument("Comment with given ID does not exist");
  }

  Comment updated = comment;
  if (updated.getTimeStamp() == 0) {
    updated.setTimeStamp(currentTimeMillis());
  }
  SqliteStmt stmt(db_,
                  "UPDATE comments SET author_id = ?, text = ?, timestamp = ? "
                  "WHERE issue_id = ? AND id = ?;");
  sqlite3_bind_text(stmt.get(), 1, updated.getAuthor().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 2, updated.getText().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt.get(), 3, updated.getTimeStamp());
  sqlite3_bind_int(stmt.get(), 4, issueId);
  sqlite3_bind_int(stmt.get(), 5, commentId);

  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to update comment");
  }

  return updated;
}

bool SQLiteIssueRepository::deleteComment(int issueId, int commentId) {
  if (!issueExists(issueId)) {
    throw std::invalid_argument("Issue with given ID does not exist");
  }

  if (!commentExists(issueId, commentId)) {
    throw std::invalid_argument("Comment with given ID does not exist");
  }

  SqliteStmt clearDesc(
      db_,
      "UPDATE issues SET description_comment_id = -1 WHERE id = ? "
      "AND description_comment_id = ?;");
  sqlite3_bind_int(clearDesc.get(), 1, issueId);
  sqlite3_bind_int(clearDesc.get(), 2, commentId);
  sqlite3_step(clearDesc.get());

  SqliteStmt stmt(db_, "DELETE FROM comments WHERE issue_id = ? AND id = ?;");
  sqlite3_bind_int(stmt.get(), 1, issueId);
  sqlite3_bind_int(stmt.get(), 2, commentId);
  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to delete comment");
  }
  return sqlite3_changes(db_) > 0;
}

User SQLiteIssueRepository::getUser(const std::string& userId) const {
  SqliteStmt stmt(db_, "SELECT name, role FROM users WHERE name = ? LIMIT 1;");
  sqlite3_bind_text(stmt.get(), 1, userId.c_str(), -1, SQLITE_TRANSIENT);

  if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
    throw std::invalid_argument("User with given ID does not exist");
  }

  std::string name = columnText(stmt.get(), 0);
  std::string role = columnText(stmt.get(), 1);
  return User(name, role);
}

User SQLiteIssueRepository::saveUser(const User& user) {
  if (user.getName().empty()) {
    throw std::invalid_argument("User ID must be non-empty");
  }

  SqliteStmt stmt(db_,
                  "INSERT INTO users (name, role) VALUES (?, ?) "
                  "ON CONFLICT(name) DO UPDATE SET role = excluded.role;");
  sqlite3_bind_text(stmt.get(), 1, user.getName().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 2, user.getRole().c_str(), -1,
                    SQLITE_TRANSIENT);

  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to save user");
  }
  return user;
}

bool SQLiteIssueRepository::deleteUser(const std::string& userId) {
  SqliteStmt stmt(db_, "DELETE FROM users WHERE name = ?;");
  sqlite3_bind_text(stmt.get(), 1, userId.c_str(), -1, SQLITE_TRANSIENT);
  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to delete user");
  }
  return sqlite3_changes(db_) > 0;
}

std::vector<User> SQLiteIssueRepository::listAllUsers() const {
  std::vector<User> users;
  forEachRow("SELECT name, role FROM users ORDER BY name ASC;", {},
             [&users](sqlite3_stmt* stmt) {
               users.emplace_back(columnText(stmt, 0), columnText(stmt, 1));
             });
  return users;
}

// ==================== MILESTONE IMPLEMENTATIONS ====================

bool SQLiteIssueRepository::milestoneExists(int milestoneId) const {
  return exists(
    "SELECT 1 FROM milestones WHERE id = ? LIMIT 1;",
    [milestoneId](sqlite3_stmt* stmt) { 
      sqlite3_bind_int(stmt, 1, milestoneId); 
    }
  );
}

Milestone SQLiteIssueRepository::saveMilestone(const Milestone& milestone) {
  Milestone stored = milestone;
  
  // If milestone doesn't have a persistent ID, INSERT
  if (!stored.hasPersistentId()) {
    SqliteStmt stmt(
      db_,
      "INSERT INTO milestones (name, description, start_date, end_date, created_at) "
      "VALUES (?, ?, ?, ?, ?);");
    
    sqlite3_bind_text(stmt.get(), 1, stored.getName().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 2, stored.getDescription().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 3, stored.getStartDate().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 4, stored.getEndDate().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt.get(), 5, std::time(nullptr)); // Current timestamp
    
    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
      throw std::runtime_error("Failed to insert milestone");
    }
    
    int newId = static_cast<int>(sqlite3_last_insert_rowid(db_));
    stored.setIdForPersistence(newId);
    
    // Save all associated issues
    for (int issueId : stored.getIssueIds()) {
      addIssueToMilestone(newId, issueId);
    }
    
    return getMilestone(newId);
  }
  
  // UPDATE existing milestone
  if (!milestoneExists(stored.getId())) {
    throw std::invalid_argument("Milestone with given ID does not exist: " + 
                                std::to_string(stored.getId()));
  }
  
  SqliteStmt stmt(
    db_,
    "UPDATE milestones SET name = ?, description = ?, start_date = ?, end_date = ? "
    "WHERE id = ?;");
  
  sqlite3_bind_text(stmt.get(), 1, stored.getName().c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 2, stored.getDescription().c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 3, stored.getStartDate().c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 4, stored.getEndDate().c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt.get(), 5, stored.getId());
  
  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to update milestone");
  }
  
  // Update issue associations (remove all, then re-add)
  SqliteStmt deleteStmt(db_, "DELETE FROM milestone_issues WHERE milestone_id = ?;");
  sqlite3_bind_int(deleteStmt.get(), 1, stored.getId());
  sqlite3_step(deleteStmt.get());
  
  for (int issueId : stored.getIssueIds()) {
    addIssueToMilestone(stored.getId(), issueId);
  }
  
  return getMilestone(stored.getId());
}

Milestone SQLiteIssueRepository::getMilestone(int milestoneId) const {
  SqliteStmt stmt(
    db_,
    "SELECT id, name, description, start_date, end_date, created_at "
    "FROM milestones WHERE id = ? LIMIT 1;");
  
  sqlite3_bind_int(stmt.get(), 1, milestoneId);
  
  if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
    throw std::invalid_argument("Milestone with given ID does not exist");
  }
  
  int id = sqlite3_column_int(stmt.get(), 0);
  std::string name = columnText(stmt.get(), 1);
  std::string description = columnText(stmt.get(), 2);
  std::string startDate = columnText(stmt.get(), 3);
  std::string endDate = columnText(stmt.get(), 4);
  
  Milestone milestone(id, name, description, startDate, endDate);
  
  // Load all associated issues
  std::vector<Issue> issues = getIssuesForMilestone(milestoneId);
  for (const auto& issue : issues) {
    milestone.addIssue(issue.getId());
  }
  
  return milestone;
}

bool SQLiteIssueRepository::deleteMilestone(int milestoneId, bool cascade) {
  if (!milestoneExists(milestoneId)) {
    return false;
  }
  
  if (cascade) {
    // Delete all issues associated with this milestone
    std::vector<Issue> issues = getIssuesForMilestone(milestoneId);
    for (const auto& issue : issues) {
      deleteIssue(issue.getId());
    }
  } else {
    // Just remove the associations
    SqliteStmt stmt(db_, "DELETE FROM milestone_issues WHERE milestone_id = ?;");
    sqlite3_bind_int(stmt.get(), 1, milestoneId);
    sqlite3_step(stmt.get());
  }
  
  // Delete the milestone itself
  SqliteStmt stmt(db_, "DELETE FROM milestones WHERE id = ?;");
  sqlite3_bind_int(stmt.get(), 1, milestoneId);
  
  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to delete milestone");
  }
  
  return sqlite3_changes(db_) > 0;
}

std::vector<Milestone> SQLiteIssueRepository::listAllMilestones() const {
  std::vector<Milestone> milestones;
  
  forEachRow(
    "SELECT id FROM milestones ORDER BY id ASC;",
    {},
    [this, &milestones](sqlite3_stmt* stmt) {
      int id = sqlite3_column_int(stmt, 0);
      milestones.push_back(getMilestone(id));
    }
  );
  
  return milestones;
}

bool SQLiteIssueRepository::addIssueToMilestone(int milestoneId, int issueId) {
  if (!milestoneExists(milestoneId)) {
    throw std::invalid_argument("Milestone does not exist");
  }
  
  if (!issueExists(issueId)) {
    throw std::invalid_argument("Issue does not exist");
  }
  
  // Check if already associated
  if (exists(
    "SELECT 1 FROM milestone_issues WHERE milestone_id = ? AND issue_id = ? LIMIT 1;",
    [milestoneId, issueId](sqlite3_stmt* stmt) {
      sqlite3_bind_int(stmt, 1, milestoneId);
      sqlite3_bind_int(stmt, 2, issueId);
    }
  )) {
    return true; // Already exists, no error
  }
  
  SqliteStmt stmt(
    db_,
    "INSERT INTO milestone_issues (milestone_id, issue_id) VALUES (?, ?);");
  
  sqlite3_bind_int(stmt.get(), 1, milestoneId);
  sqlite3_bind_int(stmt.get(), 2, issueId);
  
  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to add issue to milestone");
  }
  
  return true;
}

bool SQLiteIssueRepository::removeIssueFromMilestone(int milestoneId, int issueId) {
  SqliteStmt stmt(
    db_,
    "DELETE FROM milestone_issues WHERE milestone_id = ? AND issue_id = ?;");
  
  sqlite3_bind_int(stmt.get(), 1, milestoneId);
  sqlite3_bind_int(stmt.get(), 2, issueId);
  
  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to remove issue from milestone");
  }
  
  return sqlite3_changes(db_) > 0;
}

std::vector<Issue> SQLiteIssueRepository::getIssuesForMilestone(int milestoneId) const {
  std::vector<Issue> issues;
  
  forEachRow(
    "SELECT issue_id FROM milestone_issues WHERE milestone_id = ? ORDER BY issue_id ASC;",
    [milestoneId](sqlite3_stmt* stmt) {
      sqlite3_bind_int(stmt, 1, milestoneId);
    },
    [this, &issues](sqlite3_stmt* stmt) {
      int issueId = sqlite3_column_int(stmt, 0);
      try {
        issues.push_back(getIssue(issueId));
      } catch (const std::invalid_argument&) {
        // Issue was deleted but junction table entry remains (shouldn't happen with CASCADE)
      }
    }
  );
  
  return issues;
}