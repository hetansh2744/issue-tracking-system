#include "Runner.hpp"

#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/Server.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "controller/IssueApiController.hpp"
#include "SwaggerComponent.hpp"

void Runner::run() {
  auto router = oatpp::web::server::HttpRouter::createShared();

  auto objectMapper =
      oatpp::parser::json::mapping::ObjectMapper::createShared();

  // Register API controller
  auto endpoints = router
      ->addController(IssueApiController::createShared(objectMapper))
      ->getEndpoints();

  // Swagger
  SwaggerComponent swagger;

  router->addController(
      oatpp::swagger::Controller::createShared(
          endpoints,
          swagger.m_documentInfo,
          swagger.m_resources));

  auto handler =
      oatpp::web::server::HttpConnectionHandler::createShared(router);

  oatpp::network::Server server(
      m_tcpConnectionProvider, handler);

  server.run();
}
