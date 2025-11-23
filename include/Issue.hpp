#ifndef ISSUE_HPP_
#define ISSUE_HPP_

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <set>
#include "Comment.hpp"

/**
 * @brief Domain model for an issue.
 *
 * Functions as aggregate root for Comments:
 *  - New Issue starts with id == 0; repository assigns > 0 via
 *    setIdForPersistence() once.
 *  - author_id_ and title_ are non-empty (validated).
 *  - description_comment_id_ == -1 => no description linked.
 *  - assigned_to_ empty => unassigned.
 *  - We keep both comment id list (persistence) and Comment objects
 *    (in-memory lookups/edits).
 */
class Issue {
 public:
  /// @brief Epoch milliseconds; 0 means unknown/unset.
  using TimePoint = std::int64_t;

 private:
  // Core fields
  int id_{0};              ///< 0 => new (not yet persisted)
  std::string author_id_;  ///< non-empty creator user id
  std::string title_;      ///< non-empty short summary

  // Relationships / metadata
  int description_comment_id_{-1};       ///< -1 => none linked
  std::string assigned_to_;              ///< assignee user id; empty => none
  std::string status_{"To Be Done"};     ///< issue status

  // Persistence ids + in-memory objects
  std::vector<int> comment_ids_;   ///< unique attached comment ids
  std::vector<Comment> comments_;  ///< stored Comment objects

  TimePoint created_at_{0};        ///< creation time; 0 => unknown
  std::set<std::string> tags_;

 public:
  /// @brief Default construct (id==0, empty fields).
  Issue() = default;

  /**
   * @brief Construct and validate an issue.
   * @param id          >= 0 (0 new; >0 persisted)
   * @param author_id   non-empty creator id
   * @param title       non-empty title
   * @param created_at  epoch ms (0 allowed for unknown)
   * @throws std::invalid_argument on bad inputs
   */
  Issue(int id,
        std::string author_id,
        std::string title,
        TimePoint created_at = 0);

  // ---------------------------
  // id helpers (persistence)
  // ---------------------------

  /**
   * @brief Whether issue has a persistent id.
   * @return true if id_ > 0, else false.
   */
  bool hasPersistentId() const noexcept { return id_ > 0; }

  /**
   * @brief Get current id.
   * @return id (0 if not yet persisted).
   */
  int getId() const noexcept { return id_; }

  /**
   * @brief Assign persistent id exactly once.
   * @param new_id  > 0
   * @throws std::logic_error if id already set
   * @throws std::invalid_argument if new_id <= 0
   */
  void setIdForPersistence(int new_id);

  // ---------------------------
  // accessors
  // ---------------------------

  /**
   * @brief Get creator user id.
   * @return non-empty author id.
   */
  const std::string& getAuthorId() const noexcept { return author_id_; }

  /**
   * @brief Get title.
   * @return non-empty title.
   */
  const std::string& getTitle() const noexcept { return title_; }

  /**
   * @brief Whether description comment is linked.
   * @return true if description_comment_id_ >= 0.
   */
  bool hasDescriptionComment() const noexcept {
    return description_comment_id_ >= 0;
  }

  /**
   * @brief Get description comment id.
   * @return id (0 if none).
   */
  int getDescriptionCommentId() const noexcept {
    return description_comment_id_;
  }

  /**
   * @brief Whether issue has an assignee.
   * @return true if assigned_to_ not empty.
   */
  bool hasAssignee() const noexcept { return !assigned_to_.empty(); }

  /**
   * @brief Get assignee user id.
   * @return user id (empty if unassigned).
   */
  const std::string& getAssignedTo() const noexcept { return assigned_to_; }

  /**
   * @brief Get current status of the issue.
   * @return status string (e.g., "To Be Done", "In Progress", "Done").
   */
  const std::string& getStatus() const noexcept { return status_; }

  /**
   * @brief Get list of comment ids (read-only).
   * @return const ref to id vector.
   */
  const std::vector<int>& getCommentIds() const noexcept {
    return comment_ids_;
  }

  /**
   * @brief Get list of stored Comment objects (read-only).
   * @return const ref to comments_ vector.
   */
  const std::vector<Comment>& getComments() const noexcept {
    return comments_;
  }

  /**
   * @brief Get creation timestamp.
   * @return epoch ms (0 if unknown).
   */
  TimePoint getTimestamp() const noexcept { return created_at_; }

  /**
   * @brief Set creation timestamp.
   * @param ts epoch ms (>= 0)
   * @throws std::invalid_argument if ts < 0
   */
  void setTimestamp(TimePoint ts);

  // ---------------------------
  // mutators / rules
  // ---------------------------

  /**
   * @brief Set a new title.
   * @param new_title  non-empty
   * @throws std::invalid_argument if empty
   */
  void setTitle(std::string new_title);

  /**
   * @brief Link description to a comment id and ensure it is tracked
   *        in comment_ids_.
   * @param comment_id  >= 0
   * @throws std::invalid_argument if comment_id < 0
   */
  void setDescriptionCommentId(int comment_id);

  /**
   * @brief Assign the issue to a user id (empty clears).
   * @param user_id user id (empty allowed to clear)
   */
  void assignTo(std::string user_id) { assigned_to_ = std::move(user_id); }

  /// @brief Clear the assignee.
  void unassign() { assigned_to_.clear(); }

  /**
   * @brief Set the status of the issue.
   *
   * For this project we use:
   *  - "To Be Done"
   *  - "In Progress"
   *  - "Done"
   *
   * The view/controller are responsible for passing a sensible value.
   */
  void setStatus(std::string status) { status_ = std::move(status); }

  /**
   * @brief Add a comment id to comment_ids_ (de-duplicated).
   * @param comment_id  >= 0
   * @throws std::invalid_argument if comment_id < 0
   */
  void addComment(int comment_id);

  /**
   * @brief Remove a comment id. Clears description if
   *        it pointed to that id.
   * @param comment_id
   * @return true if removed; false if not found
   */
  bool removeComment(int comment_id);

  // ---------------------------
  // full Comment object API
  // ---------------------------

  /**
   * @brief Upsert a Comment (copy) by id into comments_. Ensures its id
   *        is in comment_ids_.
   * @param comment  Comment with id >= 0
   * @throws std::invalid_argument if comment id < 0
   */
  void addComment(const Comment& comment);

  /**
   * @brief Upsert a Comment (move) by id into comments_. Ensures its id
   *        is in comment_ids_.
   * @param comment  rvalue Comment with id >= 0
   * @throws std::invalid_argument if comment id < 0
   */
  void addComment(Comment&& comment);

  /**
   * @brief Find a comment by id (read-only).
   * @param id  comment id
   * @return pointer to Comment or nullptr if not found
   */
  const Comment* findCommentById(int id) const noexcept;

  /**
   * @brief Find a comment by id (mutable).
   * @param id  comment id
   * @return pointer to Comment or nullptr if not found
   */
  Comment* findCommentById(int id) noexcept;

  /**
   * @brief Remove a Comment object by id.
   * @param id comment id
   * @return true if removed from either store; false if not found
   */
  bool removeCommentById(int id);

  /**
   * @brief Getter method for time of creation.
   * @return const reference to creation time.
   */
  const TimePoint& getCreatedAt() const { return created_at_; }

  // ---------------------------
  // tags
  // ---------------------------

  /**
   * @brief Add a tag to the issue.
   * @param tag non-empty tag string
   * @return true if the tag was newly added, false if it already existed
   * @throws std::invalid_argument if tag is empty
   */
  bool addTag(const std::string& tag);

  /**
   * @brief Remove a tag from the issue.
   * @param tag tag to remove
   * @return true if the tag was removed, false if it did not exist
   */
  bool removeTag(const std::string& tag);

  /**
   * @brief Check if the issue has a given tag.
   * @param tag tag to check
   * @return true if the tag exists on this issue
   */
  bool hasTag(const std::string& tag) const;

  /**
   * @brief Get all tags on this issue.
   * @return set of tags
   */
  std::set<std::string> getTags() const;
};

#endif  // ISSUE_HPP_
