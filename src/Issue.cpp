#include "Issue.hpp"

Issue::Issue(int id,
             std::string author_id,
             std::string title,
             TimePoint created_at)
    : id_{id},
      author_id_{std::move(author_id)},
      title_{std::move(title)},
      created_at_{created_at} {
  if (id_ < 0) {
    throw std::invalid_argument("id must be >= 0");
  }
  if (author_id_.empty()) {
    throw std::invalid_argument("authorId must not be empty");
  }
  if (title_.empty()) {
    throw std::invalid_argument("title must not be empty");
  }
}

void Issue::setIdForPersistence(int new_id) {
  if (hasPersistentId()) {
    throw std::logic_error("id already set");
  }
  if (new_id <= 0) {
    throw std::invalid_argument("new_id must be > 0");
  }
  id_ = new_id;
}

void Issue::setTitle(std::string new_title) {
  if (new_title.empty()) {
    throw std::invalid_argument("title must not be empty");
  }
  title_ = std::move(new_title);
}

void Issue::addComment(int comment_id) {
  if (comment_id <= 0) {
    throw std::invalid_argument("comment_id must be > 0");
  }
  auto it = std::find(comment_ids_.begin(), comment_ids_.end(), comment_id);
  if (it == comment_ids_.end()) {
    comment_ids_.push_back(comment_id);
  }
}

bool Issue::removeComment(int comment_id) {
  auto it = std::find(comment_ids_.begin(), comment_ids_.end(), comment_id);
  if (it == comment_ids_.end()) {
    return false;
  }
  if (description_comment_id_ == comment_id) {
    description_comment_id_ = 0;  // clear description
  }
  comment_ids_.erase(it);
  return true;
}

void Issue::setDescriptionCommentId(int comment_id) {
  if (comment_id <= 0) {
    throw std::invalid_argument("comment_id must be > 0");
  }
  auto it = std::find(comment_ids_.begin(), comment_ids_.end(), comment_id);
  if (it == comment_ids_.end()) {
    comment_ids_.push_back(comment_id);
  }
  description_comment_id_ = comment_id;
}
