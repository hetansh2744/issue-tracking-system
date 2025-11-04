#include "Comment.hpp"
#include <utility>

Comment::Comment(int id,
                 std::string author_id,
                 std::string text,
                 TimePoint timestamp)
    : id_{id},
      author_id_{std::move(author_id)},
      text_{std::move(text)},
      timestamp_{timestamp} {
  if (id_ < 0) throw std::invalid_argument("id must be >= 0");
  if (author_id_.empty()) throw std::invalid_argument("authorId empty");
  if (text_.empty()) throw std::invalid_argument("text empty");
}

bool Comment::hasPersistentId() const noexcept { return id_ > 0; }
int Comment::getId() const noexcept { return id_; }

void Comment::setIdForPersistence(int new_id) {
  if (hasPersistentId()) throw std::logic_error("id already set");
  if (new_id <= 0) throw std::invalid_argument("new_id must be > 0");
  id_ = new_id;
}

const std::string& Comment::getAuthor() const noexcept { return author_id_; }
const std::string& Comment::getText() const noexcept { return text_; }
Comment::TimePoint Comment::getTimeStamp() const noexcept { return timestamp_; }

void Comment::setText(std::string new_text) {
  if (new_text.empty()) throw std::invalid_argument("text empty");
  text_ = std::move(new_text);
}

void Comment::setTimeStamp(TimePoint ts) { timestamp_ = ts; }
