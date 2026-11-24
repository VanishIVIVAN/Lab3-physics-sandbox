#include <spdlog/spdlog.h>

#include <cstdlib>
#include <exception>

#include "sandbox/Application.h"
#include "sandbox/Logging.h"

int main() {
  sandbox::initLogging();
  try {
    sandbox::Application application;
    return application.run();
  } catch (const std::exception& error) {
    spdlog::critical("Fatal error: {}", error.what());
    spdlog::shutdown();
    return EXIT_FAILURE;
  }
}
