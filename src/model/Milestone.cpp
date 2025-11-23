#include "Milestone.hpp"
#include <algorithm>

Milestone::Milestone(int mID_,
                     std::string name_,
                     std::string descrip_,
                     std::string start_date,
                     std::string end_date)
  : mID_{mID_}, 
    name_{std::move(name_)},
    descrip_{std::move(descrip_)},
    start_date{std::move(start_date)},
    end_date{std::move(end_date)} {
  
  // Only validate if trying to create a "real" milestone
  if (mID_ >= 0) {
    if (name_.empty()) {
      throw std::invalid_argument("Must fill name");
    }
    if (start_date.empty()) {
      throw std::invalid_argument("Must fill start date");
    }
    if (end_date.empty()) {
      throw std::invalid_argument("Must fill end date");
    }
  }
}

void Milestone::setName(std::string newname_) {
  if (newname_.empty()) {
    throw std::invalid_argument("Name cannot be empty");
  }
  name_ = std::move(newname_);
}

void Milestone::setDescription(std::string newdescrip_) {
  descrip_ = std::move(newdescrip_);
}

void Milestone::setDateEnd(std::string newend_date_) {
  if (newend_date_.empty()) {
    throw std::invalid_argument("End date cannot be empty");
  }
  end_date = std::move(newend_date_);
}

void Milestone::setDateStart(std::string newstart_date_) {
  if (newstart_date_.empty()) {
    throw std::invalid_argument("Start date cannot be empty");
  }
  start_date = std::move(newstart_date_);
}