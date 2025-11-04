#ifndef COMMENT_HPP_
#define COMMENT_HPP_

#include <cstdint>
#include <stdexcept>
#include <string>

class Comment {
 public:
  using TimePoint = std::int64_t;  // epoch ms (0 if unset)

 private:
  int id_{0};
  std::string author_id_;
  std::string text_;
  TimePoint timestamp_{0};

 public:
  Comment() = default;

  Comment(int id,
          std::string author_id,
          std::string text,
          TimePoint timestamp = 0);

  // ---- id helpers ----
  bool hasPersistentId() const noexcept;
  int getId() const noexcept;
  void setIdForPersistence(int new_id);

  // ---- accessors ----
  const std::string& getAuthor() const noexcept;
  const std::string& getText() const noexcept;
  TimePoint getTimeStamp() const noexcept;

  // ---- mutators / rules ----
  void setText(std::string new_text);  // non-empty
  void setTimeStamp(TimePoint ts);
};

#endif  // COMMENT_HPP_
