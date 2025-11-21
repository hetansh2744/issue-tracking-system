#ifndef ISSUE_DTO_HPP_
#define ISSUE_DTO_HPP_

#include "oatpp/core/Types.hpp"
#include "oatpp/core/mapping/type/Object.hpp"

class IssueDto : public oatpp::DTO {
  DTO_INIT(IssueDto, DTO)

  DTO_FIELD(oatpp::Int32, id);
  DTO_FIELD(oatpp::String, authorId, "author_id");
  DTO_FIELD(oatpp::String, title);
  DTO_FIELD(oatpp::String, description);
  DTO_FIELD(oatpp::String, assignedTo, "assigned_to");
  DTO_FIELD(oatpp::List<oatpp::Int32>, commentIds, "comment_ids");
  DTO_FIELD(oatpp::Int64, createdAt, "created_at");
};

class IssueCreateDto : public oatpp::DTO {
  DTO_INIT(IssueCreateDto, DTO)

  DTO_FIELD(oatpp::String, title);
  DTO_FIELD(oatpp::String, description);
  DTO_FIELD(oatpp::String, authorId, "author_id");
};

class IssueUpdateFieldDto : public oatpp::DTO {
  DTO_INIT(IssueUpdateFieldDto, DTO)

  DTO_FIELD(oatpp::String, field);
  DTO_FIELD(oatpp::String, value);
};

#endif
