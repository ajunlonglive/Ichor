#pragma once

#include <ichor/DependencyManager.h>
#include <ichor/services/logging/Logger.h>
#include <ichor/Service.h>
#include <ichor/LifecycleManager.h>
#include "CustomEvent.h"

using namespace Ichor;

class OtherService final : public Service<OtherService> {
public:
    OtherService(DependencyRegister &reg, Properties props, DependencyManager *mng) : Service(std::move(props), mng) {
        reg.registerDependency<ILogger>(this, true);
    }
    ~OtherService() final = default;

private:
    StartBehaviour start() final {
        ICHOR_LOG_INFO(_logger, "OtherService started with dependency");
        _customEventHandler = getManager().registerEventHandler<CustomEvent>(this);
        return StartBehaviour::SUCCEEDED;
    }

    StartBehaviour stop() final {
        ICHOR_LOG_INFO(_logger, "OtherService stopped with dependency");
        _customEventHandler.reset();
        return StartBehaviour::SUCCEEDED;
    }

    void addDependencyInstance(ILogger *logger, IService *isvc) {
        _logger = logger;

        ICHOR_LOG_INFO(_logger, "Inserted logger svcid {} for svcid {}", isvc->getServiceId(), getServiceId());
    }

    void removeDependencyInstance(ILogger *logger, IService *) {
        _logger = nullptr;
    }

    AsyncGenerator<void> handleEvent(CustomEvent const &evt) {
        ICHOR_LOG_INFO(_logger, "Handling custom event");
        getManager().pushEvent<QuitEvent>(getServiceId());
        getManager().getCommunicationChannel()->broadcastEvent<QuitEvent>(getManager(), getServiceId());
        co_return;
    }

    friend DependencyRegister;
    friend DependencyManager;

    ILogger *_logger{};
    EventHandlerRegistration _customEventHandler{};
};