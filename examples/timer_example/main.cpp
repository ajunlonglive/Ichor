#include "UsingTimerService.h"
#include <optional_bundles/logging_bundle/SpdlogFrameworkLogger.h>
#include <optional_bundles/logging_bundle/LoggerAdmin.h>
#include <optional_bundles/logging_bundle/SpdlogLogger.h>
#include <chrono>
#include <iostream>

using namespace std::string_literals;

int main() {
    std::locale::global(std::locale("en_US.UTF-8"));

    auto start = std::chrono::system_clock::now();
    DependencyManager dm{};
    auto logMgr = dm.createServiceManager<IFrameworkLogger, SpdlogFrameworkLogger>();
    auto logAdminMgr = dm.createDependencyServiceManager<ILoggerAdmin, LoggerAdmin<SpdlogLogger>>(RequiredList<IFrameworkLogger>, OptionalList<>);
    auto testOneMgr = dm.createDependencyServiceManager<IUsingTimerService, UsingTimerService>(RequiredList<ILogger>, OptionalList<>);
    dm.start();
    auto end = std::chrono::system_clock::now();
    std::cout << fmt::format("Program ran for {:n} µs\n", std::chrono::duration_cast<std::chrono::microseconds>(end-start).count());

    return 0;
}