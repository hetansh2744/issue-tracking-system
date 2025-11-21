#ifndef ISSUE_REPOSITORY_H_INCLUDED
#define ISSUE_REPOSITORY_H_INCLUDED

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "Comment.hpp"
#include "Issue.hpp"
#include "User.hpp"

/**
 * @brief Abstract repository interface for issue tracking data operations
 *
 * The IssueRepository defines the contract for data persistence operations
 * in the issue tracking system. It provides methods for managing issues,
 * comments, and users through a repository pattern.
 */
class IssueRepository {
 public:
  // === Issues ===

  /**
   * @brief Retrieves an issue by its unique identifier
   *
   * @param issueId The ID of the issue to retrieve
   * @return Issue The requested issue object
   */
  virtual Issue getIssue(int issueId) const = 0;

  /**
   * @brief Saves an issue (creates new or updates existing)
   *
   * @param issue The issue object to save
   * @return Issue The saved issue with updated persistence data
   */
  virtual Issue saveIssue(const Issue& issue) = 0;

  /**
   * @brief Deletes an issue from the repository
   *
   * @param issueId The ID of the issue to delete
   * @return bool True if deletion was successful, false otherwise
   */
  virtual bool deleteIssue(int issueId) = 0;

  /**
   * @brief Retrieves all issues in the repository
   *
   * @return std::vector<Issue> List of all issues
   */
  virtual std::vector<Issue> listIssues() const = 0;

  /**
   * @brief Finds issues matching custom criteria
   *
   * @param criteria Function that defines the matching condition
   * @return std::vector<Issue> List of issues matching the criteria
   */
  virtual std::vector<Issue> findIssues(
      std::function<bool(const Issue&)> criteria) const = 0;

  /**
   * @brief Finds issues assigned to a specific user
   *
   * @param userId The ID of the user to search for
   * @return std::vector<Issue> List of issues assigned to the user
   */
  virtual std::vector<Issue> findIssues(const std::string& userId) const;

  /**
   * @brief Retrieves all unassigned issues
   *
   * @return std::vector<Issue> List of issues with no assignee
   */
  virtual std::vector<Issue> listAllUnassigned() const;

  // === Comments ===

  /**
   * @brief Retrieves a specific comment from an issue
   *
   * @param commentId The ID of the comment to retrieve
   * @param issueId The ID of the issue containing the comment
   * @return Comment The requested comment object
   */
  virtual Comment getComment(int issueId, int commentId) const = 0;

  /**
   * @brief Retrieves all comments for a specific issue
   *
   * @param issueId The ID of the issue
   * @return std::vector<Comment> List of all comments on the issue
   */
  virtual std::vector<Comment> getAllComments(int issueId) const = 0;

  /**
   * @brief Saves a comment to an issue
   *
   * @param issueId The ID of the issue to add the comment to
   * @param comment The comment object to save
   * @return Comment The saved comment with updated persistence data
   */
  virtual Comment saveComment(int issueId, const Comment& comment) = 0;

  /**
   * @brief Deletes a comment from a specific issue
   *
   * @param issueId The ID of the issue containing the comment
   * @param commentId The ID of the comment to delete
   * @return bool True if deletion was successful, false otherwise
   */
  virtual bool deleteComment(int issueId, int commentId) = 0;

  // === Users ===

  /**
   * @brief Retrieves a user by their unique identifier
   *
   * @param userId The ID of the user to retrieve
   * @return User The requested user object
   */
  virtual User getUser(const std::string& userId) const = 0;

  /**
   * @brief Saves a user (creates new or updates existing)
   *
   * @param user The user object to save
   * @return User The saved user with updated persistence data
   */
  virtual User saveUser(const User& user) = 0;

  /**
   * @brief Deletes a user from the repository
   *
   * @param userId The ID of the user to delete
   * @return bool True if deletion was successful, false otherwise
   */
  virtual bool deleteUser(const std::string& userId) = 0;

  /**
   * @brief Retrieves all users in the repository
   *
   * @return std::vector<User> List of all users
   */
  virtual std::vector<User> listAllUsers() const = 0;

  /**
   * @brief Virtual destructor for proper inheritance support
   */
  virtual ~IssueRepository() = default;
};

/**
 * @brief Factory function to create a repository instance
 *
 * @return IssueRepository* Pointer to a new repository instance
 */
IssueRepository* createIssueRepository();

#endif  // ISSUE_REPOSITORY_H_INCLUDED
