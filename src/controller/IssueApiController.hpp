#ifndef ISSUE_API_CONTROLLER_HPP_
#define ISSUE_API_CONTROLLER_HPP_

#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <string>

#include "Comment.hpp"
#include "CommentDto.hpp"
#include "DatabaseDto.hpp"
#include "Issue.hpp"
#include "IssueDto.hpp"
#include "Milestone.hpp"
#include "MilestoneDto.hpp"
#include "TagDto.hpp"
#include "ErrorDto.hpp"
#include "User.hpp"
#include "UserDto.hpp"
#include "UserRoles.hpp"

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
  static std::optional<std::string> asOptionalStdString(
      const oatpp::String& value) {
    if (!value) {
      return std::nullopt;
    }
    return std::optional<std::string>(*value);
  }
  IssueService& issues() const { return dbService->getIssueService(); }
  static std::string withDbExtension(const std::string& name) {
    if (name.size() >= 3 && name.substr(name.size() - 3) == ".db") {
      return name;
    }
    return name + ".db";
  }

  static std::string toLower(const std::string& str) {
    std::string out = str;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return
                    static_cast<char>(std::tolower(c)); });
    return out;
  }

  static std::string normalizeStatusKey(const std::string& raw) {
    std::string lowered = toLower(raw);
    std::string key;
    key.reserve(lowered.size());

    for (char ch : lowered) {
      unsigned char c = static_cast<unsigned char>(ch);
      if (!std::isspace(c) && ch != '-' && ch != '_') {
        key.push_back(static_cast<char>(c));
      }
    }
    return key;
  }

  static std::string canonicalStatusLabel(const std::string& raw) {
    const std::string key = normalizeStatusKey(raw);

    if (key == "1" || key == "tobedone") {
      return "To Be Done";
    }
    if (key == "2" || key == "inprogress") {
      return "In Progress";
    }
    if (key == "3" || key == "done") {
      return "Done";
    }

    return raw;
  }

  std::shared_ptr<OutgoingResponse> error(
      const Status& status,
      const std::string& code,
      const std::string& message) {
    auto dto = ErrorDto::createShared();
    dto->statusCode = status.code;
    dto->error = code.c_str();
    dto->message = message.c_str();
    return createDtoResponse(status, dto);
  }

 public:
  IssueApiController(
      const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& objectMapper)
      : oatpp::web::server::api::ApiController(objectMapper),
        dbService(std::make_shared<DatabaseService>()) {}

  static oatpp::Object<IssueDto> issueToDto(const Issue& i) {
    auto dto = IssueDto::createShared();
    dto->id = i.getId();
    dto->authorId = i.getAuthorId().c_str();
    dto->title = i.getTitle().c_str();

    dto->description = i.hasDescriptionComment() ? i.getDescriptionComment().c_str() : "";

    dto->assignedTo =
        i.hasAssignee() ? i.getAssignedTo().c_str() : "";

    dto->status = i.getStatus().c_str();

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
    auto issueIds = oatpp::List<oatpp::Int32>::createShared();
    for (int id : m.getIssueIds()) {
      issueIds->push_back(id);
    }
    dto->issueIds = issueIds;
    return dto;
  }

  // ---- Issue endpoints ----
  ENDPOINT_INFO(createIssue) {
    info->summary = "Create a new issue";
    info->addConsumes<Object<IssueCreateDto>>("application/json");
    info->addResponse<Object<IssueDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_400, "application/json",
                            "Missing required fields: title, authorId");
  }
  ENDPOINT("POST", "/issues", createIssue,
           BODY_DTO(oatpp::Object<IssueCreateDto>, body)) {
    if (!body || !body->title || !body->authorId) {
      return error(Status::CODE_400,
                   "MISSING_FIELDS",
                   "title and authorId are required");
    }

    Issue i = issues().createIssue(
        asStdString(body->title),
        asStdString(body->description),
        asStdString(body->authorId));
    if (!i.hasPersistentId()) {
      return error(Status::CODE_400,
                   "INVALID_AUTHOR",
                   "Author does not exist");
    }
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

  ENDPOINT("GET", "/issues/unassigned", listUnassignedIssues) {
    auto issueList = issues().listAllUnassignedIssues(); // Assuming issues() is IssueService
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
      return error(Status::CODE_404,
                   "ISSUE_NOT_FOUND",
                   "Issue not found");
    }
  }

  ENDPOINT("PATCH", "/issues/{id}", updateIssue, PATH(oatpp::Int32, id),
           BODY_DTO(oatpp::Object<IssueUpdateFieldDto>, body)) {
    bool ok = issues().updateIssueField(id, asStdString(body->field),
                                        asStdString(body->value));
    return ok ? createResponse(Status::CODE_204, "")
              : error(Status::CODE_400,
                      "UPDATE_FAILED",
                      "Unable to update issue");
  }

  ENDPOINT_INFO(deleteIssue) {
    info->summary = "Delete an issue by id";
    info->addResponse<String>(Status::CODE_204, "text/plain",
                              "Issue deleted (no content)");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json",
                                        "Issue not found");
  }
  ENDPOINT("DELETE", "/issues/{id}", deleteIssue,
           PATH(oatpp::Int32, id)) {
    bool ok = issues().deleteIssue(id);
    if (ok) {
      return createResponse(Status::CODE_204, "");
    }

    return error(Status::CODE_404,
                 "ISSUE_NOT_FOUND",
                 "Issue not found");
  }

  // ---- Comment endpoints ----

  ENDPOINT("POST", "/issues/{id}/comments", addComment, PATH(oatpp::Int32, id),
           BODY_DTO(oatpp::Object<CommentCreateDto>, body)) {
    if (!body || !body->text || !body->authorId) {
      return error(Status::CODE_400,
                   "MISSING_FIELDS",
                   "text and authorId are required");
    }

    Comment c = issues().addCommentToIssue(
        id,
        asStdString(body->text),
        asStdString(body->authorId));

    if (!c.hasPersistentId()) {
      return error(Status::CODE_404,
                   "COMMENT_NOT_CREATED",
                   "Issue or author not found");
    }
    return createDtoResponse(Status::CODE_201, commentToDto(c));
  }

  ENDPOINT("GET", "/issues/{id}/comments", listComments,
           PATH(oatpp::Int32, id)) {
    try {
      auto comments = issues().getAllComments(id);
      auto list = oatpp::List<oatpp::Object<CommentDto>>::createShared();

      for (auto& c : comments) {
        list->push_back(commentToDto(c));
      }
      return createDtoResponse(Status::CODE_200, list);
    } catch (...) {
      return error(Status::CODE_404,
                   "ISSUE_NOT_FOUND",
                   "Issue not found");
    }
  }

  ENDPOINT("PATCH", "/issues/{issueId}/comments/{commentId}", updateComment,
           PATH(oatpp::Int32, issueId), PATH(oatpp::Int32, commentId),
           BODY_DTO(oatpp::Object<CommentUpdateDto>, body)) {
    if (!body || !body->text) {
      return error(Status::CODE_400,
                   "MISSING_FIELDS",
                   "text is required");
    }

    bool ok = issues().updateComment(
        issueId,
        commentId,
        asStdString(body->text));

    return ok ? createResponse(Status::CODE_204, "")
              : error(Status::CODE_404,
                      "COMMENT_NOT_FOUND",
                      "Comment not found");
  }

  ENDPOINT("DELETE", "/issues/{issueId}/comments/{commentId}", deleteComment,
           PATH(oatpp::Int32, issueId), PATH(oatpp::Int32, commentId)) {
    bool ok = issues().deleteComment(issueId, commentId);
    return ok ? createResponse(Status::CODE_204, "")
              : error(Status::CODE_404,
                      "COMMENT_NOT_FOUND",
                      "Comment not found");
  }

  // ---- User endpoints ----

  ENDPOINT("POST", "/users", createUser,
           BODY_DTO(oatpp::Object<UserCreateDto>, body)) {
    if (!body || !body->name || !body->role) {
      return error(Status::CODE_400,
                   "MISSING_FIELDS",
                   "name and role are required");
    }
    const std::string name = asStdString(body->name);
    const std::string role = asStdString(body->role);
    if (name.empty() || !user_roles::isValidRole(role)) {
      return error(Status::CODE_400,
                   "INVALID_USER",
                   "Invalid name or role");
    }

    User u = issues().createUser(
        name,
        role);

    return createDtoResponse(Status::CODE_201, userToDto(u));
  }

  ENDPOINT("GET", "/users", listUsers) {
    auto usersList = issues().listAllUsers();
    auto list = oatpp::List<oatpp::Object<UserDto>>::createShared();
    for (auto& u : usersList) {
      list->push_back(userToDto(u));
    }
    return createDtoResponse(Status::CODE_200, list);
  }

  ENDPOINT("PATCH", "/users/{id}", updateUser, PATH(oatpp::String, id),
           BODY_DTO(oatpp::Object<UserUpdateDto>, body)) {
    bool ok = issues().updateUser(asStdString(id), asStdString(body->field),
                                  asStdString(body->value));
    return ok ? createResponse(Status::CODE_204, "")
              : error(Status::CODE_400,
                      "UPDATE_FAILED",
                      "Unable to update user");
  }

  ENDPOINT("DELETE", "/users/{id}", deleteUser, PATH(oatpp::String, id)) {
    bool ok = issues().removeUser(asStdString(id));
    return ok ? createResponse(Status::CODE_204, "")
              : error(Status::CODE_404,
                      "USER_NOT_FOUND",
                      "User not found");
  }

  ENDPOINT("GET", "/users/{id}/issues", listIssuesByUser,
           PATH(oatpp::String, id)) {
    std::string input = toLower(asStdString(id));
    std::string realId;
    bool found = false;

    auto allUsers = issues().listAllUsers();
    for (const auto& user : allUsers) {
      if (toLower(user.getName()) == input) {
        realId = user.getName();
        found = true;
        break;
      }
    }

    if (!found) {
      return error(Status::CODE_404,
                   "USER_NOT_FOUND",
                   "User not found");
    }

    auto userIssues = issues().findIssuesByUserId(realId);
    auto list = oatpp::List<oatpp::Object<IssueDto>>::createShared();
    for (auto& issue : userIssues) {
      list->push_back(issueToDto(issue));
    }

    return createDtoResponse(Status::CODE_200, list);
  }
  ENDPOINT("POST", "/users/{id}/issues", assignUserToIssue,
         PATH(oatpp::String, id),
         BODY_DTO(oatpp::Object<AssignIssueDto>, body)) {

  if (!body || !body->issueId) {
    return error(Status::CODE_400,
                 "MISSING_ISSUE_ID",
                 "issueId is required");
  }

  std::string inputUser = toLower(asStdString(id));
  std::string realUser;
  bool found = false;

  for (const auto& user : issues().listAllUsers()) {
    if (toLower(user.getName()) == inputUser) {
      realUser = user.getName();
      found = true;
      break;
    }
  }

  if (!found) {
    return error(Status::CODE_404,
                 "USER_NOT_FOUND",
                 "User not found");
  }

  bool ok = issues().assignUserToIssue(body->issueId, realUser);
  if (!ok) {
    return error(Status::CODE_404,
                 "ISSUE_NOT_FOUND",
                 "Issue not found or assignment failed");
  }

  try {
    Issue updated = issues().getIssue(body->issueId);
    return createDtoResponse(Status::CODE_200, issueToDto(updated));
  } catch (...) {
    return error(Status::CODE_404,
                 "ISSUE_NOT_FOUND",
                 "Issue not found after assignment");
  }
}


  // ---- Tag endpoints ----

  ENDPOINT("POST", "/issues/{id}/tags", addTag, PATH(oatpp::Int32, id),
           BODY_DTO(oatpp::Object<TagDto>, body)) {
    if (!body || !body->tag) {
      return error(Status::CODE_400,
                   "MISSING_TAG",
                   "Missing tag");
    }

    const std::string tag = asStdString(body->tag);
    if (tag.empty()) {
      return error(Status::CODE_400,
                   "MISSING_TAG",
                   "Missing tag");
    }

    bool ok = issues().addTagToIssue(id, tag);

    return ok ? createResponse(Status::CODE_201, "Tag added")
              : error(Status::CODE_400,
                      "TAG_ADD_FAILED",
                      "Failed to add tag");
  }

  ENDPOINT("DELETE", "/issues/{id}/tags", removeTag, PATH(oatpp::Int32, id),
           BODY_DTO(oatpp::Object<TagDto>, body)) {
    if (!body || !body->tag) {
      return error(Status::CODE_400,
                   "MISSING_TAG",
                   "Missing tag");
    }

    const std::string tag = asStdString(body->tag);
    if (tag.empty()) {
      return error(Status::CODE_400,
                   "MISSING_TAG",
                   "Missing tag");
    }

    bool ok = issues().removeTagFromIssue(id, tag);

    return ok ? createResponse(Status::CODE_204, "")
              : error(Status::CODE_404,
                      "TAG_NOT_FOUND",
                      "Tag not found on issue");
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
      return error(Status::CODE_404,
                   "ISSUE_NOT_FOUND",
                   "Issue not found");
    }
  }

  ENDPOINT("GET", "/issues/tags/{tag}", getIssuesByTag,
           PATH(oatpp::String, tag)) {
    auto list = oatpp::List<oatpp::Object<IssueDto>>::createShared();

    if (!tag) {
      return createDtoResponse(Status::CODE_200, list);
    }

    std::string searchTag = asStdString(tag);
    if (searchTag.empty()) {
      return createDtoResponse(Status::CODE_200, list);
    }

    auto allIssues = issues().listAllIssues();
    
    for (const auto& issue : allIssues) {
      if (issue.hasTag(searchTag)) {
        list->push_back(issueToDto(issue));
      }
    }

    return createDtoResponse(Status::CODE_200, list);
  }

  ENDPOINT("GET", "/issues/tags", getIssuesByTags,
           QUERY(oatpp::String, tags)) {
    auto list = oatpp::List<oatpp::Object<IssueDto>>::createShared();

    if (!tags) {
      return createDtoResponse(Status::CODE_200, list);
    }

    std::string tagsStr = asStdString(tags);
    if (tagsStr.empty()) {
      return createDtoResponse(Status::CODE_200, list);
    }

    std::vector<std::string> searchTags;
    std::istringstream iss(tagsStr);
    std::string tag;
    while (std::getline(iss, tag, ',')) {
      tag.erase(0, tag.find_first_not_of(" \t"));
      tag.erase(tag.find_last_not_of(" \t") + 1);
      if (!tag.empty()) {
        searchTags.push_back(tag);
      }
    }

    if (searchTags.empty()) {
      return createDtoResponse(Status::CODE_200, list);
    }

    auto allIssues = issues().listAllIssues();
    
    for (const auto& issue : allIssues) {
      for (const auto& searchTag : searchTags) {
        if (issue.hasTag(searchTag)) {
          list->push_back(issueToDto(issue));
          break;
        }
      }
    }

    return createDtoResponse(Status::CODE_200, list);
  }

  // ---- Milestone endpoints ----

  ENDPOINT("POST", "/milestones", createMilestone,
           BODY_DTO(oatpp::Object<MilestoneCreateDto>, body)) {
    if (!body) {
      return error(Status::CODE_400,
                   "MISSING_PAYLOAD",
                   "Milestone payload is required");
    }

    const std::string name = asStdString(body->name);
    const std::string start = asStdString(body->startDate);
    const std::string end = asStdString(body->endDate);

    if (name.empty() || start.empty() || end.empty()) {
      return error(Status::CODE_400,
                   "MISSING_FIELDS",
                   "name, startDate, and endDate are required");
    }

    const std::string desc = asStdString(body->description);

    try {
      Milestone m = issues().createMilestone(name, desc, start, end);
      return createDtoResponse(Status::CODE_201, milestoneToDto(m));
    } catch (const std::invalid_argument& ex) {
      return error(Status::CODE_400,
                   "INVALID_MILESTONE",
                   ex.what());
    }
  }

  ENDPOINT("GET", "/milestones", listMilestones) {
    auto list = issues().listAllMilestones();

    auto dtoList = oatpp::List<oatpp::Object<MilestoneDto>>::createShared();
    for (auto& m : list) {
      dtoList->push_back(milestoneToDto(m));
    }

    return createDtoResponse(Status::CODE_200, dtoList);
  }

  ENDPOINT("GET", "/milestones/{id}", getMilestone, PATH(oatpp::Int32, id)) {
    try {
      auto m = issues().getMilestone(id);
      return createDtoResponse(Status::CODE_200, milestoneToDto(m));
    } catch (const std::out_of_range&) {
      return error(Status::CODE_404,
                   "MILESTONE_NOT_FOUND",
                   "Milestone not found");
    }
  }

  ENDPOINT("PATCH", "/milestones/{id}", updateMilestone, PATH(oatpp::Int32, id),
           BODY_DTO(oatpp::Object<MilestoneUpdateDto>, body)) {
    if (!body) {
      return error(Status::CODE_400,
                   "MISSING_PAYLOAD",
                   "Milestone payload is required");
    }

    if (!body->name && !body->description &&
        !body->startDate && !body->endDate) {
      return error(Status::CODE_400,
                   "NO_FIELDS",
                   "No fields to update");
    }

    try {
      auto updated =
          issues().updateMilestone(id, asOptionalStdString(body->name),
                                   asOptionalStdString(body->description),
                                   asOptionalStdString(body->startDate),
                                   asOptionalStdString(body->endDate));
      return createDtoResponse(Status::CODE_200, milestoneToDto(updated));
    } catch (const std::out_of_range&) {
      return error(Status::CODE_404,
                   "MILESTONE_NOT_FOUND",
                   "Milestone not found");
    } catch (const std::invalid_argument& ex) {
      return error(Status::CODE_400,
                   "INVALID_MILESTONE",
                   ex.what());
    }
  }

  ENDPOINT("DELETE", "/milestones/{id}", deleteMilestone,
           PATH(oatpp::Int32, id), QUERY(oatpp::Boolean, cascade)) {
    try {
      bool ok = issues().deleteMilestone(id, cascade);
      return createResponse(Status::CODE_200, ok ? "Deleted" : "Failed");
    } catch (const std::out_of_range&) {
      return error(Status::CODE_404,
                   "MILESTONE_NOT_FOUND",
                   "Milestone not found");
    }
  }

  ENDPOINT("POST", "/milestones/{id}/issues/{issueId}", addIssueToMilestone,
           PATH(oatpp::Int32, id), PATH(oatpp::Int32, issueId)) {
    try {
      bool linked = issues().addIssueToMilestone(id, issueId);
      if (!linked) {
        return error(Status::CODE_400,
                     "ISSUE_ALREADY_LINKED",
                     "Issue already linked");
      }
      auto milestone = issues().getMilestone(id);
      return createDtoResponse(Status::CODE_200, milestoneToDto(milestone));
    } catch (const std::out_of_range&) {
      return error(Status::CODE_404,
                   "MILESTONE_NOT_FOUND",
                   "Milestone not found");
    } catch (const std::invalid_argument& ex) {
      return error(Status::CODE_400,
                   "INVALID_MILESTONE",
                   ex.what());
    }
  }

  ENDPOINT("DELETE", "/milestones/{id}/issues/{issueId}",
           removeIssueFromMilestone, PATH(oatpp::Int32, id),
           PATH(oatpp::Int32, issueId)) {
    try {
      bool ok = issues().removeIssueFromMilestone(id, issueId);
      return ok ? createResponse(Status::CODE_204, "")
                : error(Status::CODE_404,
                        "ISSUE_NOT_LINKED",
                        "Issue not linked to milestone");
    } catch (const std::out_of_range&) {
      return error(Status::CODE_404,
                   "MILESTONE_NOT_FOUND",
                   "Milestone not found");
    }
  }

  ENDPOINT("GET", "/milestones/{id}/issues", getMilestoneIssues,
           PATH(oatpp::Int32, id)) {
    try {
      auto list = issues().getIssuesForMilestone(id);

      auto dtoList = oatpp::List<oatpp::Object<IssueDto>>::createShared();
      for (auto& i : list) {
        dtoList->push_back(issueToDto(i));
      }

      return createDtoResponse(Status::CODE_200, dtoList);
    } catch (const std::out_of_range&) {
      return error(Status::CODE_404,
                   "MILESTONE_NOT_FOUND",
                   "Milestone not found");
    }
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
      return error(Status::CODE_400,
                   "MISSING_NAME",
                   "Database name is required");
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
      return error(Status::CODE_409,
                   "DATABASE_EXISTS",
                   "Database already exists");
    }
    return error(Status::CODE_400,
                 "DATABASE_CREATE_FAILED",
                 "Unable to create database");
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
      return error(Status::CODE_404,
                   "DATABASE_NOT_FOUND",
                   "Database not found");
    }
    if (normalized == active) {
      return error(Status::CODE_409,
                   "DATABASE_ACTIVE",
                   "Cannot delete the active database");
    }

    bool deleted = dbService->deleteDatabase(provided);
    return deleted
               ? createResponse(Status::CODE_204, "")
               : error(Status::CODE_400,
                       "DATABASE_DELETE_FAILED",
                       "Unable to delete database");
  }

  ENDPOINT("POST", "/databases/{name}/switch", switchDatabase,
           PATH(oatpp::String, name)) {
    std::string provided = asStdString(name);
    bool ok = dbService->switchDatabase(provided);
    if (!ok) {
      return error(Status::CODE_404,
                   "DATABASE_NOT_FOUND",
                   "Database not found");
    }

    auto dto = DatabaseDto::createShared();
    dto->name = dbService->getActiveDatabaseName().c_str();
    dto->active = true;
    return createDtoResponse(Status::CODE_200, dto);
  }

  // ---- Status endpoints ----

  ENDPOINT("PUT", "/issues/{id}/status", updateIssueStatus,
           PATH(Int32, id),
           BODY_STRING(String, status)) {
    if (!status) {
      return error(Status::CODE_400,
                   "MISSING_STATUS",
                   "Status is required");
    }

    const std::string canonical =
        canonicalStatusLabel(asStdString(status));

    bool ok = issues().updateIssueField(id, "status", canonical);

    return ok ? createResponse(Status::CODE_200, "Status updated")
              : error(Status::CODE_404,
                      "ISSUE_NOT_FOUND",
                      "Issue not found");
  }

  ENDPOINT("GET", "/issues/status/{status}", getIssuesByStatus,
           PATH(oatpp::String, status)) {
    auto list = oatpp::List<oatpp::Object<IssueDto>>::createShared();

    if (!status) {
      return createDtoResponse(Status::CODE_200, list);
    }

    const std::string canonical =
        canonicalStatusLabel(asStdString(status));

    if (canonical != "To Be Done" && canonical !=
      "In Progress" && canonical != "Done") {
      return error(Status::CODE_400,
                   "INVALID_STATUS",
                   "Status must be 'To Be Done', 'In Progress', 'Done', or a valid alias (e.g., '1', '2', 'tobedone').");
    }

    auto all = issues().listAllIssues();
    for (const auto& issue : all) {
      if (canonicalStatusLabel(issue.getStatus()) == canonical) {
        list->push_back(issueToDto(issue));
      }
    }

    return createDtoResponse(Status::CODE_200, list);
  }
};

#include OATPP_CODEGEN_END(ApiController)

#endif  // ISSUE_API_CONTROLLER_HPP_
