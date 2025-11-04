#include "IssueRepository.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace {
template <typename T>
const T& getOrThrowImpl(const std::unordered_map<int, T>& map, int id,
                        std::string errorMessage) {
  auto it = map.find(id);
  if (it == map.end()) {
    throw std::invalid_argument(errorMessage);
  }
  return it->second;
}

template <typename T>
T saveImpl(const T& in, std::unordered_map<int, T>* byId, int* nextId,
           std::string errorMessage) {
  T stored = in;
  if (stored.id == 0) {
    stored.id = ++(*nextId);                          // create
  } else if (byId->find(stored.id) == byId->end()) {  // id already exists
    throw std::invalid_argument(errorMessage);
  }
  (*byId)[stored.id] = stored;  // upsert
  return stored;
}
}  // namespace

class InMemoryIssueRepository : public IssueRepository {
 private:
  std::unordered_map<int, Issue> issuesById_;
  std::unordered_map<int, Comment> commentsById_;
  std::unordered_map<int, User> userById_;
  int nextIssueId_ = 0;
  int nextCommentId_ = 0;
  int nextUserId_ = 0;

 public:
  InMemoryIssueRepository() = default;

  // === Issues ===
  Issue getIssue(int issueId) const override {
    return getOrThrowImpl(issuesById_, issueId,
                          "Issue with given ID does not exist");
  }

  Issue saveIssue(const Issue& issue) override {
    return saveImpl(issue, &issuesById_, &nextIssueId_,
                    "Issue with given ID does not exist");
  }

  bool deleteIssue(int issueId) override {
    return issuesById_.erase(issueId) > 0;
  }

  std::vector<Issue> listIssues() const override {
    std::vector<Issue> issues;
    issues.reserve(issuesById_.size());
    std::transform(issuesById_.begin(), issuesById_.end(),
                   std::back_inserter(issues),
                   [](const auto& pair) { return pair.second; });
    return issues;
  }

  std::vector<Issue> findIssues(
      std::function<bool(const Issue&)> criteria) const override {
    std::vector<Issue> results;
    for (const auto& pair : issuesById_) {
      if (criteria(pair.second)) {
        results.push_back(pair.second);
      }
    }
    return results;
  }

  // === Comments ===
  Comment getComment(int userId) const override {
    return getOrThrowImpl(commentsById_, userId,
                          "Comment with given ID does not exist");
  }

  Comment saveComment(const Comment& comment) override {
    return saveImpl(comment, &commentsById_, &nextCommentId_,
                    "Comment with given ID does not exist");
  }

  bool deleteComment(int commentId) override {
    return commentsById_.erase(commentId) > 0;
  }

  // === Users ===
  User getUser(int userId) const override {
    return getOrThrowImpl(userById_, userId,
                          "User with given ID does not exist");
  }

  User saveUser(const User& user) override {
    return saveImpl(user, &userById_, &nextUserId_,
                    "User with given ID does not exist");
  }

  bool deleteUser(int userId) override { return userById_.erase(userId) > 0; }
};

std::unique_ptr<IssueRepository> createIssueRepository() {
  return std::make_unique<InMemoryIssueRepository>();
}
