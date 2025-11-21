#ifndef ISSUE_API_CONTROLLER_HPP_
#define ISSUE_API_CONTROLLER_HPP_

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include "dtos/IssueDto.hpp"
#include "dtos/CommentDto.hpp"
#include "dtos/UserDto.hpp"
#include "service/IssueService.hpp"

#include "Issue.hpp"
#include "Comment.hpp"
#include "User.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class IssueApiController : public oatpp::web::server::api::ApiController {
 private:
  std::shared_ptr<IssueService> service;

 public:
  IssueApiController(
      const std::shared_ptr<oatpp::data::mapping::ObjectMapper>& objectMapper)
      : oatpp::web::server::api::ApiController(objectMapper),
        service(std::make_shared<IssueService>()) {}

  // Convert helpers
  static oatpp::Object<IssueDto> issueToDto(const Issue& i) {
    auto dto = IssueDto::createShared();
    dto->id = i.getId();
    dto->authorId = i.getAuthorId().c_str();
    dto->title = i.getTitle().c_str();
    dto->description = "";
    dto->assignedTo =
        i.hasAssignee() ? i.getAssignedTo().c_str() : "";
    auto ids = oatpp::List<oatpp::Int32>::createShared();
    for (int cid : i.getCommentIds()) {
      ids->pushBack(cid);
    }
    dto->commentIds = ids;
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

  // ---- Issue endpoints ----

  ENDPOINT("POST", "/issues", createIssue,
           BODY_DTO(oatpp::Object<IssueCreateDto>, body)) {
    if (!body || !body->title || !body->authorId) {
      return createResponse(Status::CODE_400, "Missing fields");
    }
    Issue i = service->createIssue(
        body->title->std_str(),
        body->description ? body->description->std_str() : "",
        body->authorId->std_str());
    return createDtoResponse(Status::CODE_201, issueToDto(i));
  }

  ENDPOINT("GET", "/issues", listIssues) {
    auto issues = service->listAllIssues();
    auto list = oatpp::List<oatpp::Object<IssueDto>>::createShared();
    for (auto& i : issues) {
      list->pushBack(issueToDto(i));
    }
    return createDtoResponse(Status::CODE_200, list);
  }

  ENDPOINT("GET", "/issues/{id}", getIssue,
           PATH(oatpp::Int32, id)) {
    try {
      Issue i = service->getIssue(id);
      return createDtoResponse(Status::CODE_200, issueToDto(i));
    } catch (...) {
      return createResponse(Status::CODE_404, "Not found");
    }
  }

  ENDPOINT("PATCH", "/issues/{id}", updateIssue,
           PATH(oatpp::Int32, id),
           BODY_DTO(oatpp::Object<IssueUpdateFieldDto>, body)) {
    bool ok = service->updateIssueField(
        id, body->field->std_str(), body->value->std_str());
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_400, "Failed");
  }

  ENDPOINT("DELETE", "/issues/{id}", deleteIssue,
           PATH(oatpp::Int32, id)) {
    bool ok = service->deleteIssue(id);
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_404, "Not found");
  }

  // ---- Comment endpoints ----

  ENDPOINT("POST", "/issues/{id}/comments", addComment,
           PATH(oatpp::Int32, id),
           BODY_DTO(oatpp::Object<CommentCreateDto>, body)) {
    Comment c = service->addCommentToIssue(
        id, body->text->std_str(), body->authorId->std_str());
    return createDtoResponse(Status::CODE_201, commentToDto(c));
  }

  ENDPOINT("GET", "/issues/{id}/comments", listComments,
           PATH(oatpp::Int32, id)) {
    auto comments = service->getAllComments(id);
    auto list = oatpp::List<oatpp::Object<CommentDto>>::createShared();
    for (auto& c : comments) {
      list->pushBack(commentToDto(c));
    }
    return createDtoResponse(Status::CODE_200, list);
  }

  ENDPOINT("PATCH", "/issues/{issueId}/comments/{commentId}",
           updateComment,
           PATH(oatpp::Int32, issueId),
           PATH(oatpp::Int32, commentId),
           BODY_DTO(oatpp::Object<CommentUpdateDto>, body)) {
    bool ok = service->updateComment(
        issueId, commentId, body->text->std_str());
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_404, "Not found");
  }

  ENDPOINT("DELETE", "/issues/{issueId}/comments/{commentId}",
           deleteComment,
           PATH(oatpp::Int32, issueId),
           PATH(oatpp::Int32, commentId)) {
    bool ok = service->deleteComment(issueId, commentId);
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_404, "Not found");
  }

  // ---- User endpoints ----

  ENDPOINT("POST", "/users", createUser,
           BODY_DTO(oatpp::Object<UserCreateDto>, body)) {
    User u = service->createUser(
        body->name->std_str(), body->role->std_str());
    return createDtoResponse(Status::CODE_201, userToDto(u));
  }

  ENDPOINT("GET", "/users", listUsers) {
    auto users = service->listAllUsers();
    auto list = oatpp::List<oatpp::Object<UserDto>>::createShared();
    for (auto& u : users) {
      list->pushBack(userToDto(u));
    }
    return createDtoResponse(Status::CODE_200, list);
  }

  ENDPOINT("PATCH", "/users/{id}", updateUser,
           PATH(oatpp::String, id),
           BODY_DTO(oatpp::Object<UserUpdateDto>, body)) {
    bool ok = service->updateUser(
        id->std_str(), body->field->std_str(), body->value->std_str());
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_400, "Failed");
  }

  ENDPOINT("DELETE", "/users/{id}", deleteUser,
           PATH(oatpp::String, id)) {
    bool ok = service->removeUser(id->std_str());
    return ok ? createResponse(Status::CODE_204, "")
              : createResponse(Status::CODE_404, "Not found");
  }

};

#include OATPP_CODEGEN_END(ApiController)

#endif
