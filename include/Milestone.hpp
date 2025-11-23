#ifndef MILESTONE_HPP
#define MILESTONE_HPP

#include <string>
#include <vector>
#include <stdexcept>

class Milestone {
private:
  int mID_;
  std::string name_;
  std::string descrip_;
  std::string start_date;
  std::string end_date;
  std::vector<int> issue_ids_;  // Store issue IDs

public:
  // Your existing constructor
  Milestone(int mID_ = -1,
            std::string name_ = "",
            std::string descrip_ = "",
            std::string start_date = "",
            std::string end_date = "");
  
  // Getters
  int getId() const { return mID_; }
  std::string getName() const { return name_; }
  std::string getDescription() const { return descrip_; }
  std::string getStartDate() const { return start_date; }
  std::string getEndDate() const { return end_date; }
  const std::vector<int>& getIssueIds() const { return issue_ids_; }
  
  // Check if has persistent ID
  bool hasPersistentId() const { return mID_ >= 0; }
  
  // Set ID (for persistence layer only)
  void setIdForPersistence(int id) { mID_ = id; }
  
  // Your existing setters
  void setName(std::string newname_);
  void setDescription(std::string newdescrip_);
  void setDateEnd(std::string newend_date_);
  void setDateStart(std::string newstart_date_);
  
  // Issue management
  void addIssue(int issueId) {
    // Check if already added
    for (int id : issue_ids_) {
      if (id == issueId) return;
    }
    issue_ids_.push_back(issueId);
  }
  
  void removeIssue(int issueId) {
    issue_ids_.erase(
      std::remove(issue_ids_.begin(), issue_ids_.end(), issueId),
      issue_ids_.end()
    );
  }
  
  bool hasIssue(int issueId) const {
    for (int id : issue_ids_) {
      if (id == issueId) return true;
    }
    return false;
  }
  
  int getIssueCount() const { return issue_ids_.size(); }
};

#endif