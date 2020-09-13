#pragma once

#include <cppelix/DependencyManager.h>
#include <cppelix/optional_bundles/logging_bundle/Logger.h>
#include <cppelix/optional_bundles/timer_bundle/TimerService.h>
#include <cppelix/optional_bundles/network_bundle/NetworkDataEvent.h>
#include <cppelix/optional_bundles/network_bundle/IConnectionService.h>
#include <cppelix/optional_bundles/network_bundle/IHostService.h>
#include <cppelix/Service.h>
#include <cppelix/LifecycleManager.h>
#include <cppelix/optional_bundles/serialization_bundle/SerializationAdmin.h>
#include "TestMsg.h"

using namespace Cppelix;


struct IUsingWsService : virtual public IService {
    static constexpr InterfaceVersion version = InterfaceVersion{1, 0, 0};
};

class UsingWsService final : public IUsingWsService, public Service {
public:
    UsingWsService(DependencyRegister &reg, CppelixProperties props) : Service(std::move(props)) {
        reg.registerDependency<ILogger>(this, true);
        reg.registerDependency<ISerializationAdmin>(this, true);
        reg.registerDependency<IConnectionService>(this, true, *getProperties());
    }
    ~UsingWsService() final = default;

    bool start() final {
        LOG_INFO(_logger, "UsingWsService started");
        _timerEventRegistration = getManager()->registerEventHandler<NetworkDataEvent>(getServiceId(), this);
        _connectionService->send(_serializationAdmin->serialize(TestMsg{11, "hello"}));
        return true;
    }

    bool stop() final {
        _timerEventRegistration = nullptr;
        LOG_INFO(_logger, "UsingWsService stopped");
        return true;
    }

    void addDependencyInstance(ILogger *logger) {
        _logger = logger;
    }

    void removeDependencyInstance(ILogger *logger) {
        _logger = nullptr;
    }

    void addDependencyInstance(ISerializationAdmin *serializationAdmin) {
        _serializationAdmin = serializationAdmin;
        LOG_INFO(_logger, "Inserted serializationAdmin");
    }

    void removeDependencyInstance(ISerializationAdmin *serializationAdmin) {
        _serializationAdmin = nullptr;
        LOG_INFO(_logger, "Removed serializationAdmin");
    }

    void addDependencyInstance(IConnectionService *connectionService) {
        _connectionService = connectionService;
        LOG_INFO(_logger, "Inserted connectionService");
    }

    void addDependencyInstance(IHostService *) {
    }

    void removeDependencyInstance(IHostService *) {
    }

    void removeDependencyInstance(IConnectionService *connectionService) {
        LOG_INFO(_logger, "Removed connectionService");
    }

    Generator<bool> handleEvent(NetworkDataEvent const * const evt) {
        auto msg = _serializationAdmin->deserialize<TestMsg>(evt->getData());
        LOG_INFO(_logger, "Received TestMsg id {} val {}", msg->id, msg->val);
        getManager()->pushEvent<QuitEvent>(getServiceId());

        co_return (bool)PreventOthersHandling;
    }

private:
    ILogger *_logger{nullptr};
    ISerializationAdmin *_serializationAdmin;
    IConnectionService *_connectionService;
    std::unique_ptr<EventHandlerRegistration> _timerEventRegistration{nullptr};
};