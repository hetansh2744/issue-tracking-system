#include "Milestone.hpp"

Milestone::Milestone(int mID_,
                std::string name_,
                std::string descrip_,
                std::string start_date,
                std::string end_date) : 
                mID_{mID_}, name_{name_},
                descrip_{descrip_},
                start_date{std::move(start_date)},
                end_date{std::move(end_date)} {
                    if(mID_ < 0){
                        throw(std::invalid_argument("id must be greater than 0"));
                    }
                    if(name_.empty()){
                        throw(std::invalid_argument("Must fill name"));
                    }
                    if(descrip_.empty()){
                        throw(std::invalid_argument("Must fill description"));
                    }
                    if(start_date.empty()){
                        throw(std::invalid_argument("Must fill start date"));
                    }
                    if(end_date.empty()){
                        throw(std::invalid_argument("Must fill end date"));
                    }
                };

std::string Milestone::setName(std::string newname_){
    if(name_.empty()){
        throw(std::invalid_argument("Must have a title name"));
    }
    name_ = std::move(newname_);
}

std::string Milestone::setDescription(std::string newdescrip_){
    if(descrip_.empty()){
        throw(std::invalid_argument("Must have a title name"));
    }
    descrip_ = std::move(newdescrip_);
}

std::string Milestone::setDateEnd(std::string newend_date_){
    if(end_date.empty()){
        throw(std::invalid_argument("Must have a title name"));
    }
    end_date = std::move(newend_date_);
}
std::string Milestone::setDateStart(std::string newstart_date_){
    if(start_date.empty()){
        throw(std::invalid_argument("Must have a title name"));
    }
    start_date = std::move(newstart_date_);
}

