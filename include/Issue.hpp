#ifndef ISSUE_HPP_
#define ISSUE_HPP_

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "Comment.hpp"

/// Rules:
/// - New Issue: id==0. Repo assigns >0 once via setIdForPersistence().
/// - author_id_ and title_ must be non-empty.
/// - description_comment_id_==0 means "no description".
/// - assigned_to_ empty means "unassigned".
class Issue {
 public:
  /// epoch ms; 0 means "unknown/unset"
  using TimePoint = std::int64_t;

 private:
  // core fields
  int id_{0};                  ///< 0 => new, not persisted
  std::string author_id_;      ///< non-empty
  std::string title_;          ///< non-empty

  // relations / metadata
  int description_comment_id_{0};  ///< 0 => none
  std::string assigned_to_;        ///< user id; empty => unassigned

  // ids for persistence + full objects for lookups
  std::vector<int> comment_ids_;
  std::vector<Comment> comments_;

  TimePoint created_at_{0};

 public:
  // ctors
  Issue() = default;

  /// Validating ctor
  Issue(int id,
        std::string author_id,
        std::string title,
        TimePoint created_at = 0);

  // id helpers
  bool hasPersistentId() const noexcept { return id_ > 0; }
  int getId() const noexcept { return id_; }
  void setIdForPersistence(int new_id);

  // accessors
  const std::string& getAuthorId() const noexcept { return author_id_; }
  const std::string& getTitle() const noexcept { return title_; }

  bool hasDescriptionComment() const noexcept {
    return description_comment_id_ > 0;
  }

  int getDescriptionCommentId() const noexcept {
    return description_comment_id_;
  }

  bool hasAssignee() const noexcept { return !assigned_to_.empty(); }
  const std::string& getAssignedTo() const noexcept { return assigned_to_; }

  const std::vector<int>& getCommentIds() const noexcept {
    return comment_ids_;
  }

  const std::vector<Comment>& getComments() const noexcept {
    return comments_;
  }

  TimePoint getTimestamp() const noexcept { return created_at_; }

  // mutators / rules
  void setTitle(std::string new_title);
  void setDescriptionCommentId(int comment_id);

  void assignTo(std::string user_id) { assigned_to_ = std::move(user_id); }
  void unassign() { assigned_to_.clear(); }

  void addComment(int comment_id);
  bool removeComment(int comment_id);

  // full Comment object API, here the comments can be added/removed/found
  void addComment(const Comment& comment);
  void addComment(Comment&& comment);

  const Comment* findCommentById(int id) const noexcept;
  Comment* findCommentById(int id) noexcept;

  bool removeCommentById(int id);
};

#endif  // ISSUE_HPP_
