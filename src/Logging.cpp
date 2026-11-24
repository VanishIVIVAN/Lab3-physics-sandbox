#include "sandbox/Logging.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>
#include <vector>

namespace sandbox {

void initLogging() {
  std::vector<spdlog::sink_ptr> sinks;
  sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

  std::string fileError;
  try {
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("sandbox.log", true));
  } catch (const spdlog::spdlog_ex& error) {
    fileError = error.what();  // reported below, once the logger exists
  }

  auto logger = std::make_shared<spdlog::logger>("sandbox", sinks.begin(), sinks.end());
  logger->set_pattern("[%H:%M:%S.%e] [%^%l%^] %v");
  logger->set_level(spdlog::level::info);
  logger->flush_on(spdlog::level::info);
  spdlog::set_default_logger(logger);

  if (!fileError.empty()) spdlog::warn("Cannot open sandbox.log, logging to console only: {}", fileError);
}

}  // namespace sandbox
