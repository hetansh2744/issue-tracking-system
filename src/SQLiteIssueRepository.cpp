#include "SQLiteIssueRepository.hpp"

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

class SqliteStmt {
 public:
  SqliteStmt(sqlite3* db, const std::string& sql) : stmt_(nullptr) {
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt_, nullptr)
        != SQLITE_OK) {
      throw std::runtime_error(sqlite3_errmsg(db));
    }
  }

  ~SqliteStmt() {
    if (stmt_ != nullptr) {
      sqlite3_finalize(stmt_);
      stmt_ = nullptr;
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

}  // namespace

SQLiteIssueRepository::SQLiteIssueRepository(const std::string& dbPath)
    : db_(nullptr) {
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
  if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg)
      != SQLITE_OK) {
    std::string message = errMsg ? errMsg : "Unknown SQLite error";
    sqlite3_free(errMsg);
    throw std::runtime_error(message);
  }
}

void SQLiteIssueRepository::initializeSchema() {
  // Base schema. For a brand-new DB this will create the issues table
  // including the status column. For an existing DB, this has no effect.
  const char* statements[] = {
      "CREATE TABLE IF NOT EXISTS issues ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "author_id TEXT NOT NULL, "
      "title TEXT NOT NULL, "
      "description_comment_id INTEGER NOT NULL DEFAULT -1, "
      "assigned_to TEXT, "
      "status TEXT NOT NULL DEFAULT 'To Be Done', "
      "created_at INTEGER DEFAULT 0);",
      "CREATE TABLE IF NOT EXISTS comments ("
      "id INTEGER NOT NULL, "
      "issue_id INTEGER NOT NULL, "
      "author_id TEXT NOT NULL, "
      "text TEXT NOT NULL, "
      "timestamp INTEGER DEFAULT 0, "
      "PRIMARY KEY(issue_id, id), "
      "FOREIGN KEY(issue_id) REFERENCES issues(id) "
      "ON DELETE CASCADE);",
      "CREATE TABLE IF NOT EXISTS users ("
      "name TEXT PRIMARY KEY, "
      "role TEXT NOT NULL);",
      "CREATE INDEX IF NOT EXISTS idx_comments_issue "
      "ON comments(issue_id);"};

  for (const char* sql : statements) {
    execOrThrow(sql);
  }

  // Migration for older DBs that were created BEFORE the status column
  // existed. Older schema had no 'status' column, so we add it here.
  try {
    execOrThrow(
        "ALTER TABLE issues "
        "ADD COLUMN status TEXT NOT NULL "
        "DEFAULT 'To Be Done';");
  } catch (const std::runtime_error& e) {
    const std::string msg = e.what();
    // If the column already exists, SQLite will say something like
    // "duplicate column name: status". That is safe to ignore.
    if (msg.find("duplicate column name") == std::string::npos) {
      throw;
    }
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
    const std::string& sql,
    const std::function<void(sqlite3_stmt*)>& binder,
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

Comment SQLiteIssueRepository::insertCommentRow(
    int issueId,
    const Comment& comment,
    int commentId) {
  SqliteStmt stmt(
      db_,
      "INSERT INTO comments (id, issue_id, author_id, text, timestamp) "
      "VALUES (?, ?, ?, ?, ?);");
  sqlite3_bind_int(stmt.get(), 1, commentId);
  sqlite3_bind_int(stmt.get(), 2, issueId);
  sqlite3_bind_text(stmt.get(), 3, comment.getAuthor().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 4, comment.getText().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt.get(), 5, comment.getTimeStamp());

  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to insert comment");
  }

  Comment stored = comment;
  if (!stored.hasPersistentId()) {
    stored.setIdForPersistence(commentId);
  }
  return stored;
}

bool SQLiteIssueRepository::issueExists(int issueId) const {
  return exists(
      "SELECT 1 FROM issues WHERE id = ? LIMIT 1;",
      [issueId](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, issueId);
      });
}

bool SQLiteIssueRepository::commentExists(int issueId,
                                          int commentId) const {
  return exists(
      "SELECT 1 FROM comments WHERE issue_id = ? AND id = ? LIMIT 1;",
      [issueId, commentId](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, issueId);
        sqlite3_bind_int(stmt, 2, commentId);
      });
}

int SQLiteIssueRepository::nextCommentIdForIssue(int issueId) const {
  SqliteStmt stmt(
      db_,
      "SELECT COALESCE(MAX(id), -1) FROM comments WHERE issue_id = ?;");
  sqlite3_bind_int(stmt.get(), 1, issueId);
  if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
    throw std::runtime_error("Failed to compute next comment id");
  }
  int maxId = sqlite3_column_int(stmt.get(), 0);
  return maxId + 1;
}

std::vector<Comment> SQLiteIssueRepository::loadComments(
    int issueId) const {
  std::vector<Comment> comments;
  forEachRow(
      "SELECT id, author_id, text, timestamp FROM comments "
      "WHERE issue_id = ? ORDER BY id ASC;",
      [issueId](sqlite3_stmt* stmt) {
        sqlite3_bind_int(stmt, 1, issueId);
      },
      [&comments](sqlite3_stmt* stmt) {
        comments.emplace_back(
            sqlite3_column_int(stmt, 0),
            columnText(stmt, 1),
            columnText(stmt, 2),
            sqlite3_column_int64(stmt, 3));
      });
  return comments;
}

Issue SQLiteIssueRepository::getIssue(int issueId) const {
  SqliteStmt stmt(
      db_,
      "SELECT id, author_id, title, description_comment_id, assigned_to, "
      "status, created_at "
      "FROM issues WHERE id = ? LIMIT 1;");
  sqlite3_bind_int(stmt.get(), 1, issueId);

  if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
    throw std::invalid_argument("Issue with given ID does not exist");
  }

  Issue issue(
      sqlite3_column_int(stmt.get(), 0),
      columnText(stmt.get(), 1),
      columnText(stmt.get(), 2),
      sqlite3_column_int64(stmt.get(), 6));

  const int descriptionId = sqlite3_column_int(stmt.get(), 3);
  const std::string assigned = columnText(stmt.get(), 4);
  const std::string status = columnText(stmt.get(), 5);

  if (!assigned.empty()) {
    issue.assignTo(assigned);
  }
  if (descriptionId >= 0) {
    issue.setDescriptionCommentId(descriptionId);
  }
  if (!status.empty()) {
    issue.setStatus(status);
  }

  for (const auto& comment : loadComments(issueId)) {
    issue.addComment(comment);
  }
  return issue;
}

Issue SQLiteIssueRepository::saveIssue(const Issue& issue) {
  Issue stored = issue;

  if (!stored.hasPersistentId()) {
    SqliteStmt stmt(
        db_,
        "INSERT INTO issues (author_id, title, description_comment_id, "
        "assigned_to, status, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?);");
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
    sqlite3_bind_text(stmt.get(), 5, stored.getStatus().c_str(), -1,
                      SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt.get(), 6, stored.getTimestamp());

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
      throw std::runtime_error("Failed to insert issue");
    }
    int newId = static_cast<int>(sqlite3_last_insert_rowid(db_));
    stored.setIdForPersistence(newId);
    return getIssue(newId);
  }

  if (!issueExists(stored.getId())) {
    throw std::invalid_argument(
        "Issue with given ID does not exist: "
        + std::to_string(stored.getId()));
  }

  SqliteStmt stmt(
      db_,
      "UPDATE issues SET author_id = ?, title = ?, "
      "description_comment_id = ?, assigned_to = ?, "
      "status = ?, created_at = ? WHERE id = ?;");
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
  sqlite3_bind_text(stmt.get(), 5, stored.getStatus().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt.get(), 6, stored.getTimestamp());
  sqlite3_bind_int(stmt.get(), 7, stored.getId());

  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to update issue");
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
  forEachRow(
      "SELECT id FROM issues ORDER BY id ASC;",
      nullptr,
      [this, &issues](sqlite3_stmt* stmt) {
        int id = sqlite3_column_int(stmt, 0);
        issues.push_back(getIssue(id));
      });
  return issues;
}

// Generic filter used by specific find/list methods.
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

// --- Interface overrides that your controller uses ---

std::vector<Issue> SQLiteIssueRepository::findIssues(
    const std::string& userId) const {
  // Same semantics as in-memory repo: match author or assignee.
  return findIssues(
      [&userId](const Issue& issue) {
        if (issue.getAuthorId() == userId) {
          return true;
        }
        return issue.hasAssignee()
               && issue.getAssignedTo() == userId;
      });
}

std::vector<Issue> SQLiteIssueRepository::listAllUnassigned() const {
  return findIssues(
      [](const Issue& issue) {
        return !issue.hasAssignee();
      });
}

// Tags are not persisted in SQLite. Keep behaviour as "unsupported".
bool SQLiteIssueRepository::addTagToIssue(
    int /*issueId*/, const std::string& /*tag*/) {
  return false;
}

bool SQLiteIssueRepository::removeTagFromIssue(
    int /*issueId*/, const std::string& /*tag*/) {
  return false;
}

// --- Comments ---

Comment SQLiteIssueRepository::getComment(int issueId,
                                          int commentId) const {
  SqliteStmt stmt(
      db_,
      "SELECT id, author_id, text, timestamp FROM comments "
      "WHERE issue_id = ? AND id = ? LIMIT 1;");
  sqlite3_bind_int(stmt.get(), 1, issueId);
  sqlite3_bind_int(stmt.get(), 2, commentId);

  if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
    throw std::invalid_argument(
        "Comment does not belong to the given issue");
  }

  int id = sqlite3_column_int(stmt.get(), 0);
  std::string author = columnText(stmt.get(), 1);
  std::string text = columnText(stmt.get(), 2);
  Comment::TimePoint ts = sqlite3_column_int64(stmt.get(), 3);
  return Comment(id, author, text, ts);
}

std::vector<Comment> SQLiteIssueRepository::getAllComments(
    int issueId) const {
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

  SqliteStmt stmt(
      db_,
      "UPDATE comments SET author_id = ?, text = ?, timestamp = ? "
      "WHERE issue_id = ? AND id = ?;");
  sqlite3_bind_text(stmt.get(), 1, comment.getAuthor().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt.get(), 2, comment.getText().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt.get(), 3, comment.getTimeStamp());
  sqlite3_bind_int(stmt.get(), 4, issueId);
  sqlite3_bind_int(stmt.get(), 5, commentId);

  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to update comment");
  }

  return comment;
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

  SqliteStmt stmt(
      db_,
      "DELETE FROM comments WHERE issue_id = ? AND id = ?;");
  sqlite3_bind_int(stmt.get(), 1, issueId);
  sqlite3_bind_int(stmt.get(), 2, commentId);
  if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
    throw std::runtime_error("Failed to delete comment");
  }
  return sqlite3_changes(db_) > 0;
}

// --- Users ---

User SQLiteIssueRepository::getUser(const std::string& userId) const {
  SqliteStmt stmt(
      db_,
      "SELECT name, role FROM users WHERE name = ? LIMIT 1;");
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

  SqliteStmt stmt(
      db_,
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
  forEachRow(
      "SELECT name, role FROM users ORDER BY name ASC;",
      nullptr,
      [&users](sqlite3_stmt* stmt) {
        users.emplace_back(
            columnText(stmt, 0),
            columnText(stmt, 1));
      });
  return users;
}
