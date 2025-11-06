// cppcheck-suppress unusedFunction
#include "IssueRepository.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace {
template <typename T>
const T& getOrThrow(const std::unordered_map<int, T>& map,
                    int id,
                    const std::string& errorMessage) {
    auto it = map.find(id);
    if (it == map.end()) {
        throw std::invalid_argument(errorMessage);
    }
    return it->second;
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
    std::unordered_map<int, Issue> issues_;
    std::unordered_map<int, Comment> comments_;
    std::unordered_map<std::string, User> users_;
    int nextIssueId_{0};
    int nextCommentId_{0};

    Issue hydrateIssue(const Issue& issue) const {
        Issue hydrated = issue;
        for (int commentId : issue.getCommentIds()) {
            auto it = comments_.find(commentId);
            if (it != comments_.end()) {
                hydrated.addComment(it->second);
            }
        }
        return hydrated;
    }

    std::vector<Comment> collectCommentsForIssue(const Issue& issue) const {
        std::vector<Comment> out;
        out.reserve(issue.getCommentIds().size());
        for (int commentId : issue.getCommentIds()) {
            auto it = comments_.find(commentId);
            if (it != comments_.end()) {
                out.push_back(it->second);
            }
        }
        std::sort(out.begin(), out.end(), [](const Comment& lhs,
                                            const Comment& rhs) {
            return lhs.getId() < rhs.getId();
        });
        return out;
    }

    int findIssueForComment(int commentId) const {
        for (const auto& pair : issues_) {
            const auto& ids = pair.second.getCommentIds();
            if (std::find(ids.begin(), ids.end(), commentId) != ids.end()) {
                return pair.first;
            }
        }
        return 0;
    }

 public:
    InMemoryIssueRepository() = default;

    Issue getIssue(int issueId) const override {
        return hydrateIssue(
            getOrThrow(issues_, issueId, "Issue with given ID does not exist"));
    }

    Issue saveIssue(const Issue& issue) override {
        Issue stored = issue;
        if (!stored.hasPersistentId()) {
            stored.setIdForPersistence(++nextIssueId_);
        } else if (issues_.find(stored.getId()) == issues_.end()) {
            throw std::invalid_argument("Issue with given ID does not exist: "+
                                        std::to_string(stored.getId()));
        }

        issues_[stored.getId()] = stored;
        return hydrateIssue(issues_.at(stored.getId()));
    }

    bool deleteIssue(int issueId) override {
        auto it = issues_.find(issueId);
        if (it == issues_.end()) {
            return false;
        }

        for (int commentId : it->second.getCommentIds()) {
            comments_.erase(commentId);
        }
        issues_.erase(it);
        return true;
    }

    std::vector<Issue> listIssues() const override {
        std::vector<Issue> out;
        out.reserve(issues_.size());
        std::transform(issues_.begin(), issues_.end(), std::back_inserter(out),
                       [&](const auto& entry) {
                           return hydrateIssue(entry.second);
                       });
        return out;
    }

    std::vector<Issue> findIssues(
        std::function<bool(const Issue&)> criteria) const override {
        std::vector<Issue> results;
        for (const auto& entry : issues_) {
            Issue hydrated = hydrateIssue(entry.second);
            if (criteria(hydrated)) {
                results.push_back(std::move(hydrated));
            }
        }
        return results;
    }

    // === Comments ===
    Comment getComment(int issueId, int commentId) const override {
        Issue issue = getIssue(issueId);  // hydrated with comments
        for (const Comment& c : issue.getComments()) {
            if (c.getId() == commentId) {
                return c;
            }
        }
        throw std::invalid_argument(
          "Comment does not belong to the given issue");
    }

    std::vector<Comment> getAllComments(int issueId) const override {
        Issue issue = getIssue(issueId);
        return collectCommentsForIssue(issue);
    }


    Comment saveComment(int issueId, const Comment& comment) override {
        auto issueIt = issues_.find(issueId);
        if (issueIt == issues_.end()) {
            throw std::invalid_argument("Issue with given ID does not exist");
        }

        Comment stored = comment;
        if (!stored.hasPersistentId()) {
            stored.setIdForPersistence(++nextCommentId_);
        } else if (comments_.find(stored.getId()) == comments_.end()) {
            throw std::invalid_argument("Comment with given ID does not exist");
        } else {
            nextCommentId_ = std::max(nextCommentId_, stored.getId());
        }

        comments_[stored.getId()] = stored;
        issueIt->second.addComment(stored);
        return stored;
    }

    bool deleteComment(int issueId, int commentId) override {
        auto issueIt = issues_.find(issueId);
        if (issueIt == issues_.end()) {
            throw std::invalid_argument("Issue with given ID does not exist");
        }

        auto commentIt = comments_.find(commentId);
        if (commentIt == comments_.end()) {
            throw std::invalid_argument("Comment with given ID does not exist");
        }

        if (!issueIt->second.removeComment(commentId)) {
            return false;
        }

        comments_.erase(commentIt);
        return true;
    }

    bool deleteComment(int commentId) override {
        auto commentIt = comments_.find(commentId);
        if (commentIt == comments_.end()) {
            throw std::invalid_argument("Comment with given ID does not exist");
        }

        int issueId = findIssueForComment(commentId);
        if (issueId != 0) {
            auto issueIt = issues_.find(issueId);
            if (issueIt != issues_.end()) {
                issueIt->second.removeComment(commentId);
            }
        }

        comments_.erase(commentIt);
        return true;
    }

    // === Users ===
    User getUser(const std::string& userId) const override {
        auto it = users_.find(userId);
        if (it == users_.end()) {
            throw std::invalid_argument("User with given ID does not exist");
        }
        return it->second;
    }

    User saveUser(const User& user) override {
        User stored = user;
        if (stored.getName().empty()) {
            throw std::invalid_argument("User ID must be non-empty");
        }
        auto [it, inserted] =
            users_.emplace(stored.getName(), stored);
        if (!inserted) {
            it->second = stored;
        }
        return stored;
    }

    bool deleteUser(const std::string& userId) override {
        return users_.erase(userId) > 0;
    }

    std::vector<User> listAllUsers() const override {
        std::vector<User> out;
        out.reserve(users_.size());
        for (const auto& entry : users_) {
            out.push_back(entry.second);
        }
        return out;
    }
};

IssueRepository* createIssueRepository() {
    return new InMemoryIssueRepository();
} // namespace
