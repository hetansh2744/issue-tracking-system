#ifndef ISSUE_HPP_
#define ISSUE_HPP_

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class Issue {
 public:
  using TimePoint = std::int64_t; // epoch ms (0 if unknown)

 private:
  int id_{0};  // 0 -> not yet assigned/persisted
  std::string author_id_;
  std::string title_;

  int description_comment_id_{0}; // 0 -> no description
  std::string assigned_to_; // empty -> unassigned

  std::vector<int> comment_ids_;
  TimePoint created_at_{0};

 public:
  Issue() = default;

  Issue(int id,
        std::string author_id,
        std::string title,
        TimePoint created_at = 0);

  // ---- id helpers ----
  bool hasPersistentId() const noexcept { return id_ > 0; }
  int getId() const noexcept { return id_; }
  void setIdForPersistence(int new_id);  // repo calls once (new_id > 0)

  // ---- accessors ----
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
  TimePoint getTimestamp() const noexcept { return created_at_; }

  // ---- mutators / rules ----
  void setTitle(std::string new_title);
  void setDescriptionCommentId(int comment_id); //add to list if missing

  void assignTo(std::string user_id) { assigned_to_ = std::move(user_id); }
  void unassign() { assigned_to_.clear(); }

  void addComment(int comment_id); // >0, de-dupe
  bool removeComment(int comment_id);// clears desk if nescessary
};

#endif  // ISSUE_HPP_
