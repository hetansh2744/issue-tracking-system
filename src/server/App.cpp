#include "AppComponent.hpp"
#include "Runner.hpp"

#include <oatpp/network/tcp/server/ConnectionProvider.hpp>

void run() {

  // Register Oat++ components
  AppComponent components;

  // Create server connection provider (localhost:8100)
  auto connectionProvider =
      oatpp::network::tcp::server::ConnectionProvider::createShared(
          {"localhost", 8100, oatpp::network::Address::IP_4});

  // Create runner with provider
  Runner runner(connectionProvider);

  // Run server
  runner.run();
}

int main(int argc, const char* argv[]) {
  oatpp::base::Environment::init();
  run();
  oatpp::base::Environment::destroy();
  return 0;
}
