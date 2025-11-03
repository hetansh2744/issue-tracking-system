#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <vector>

class User {
 public:
    std::string id;
    std::string name;
    std::vector<int> assignedIssueIds;

    // Default constructor for returning failure/invalid objects
    User(const std::string& name = "", const std::string& id = "")
        : name(name), id(id) {}

    // Minimal methods
    void addAssignedIssue(int issueId) { assignedIssueIds.push_back(issueId); }
    void removeAssignedIssue(int issueId); // Implementation needed later
};


class Comment {
 public:
    int id;
    int issueId;
    std::string authorId; // References User::id
    std::string text;
    std::string timestamp; // Placeholder for date/time

    // Default constructor for returning failure/invalid objects
    Comment(int issueId = 0, const std::string& authorId = "",
    const std::string& text = "")
        : id(0), issueId(issueId), authorId(authorId), text(text) {}

    // Minimal method
    void setTimestamp(const std::string& ts) { timestamp = ts; }
};


class Issue {
 public:
    int id;
    std::string title;
    std::string description;
    std::string assignedTo; // References User::id
    std::vector<int> commentIds; // References Comment::id

    // Default constructor for returning failure/invalid objects
    Issue(const std::string& title = "", const std::string& desc = "",
          const std::string& assignedTo = "")
        : id(0), title(title), description(desc), assignedTo(assignedTo) {}

    // Minimal methods for the Controller logic
    void addComment(int commentId) { commentIds.push_back(commentId); }
    void removeComment(int commentId); // Implementation needed later
};

#endif // ENTITIES_H
