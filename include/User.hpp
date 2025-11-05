#ifndef USER_H
#define USER_H

#include <iostream>
#include <string>

class User {
 private:
    std::string name;
    std::string role; 
public:
    User(std::string name, std::string role) : name(name), role(role) {}
    std::string getName();
    std::string getRole();
    void setName(const std::string &newname);
    void setRole(const std::string &newrole);
};
#endif