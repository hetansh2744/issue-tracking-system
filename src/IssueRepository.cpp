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
           const std::string& errorMessage) {
  T stored = in;
  if (stored.getId() == 0) {
    // assign a persistent id on first save
    stored.setIdForPersistence(++(*nextId));            // create
  } else if (byId->find(stored.getId()) == byId->end()) {  // id already exists
    throw std::invalid_argument(errorMessage);
  }
  (*byId)[stored.getId()] = stored;  // upsert
  return stored;
}
}  // namespace

std::vector<Issue> IssueRepository::findIssues(
    const std::string& userId) const {
  return findIssues([&](const Issue& issue) {
    return issue.hasAssignee() && issue.getAssignedTo() == userId;
  });
}

std::vector<Issue> IssueRepository::listAllUnassigned() const {
  return findIssues([](const Issue& issue) { return !issue.hasAssignee(); });
}

class InMemoryIssueRepository : public IssueRepository {
 private:
  std::unordered_map<int, Issue> issuesById_;
  std::unordered_map<int, Comment> commentsById_;
  std::unordered_map<std::string, User> userById_;
  int nextIssueId_ = 0;
  int nextCommentId_ = 0;

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
  Comment getComment(int commentId) const override {
    return getOrThrowImpl(commentsById_, commentId,
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
  User getUser(const std::string& userId) const override {
    auto it = userById_.find(userId);
    if (it == userById_.end()) {
      throw std::invalid_argument("User with given ID does not exist");
    }
    return it->second;
  }

  User saveUser(const User& user) override {
    User stored = user;
    const std::string id = stored.getName();
    if (id.empty()) {
      throw std::invalid_argument("User ID must be non-empty");
    }
    auto [it, inserted] = userById_.emplace(id, stored);
    if (!inserted) {
      it->second = stored;
    }
    return stored;
  }

  bool deleteUser(const std::string& userId) override {
    return userById_.erase(userId) > 0;
  }

  std::vector<User> listAllUsers() const override {
    std::vector<User> users;
    users.reserve(userById_.size());
    //for (const auto& pair : userById_) {
      //users.push_back(pair.second);
    //}
    std::transform(someMap.begin(), someMap.end(),
               std::back_inserter(users),
               [](const auto& pair) { return pair.second; });
    //return users;
  }
};

IssueRepository* createIssueRepository() {
  return new InMemoryIssueRepository();
}
