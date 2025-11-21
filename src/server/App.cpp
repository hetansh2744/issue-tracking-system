#include "AppComponent.hpp"
#include "Runner.hpp"

#include "oatpp/core/base/CommandLineArguments.hpp"
#include "oatpp/core/base/Environment.hpp"

int main(int argc, const char* argv[]) {
  oatpp::Environment::init();

  AppComponent components;

  OATPP_COMPONENT(std::shared_ptr<
    oatpp::network::tcp::server::ConnectionProvider>,
    connectionProvider);

  Runner runner(connectionProvider);
  runner.run();

  oatpp::Environment::destroy();
  return 0;
}
