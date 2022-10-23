#pragma once

#include "UselessService.h"

using namespace Ichor;

struct QuitOnStartWithDependenciesService final : public Service<QuitOnStartWithDependenciesService> {
    QuitOnStartWithDependenciesService(DependencyRegister &reg, Properties props, DependencyManager *mng) : Service(std::move(props), mng) {
        reg.registerDependency<UselessService>(this, false);
    }
    ~QuitOnStartWithDependenciesService() final = default;
    StartBehaviour start() final {
        getManager().pushEvent<QuitEvent>(getServiceId());
        return StartBehaviour::SUCCEEDED;
    }

    void addDependencyInstance(UselessService *, IService *) {
    }

    void removeDependencyInstance(UselessService *, IService *) {
    }
};