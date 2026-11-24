#pragma once

namespace sandbox {

/// Sets up spdlog: coloured console output plus the file "sandbox.log".
/// Falls back to console only if the file cannot be opened.
void initLogging();

}  // namespace sandbox
