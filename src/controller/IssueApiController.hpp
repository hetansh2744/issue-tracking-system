#ifndef ISSUE_API_CONTROLLER_HPP_
#define ISSUE_API_CONTROLLER_HPP_

#include <algorithm>
#include <memory>
#include <string>
#include <cctype>

#include "Comment.hpp"
#include "CommentDto.hpp"
#include "DatabaseDto.hpp"
#include "Issue.hpp"
#include "IssueDto.hpp"
#include "TagDto.hpp"
#include "User.hpp"
#include "UserDto.hpp"
#include "Milestone.hpp"
#include "MilestoneDto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "service/DatabaseService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class IssueApiController : public oatpp::web::server::api::ApiController {
 private:
  std::shared_ptr<DatabaseService> dbService;
  static std::string asStdString(const oatpp::String& value) {
    return value ? *value : std::string();
  }
  IssueService& issues() const { return dbService->getIssueService(); }
  static std::string withDbExtension(const std::string& name) {
    if (name.size() >= 3 && name.substr(name.size() - 3) == ".db") {
      return name;
    }
    return name + ".db";
  }

 public:
  IssueApiController(
      const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& objectMapper)
      : oatpp::web::server::api::ApiController(objectMapper),
        dbService(std::make_shared<DatabaseService>()) {}

  // Convert helpers
  static oatpp::Object<IssueDto> issueToDto(const Issue& i) {
    auto dto = IssueDto::createShared();
    dto->id = i.getId();
    dto->authorId = i.getAuthorId().c_str();
    dto->title = i.getTitle().c_str();
    dto->description = "";
    dto->assignedTo = i.hasAssignee() ? i.getAssignedTo().c_str() : "";

    auto ids = oatpp::List<oatpp::Int32>::createShared();
    for (int cid : i.getCommentIds()) {
      ids->push_back(cid);
    }
    dto->commentIds = ids;

    auto tags = oatpp::List<oatpp::String>::createShared();
    for (const auto& t : i.getTags()) {
      tags->push_back(t.c_str());
    }
    dto->tags = tags;

    dto->createdAt = i.getCreatedAt();
    return dto;
  }

  static oatpp::Object<CommentDto> commentToDto(const Comment& c) {
    auto dto = CommentDto::createShared();
    dto->id = c.getId();
    dto->authorId = c.getAuthor().c_str();
    dto->text = c.getText().c_str();
    dto->timestamp = c.getTimeStamp();
    return dto;
  }

  static oatpp::Object<UserDto> userToDto(const User& u) {
    auto dto = UserDto::createShared();
    dto->name = u.getName().c_str();
    dto->role = u.getRole().c_str();
    return dto;
  }

  static oatpp::Object<MilestoneDto> milestoneToDto(const Milestone& m) {
  auto dto = MilestoneDto::createShared();
  dto->id = m.getId();
  dto->name = m.getName().c_str();
  dto->description = m.getDescription().c_str();
  dto->startDate = m.getStartDate().c_str();
  dto->endDate = m.getEndDate().c_str();
  return dto;
}

static std::string toLower(const std::string& s) {
  std::string out = s;
  std::transform(out.begin(), out.end(), out.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return out;
}

  // ---- Issue endpoints ----

  ENDPOINT("POST", "/issues", createIssue,
           BODY_DTO(oatpp::Object<IssueCreateDto>, body)) {
    if (!body || !body->title || !body->authorId) {
      return createResponse(Status::CODE_400, "Missing fields");
    }
    Issue i = issues().createIssue(asStdString(body->title),
                                   asStdString(body->description),
                                   asStdString(body->authorId));
    return createDtoResponse(Status::CODE_201, issueToDto(i));
  }

  ENDPOINT("GET", "/issues", listIssues) {
    auto issueList = issues().listAllIssues();
    auto list = oatpp::List<oatpp::Object<IssueDto>>::createShared();
    for (auto& i : issueList) {
      list->push_back(issueToDto(i));
    }
    return createDtoResponse(Status::CODE_200, list);
  }

  ENDPOINT("GET", "/issues/{id}", getIssue, PATH(oatpp::Int32, id)) {
    try {
      Issue i = issues().getIssue(id);
      return createDtoResponse(Status::CODE_200, issueToDto(i));
    } catch (...) {
      return createResponse(Status::CODE_404, "Not found");
    }
  }

  ENDPOINT("PATCH", "/issues/{id}", updateIssue, PATH(oatpp::Int32, id),
           BODY_DTO(oatpp::Object<IssueUpdateFieldDto>, body)) {
    bool ok = issues().updateIssueField(id, asStdString(body->field),
                                        asStdString(body->value));
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_400, "Failed");
  }

  ENDPOINT("DELETE", "/issues/{id}", deleteIssue, PATH(oatpp::Int32, id)) {
    bool ok = issues().deleteIssue(id);
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_404, "Not found");
  }

  // ---- Comment endpoints ----

  ENDPOINT("POST", "/issues/{id}/comments", addComment, PATH(oatpp::Int32, id),
           BODY_DTO(oatpp::Object<CommentCreateDto>, body)) {
    Comment c = issues().addCommentToIssue(id, asStdString(body->text),
                                           asStdString(body->authorId));
    return createDtoResponse(Status::CODE_201, commentToDto(c));
  }

  ENDPOINT("GET", "/issues/{id}/comments", listComments,
           PATH(oatpp::Int32, id)) {
    auto comments = issues().getAllComments(id);
    auto list = oatpp::List<oatpp::Object<CommentDto>>::createShared();
    for (auto& c : comments) {
      list->push_back(commentToDto(c));
    }
    return createDtoResponse(Status::CODE_200, list);
  }

  ENDPOINT("PATCH", "/issues/{issueId}/comments/{commentId}", updateComment,
           PATH(oatpp::Int32, issueId), PATH(oatpp::Int32, commentId),
           BODY_DTO(oatpp::Object<CommentUpdateDto>, body)) {
    bool ok =
        issues().updateComment(issueId, commentId, asStdString(body->text));
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_404, "Not found");
  }

  ENDPOINT("DELETE", "/issues/{issueId}/comments/{commentId}", deleteComment,
           PATH(oatpp::Int32, issueId), PATH(oatpp::Int32, commentId)) {
    bool ok = issues().deleteComment(issueId, commentId);
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_404, "Not found");
  }

  // ---- User endpoints ----

  ENDPOINT("POST", "/users", createUser,
           BODY_DTO(oatpp::Object<UserCreateDto>, body)) {
    User u =
        issues().createUser(asStdString(body->name), asStdString(body->role));
    return createDtoResponse(Status::CODE_201, userToDto(u));
  }

  ENDPOINT("GET", "/users", listUsers) {
    auto users = issues().listAllUsers();
    auto list = oatpp::List<oatpp::Object<UserDto>>::createShared();
    for (auto& u : users) {
      list->push_back(userToDto(u));
    }
    return createDtoResponse(Status::CODE_200, list);
  }

  ENDPOINT("PATCH", "/users/{id}", updateUser, PATH(oatpp::String, id),
           BODY_DTO(oatpp::Object<UserUpdateDto>, body)) {
    bool ok = issues().updateUser(asStdString(id), asStdString(body->field),
                                  asStdString(body->value));
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_400, "Failed");
  }

  ENDPOINT("DELETE", "/users/{id}", deleteUser, PATH(oatpp::String, id)) {
    bool ok = issues().removeUser(asStdString(id));
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_404, "Not found");
  }

ENDPOINT("GET", "/users/{id}/issues", listIssuesByUser,
         PATH(oatpp::String, id)) {
  // case-insensitive lookup of the user
  std::string input = toLower(asStdString(id));
  std::string realId;
  bool found = false;

  for (const auto& user : issues().listAllUsers()) {
    if (toLower(user.getName()) == input) {
      realId = user.getName();
      found = true;
      break;
    }
  }

  if (!found) {
    return createResponse(Status::CODE_404, "User not found");
  }

  // get issues for that user
  auto userIssues = issues().findIssuesByUserId(realId);
  auto list = oatpp::List<oatpp::Object<IssueDto>>::createShared();
  for (auto& issue : userIssues) {
    list->push_back(issueToDto(issue));
  }

  return createDtoResponse(Status::CODE_200, list);
}


  // ---- Tag endpoints ----

  ENDPOINT("POST", "/issues/{id}/tags", addTag, PATH(oatpp::Int32, id),
           BODY_DTO(oatpp::Object<TagDto>, body)) {
    if (!body || !body->tag) {
      return createResponse(Status::CODE_400, "Missing tag");
    }

    bool ok = issues().addTagToIssue(id, asStdString(body->tag));

    return ok ? createResponse(Status::CODE_201, "Tag added")
              : createResponse(Status::CODE_400, "Failed");
  }

  ENDPOINT("DELETE", "/issues/{id}/tags", removeTag, PATH(oatpp::Int32, id),
           BODY_DTO(oatpp::Object<TagDto>, body)) {
    if (!body || !body->tag) {
      return createResponse(Status::CODE_400, "Missing tag");
    }

    bool ok = issues().removeTagFromIssue(id, asStdString(body->tag));

    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_404, "Not found");
  }
 
  ENDPOINT("GET", "/issues/{id}/tags", listTags, PATH(oatpp::Int32, id)) {
    try {
      Issue issue = issues().getIssue(id);

      auto list = oatpp::List<oatpp::String>::createShared();
      for (const auto& tag : issue.getTags()) {
        list->push_back(tag.c_str());
      }

      return createDtoResponse(Status::CODE_200, list);
    } catch (...) {
      return createResponse(Status::CODE_404, "Issue not found");
    }
  }

  // ---- Milestone endpoints ----

ENDPOINT("POST", "/milestones", createMilestone,
         BODY_DTO(oatpp::Object<MilestoneDto>, body)) {

  if (!body || !body->name) {
    return createResponse(Status::CODE_400, "Missing name");
  }

  Milestone m = issues().createMilestone(
      asStdString(body->name),
      asStdString(body->description),
      asStdString(body->startDate),
      asStdString(body->endDate));

  return createDtoResponse(Status::CODE_201, milestoneToDto(m));
}

ENDPOINT("GET", "/milestones", listMilestones) {
  auto list = issues().listAllMilestones();

  auto dtoList = oatpp::List<oatpp::Object<MilestoneDto>>::createShared();
  for (auto& m : list) {
    dtoList->push_back(milestoneToDto(m));
  }

  return createDtoResponse(Status::CODE_200, dtoList);
}

ENDPOINT("GET", "/milestones/{id}", getMilestone,
         PATH(oatpp::Int32, id)) {
  auto m = issues().getMilestone(id);
  return createDtoResponse(Status::CODE_200, milestoneToDto(m));
}

ENDPOINT("DELETE", "/milestones/{id}", deleteMilestone,
         PATH(oatpp::Int32, id),
         QUERY(oatpp::Boolean, cascade)) {
  bool ok = issues().deleteMilestone(id, cascade);
  return createResponse(Status::CODE_200, ok ? "Deleted" : "Failed");
}

ENDPOINT("POST", "/milestones/{id}/issues/{issueId}", addIssueToMilestone,
         PATH(oatpp::Int32, id),
         PATH(oatpp::Int32, issueId)) {
  bool ok = issues().addIssueToMilestone(id, issueId);
  return createResponse(Status::CODE_200, ok ? "Linked" : "Failed");
}

ENDPOINT("DELETE", "/milestones/{id}/issues/{issueId}", removeIssueFromMilestone,
         PATH(oatpp::Int32, id),
         PATH(oatpp::Int32, issueId)) {
  bool ok = issues().removeIssueFromMilestone(id, issueId);
  return createResponse(Status::CODE_200, ok ? "Unlinked" : "Failed");
}

ENDPOINT("GET", "/milestones/{id}/issues", getMilestoneIssues,
         PATH(oatpp::Int32, id)) {

  auto list = issues().getIssuesForMilestone(id);

  auto dtoList = oatpp::Vector<oatpp::Object<IssueDto>>::createShared();
  for (auto& i : list) {
    dtoList->push_back(issueToDto(i));
  }

  return createDtoResponse(Status::CODE_200, dtoList);
}

  // ---- Database endpoints ----

  ENDPOINT("GET", "/databases", listDatabases) {
    auto databases = dbService->listDatabases();
    std::string active = dbService->getActiveDatabaseName();

    auto list = oatpp::List<oatpp::Object<DatabaseDto>>::createShared();
    bool activeIncluded = false;

    for (const auto& name : databases) {
      auto dto = DatabaseDto::createShared();
      dto->name = name.c_str();
      dto->active = (name == active);
      activeIncluded = activeIncluded || dto->active;
      list->push_back(dto);
    }

    if (!activeIncluded && !active.empty()) {
      auto dto = DatabaseDto::createShared();
      dto->name = active.c_str();
      dto->active = true;
      list->push_back(dto);
    }

    return createDtoResponse(Status::CODE_200, list);
  }

  ENDPOINT("POST", "/databases", createDatabase,
           BODY_DTO(oatpp::Object<DatabaseCreateDto>, body)) {
    if (!body || !body->name) {
      return createResponse(Status::CODE_400, "Database name is required");
    }
    std::string provided = asStdString(body->name);
    std::string normalized = withDbExtension(provided);
    auto existing = dbService->listDatabases();
    bool alreadyExists = std::find(existing.begin(), existing.end(),
                                   normalized) != existing.end();

    bool created = dbService->createDatabase(provided);
    if (created) {
      auto dto = DatabaseDto::createShared();
      dto->name = normalized.c_str();
      dto->active = normalized == dbService->getActiveDatabaseName();
      return createDtoResponse(Status::CODE_201, dto);
    }

    if (alreadyExists) {
      return createResponse(Status::CODE_409, "Database already exists");
    }
    return createResponse(Status::CODE_400, "Unable to create database");
  }

  ENDPOINT("DELETE", "/databases/{name}", deleteDatabase,
           PATH(oatpp::String, name)) {
    std::string provided = asStdString(name);
    std::string normalized = withDbExtension(provided);

    auto existing = dbService->listDatabases();
    std::string active = dbService->getActiveDatabaseName();
    bool exists = std::find(existing.begin(), existing.end(), normalized) !=
                  existing.end();

    if (!exists) {
      return createResponse(Status::CODE_404, "Database not found");
    }
    if (normalized == active) {
      return createResponse(Status::CODE_409,
                            "Cannot delete the active database");
    }

    bool deleted = dbService->deleteDatabase(provided);
    return deleted
               ? createResponse(Status::CODE_204, "")
               : createResponse(Status::CODE_400, "Unable to delete database");
  }

  ENDPOINT("POST", "/databases/{name}/switch", switchDatabase,
           PATH(oatpp::String, name)) {
    std::string provided = asStdString(name);
    bool ok = dbService->switchDatabase(provided);
    if (!ok) {
      return createResponse(Status::CODE_404, "Database not found");
    }

    auto dto = DatabaseDto::createShared();
    dto->name = dbService->getActiveDatabaseName().c_str();
    dto->active = true;
    return createDtoResponse(Status::CODE_200, dto);
  }
};

#include OATPP_CODEGEN_END(ApiController)

#endif
