#ifndef MILESTONE_HPP
#define MILESTONE_HPP

#include "Issue.hpp"
#include "Comment.hpp"
#include <string>
#include <vector>
#include <iostream>

class Milestone {
 private:
//core fields
    int mID_;
    std::string name_;
    std::string start_date;
    std::string end_date;
    std::string descrip_;
//saerching functions
int issues_attached{0};
std::vector<Issue> issue_;
std::vector<int> issue_id;

 public:
    Milestone() = default;
    Milestone(int mID,
                std::string name_,
                std::string descrip_,
                std::string start_date,
                std::string end_date);
    //setters
    int setID(int id);
    std::string setName(std::string newname_);
    std::string setDescription(std::string newdescrip_);
    std::string setDateStart(std::string start_date);
    std::string setDateEnd(std::string end_date);

    int getID()const noexcept{ return mID_;}
    std::string getName()const noexcept{return name_;}
    std::string getDescription() const noexcept{return descrip_;}
    std::string getStartDate() const noexcept{return start_date;}
    std::string getEndDate() const noexcept{return end_date;}


    const std::vector<int>& getIssueIds() const noexcept {
        return issue_id;
    }
    const std::vector<Issue>& getIssues() const noexcept {
        return issue_;
    }


};

#endif
