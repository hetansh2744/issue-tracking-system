#include "IssueRepository.hpp"

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

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
  // === Issues/Users ===
  std::unordered_map<int, Issue> issuesById_;
  std::unordered_map<std::string, User> userById_;
  int nextIssueId_ = 0;

  // === Comments ===
  // Global comment store and mapping commentId -> issueId
  std::unordered_map<int, Comment> comments_;
  std::unordered_map<int, int> commentToIssue_;
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
    for (auto it = commentToIssue_.begin(); it != commentToIssue_.end();) {
      if (it->second == issueId) {
        comments_.erase(it->first);
        it = commentToIssue_.erase(it);
      } else {
        ++it;
      }
    }
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
  // All comments for an issue
  std::vector<Comment> getAllComments(int issueId) const override {
    // validate issue existence
    (void)getIssue(issueId);
    std::vector<Comment> out;
    for (const auto& kv : commentToIssue_) {
      if (kv.second == issueId) {
        auto cit = comments_.find(kv.first);
        if (cit != comments_.end()) out.push_back(cit->second);
      }
    }
    std::sort(out.begin(), out.end(),
              [](const Comment& a, const Comment& b) {
                return a.getId() < b.getId();
              });
    return out;
  }

  // Specific comment scoped by issue
  std::vector<Comment> getComment(int issueId, int commentId) const override {
    // validate issue existence
    (void)getIssue(issueId);
    auto mapIt = commentToIssue_.find(commentId);
    if (mapIt == commentToIssue_.end() || mapIt->second != issueId) {
      return {};
    }
    return {getComment(commentId)};
  }

  Comment saveComment(const Comment& comment) override {
    const int issueId = comment.getId();
    if (issueId <= 0) {
      throw std::invalid_argument("Comment must reference a valid issueId");
    }
    // ensure issue exists
    (void)getIssue(issueId);

    Comment stored = comment;
    if (!stored.hasPersistentId() || stored.getId() == 0) {
      stored.setIdForPersistence(++nextCommentId_);
    } else if (comments_.find(stored.getId()) == comments_.end()) {
      // if caller sets a non-existent id, treat as invalid
      throw std::invalid_argument("Comment with given ID does not exist");
    } else {
      // keep nextCommentId_ monotonic
      nextCommentId_ = std::max(nextCommentId_, stored.getId());
    }

    comments_[stored.getId()] = stored;
    commentToIssue_[stored.getId()] = issueId;
    return stored;
  }

  // Delete by (issueId, commentId); comment #1 cannot be deleted
  bool deleteComment(int issueId, int commentId) override {
    if (issuesById_.find(issueId) == issuesById_.end()) return false;

    if (commentId == 1) {
      // #1 is the description; cannot delete
      return false;
    }

    auto mapIt = commentToIssue_.find(commentId);
    if (mapIt == commentToIssue_.end() || mapIt->second != issueId) {
      return false;
    }

    commentToIssue_.erase(mapIt);
    return comments_.erase(commentId) > 0;
  }

  // Retain the single-id delete (compat). Disallows removing #1 as well.
  bool deleteComment(int commentId) override {
    if (commentId == 1) return false;
    auto mapIt = commentToIssue_.find(commentId);
    if (mapIt != commentToIssue_.end()) {
      commentToIssue_.erase(mapIt);
    }
    return comments_.erase(commentId) > 0;
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
    for (const auto& pair : userById_) {
      users.push_back(pair.second);
    }
    return users;
  }
};

IssueRepository* createIssueRepository() {
  return new InMemoryIssueRepository();
}
