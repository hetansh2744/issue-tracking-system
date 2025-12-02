#ifndef TAG_HPP_
#define TAG_HPP_

#include <string>

/**
 * @brief Simple value object representing a tag with a name and color.
 */
class Tag {
 private:
  std::string name_;
  std::string color_;

 public:
  Tag() = default;
  Tag(std::string name, std::string color)
      : name_(std::move(name)), color_(std::move(color)) {}

  const std::string& getName() const noexcept { return name_; }
  const std::string& getColor() const noexcept { return color_; }

  void setName(std::string name) { name_ = std::move(name); }
  void setColor(std::string color) { color_ = std::move(color); }
};

#endif  // TAG_HPP_
