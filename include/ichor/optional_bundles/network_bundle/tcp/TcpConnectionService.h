#pragma once

#include <ichor/optional_bundles/network_bundle/IConnectionService.h>
#include <ichor/optional_bundles/logging_bundle/Logger.h>
#include <ichor/optional_bundles/timer_bundle/TimerService.h>

namespace Ichor {
    class TcpConnectionService final : public IConnectionService, public Service<TcpConnectionService> {
    public:
        TcpConnectionService(DependencyRegister &reg, Properties props, DependencyManager *mng);
        ~TcpConnectionService() final = default;

        StartBehaviour start() final;
        StartBehaviour stop() final;

        void addDependencyInstance(ILogger *logger, IService *isvc);
        void removeDependencyInstance(ILogger *logger, IService *isvc);

        uint64_t sendAsync(std::vector<uint8_t>&& msg) final;
        void setPriority(uint64_t priority) final;
        uint64_t getPriority() final;

    private:
        int _socket;
        int _attempts;
        uint64_t _priority;
        uint64_t _msgIdCounter;
        bool _quit;
        ILogger *_logger{nullptr};
        Timer* _timerManager{nullptr};
    };
}