#include <ichor/Service.h>

std::atomic<uint64_t> Ichor::Service::_serviceIdCounter = 1;

Ichor::Service::Service() noexcept : IService(), _serviceId(_serviceIdCounter.fetch_add(1, std::memory_order_acq_rel)), _serviceGid(sole::uuid4()), _serviceState(ServiceState::INSTALLED) {

}

Ichor::Service::Service(Ichor::IchorProperties props) noexcept : IService(), _properties(std::move(props)), _serviceId(_serviceIdCounter.fetch_add(1, std::memory_order_acq_rel)), _serviceGid(sole::uuid4()), _serviceState(ServiceState::INSTALLED) {

}

Ichor::Service::~Service() {
    _serviceId = 0;
    _serviceGid.ab = 0;
    _serviceGid.cd = 0;
    _serviceState = ServiceState::UNINSTALLED;
}


bool Ichor::Service::internal_start() {
    if(_serviceState != ServiceState::INSTALLED) {
        return false;
    }

    _serviceState = ServiceState::STARTING;
    if(start()) {
        _serviceState = ServiceState::ACTIVE;
        return true;
    } else {
        _serviceState = ServiceState::INSTALLED;
    }

    return false;
}

bool Ichor::Service::internal_stop() {
    if(_serviceState != ServiceState::ACTIVE) {
        return true;
    }

    _serviceState = ServiceState::STOPPING;
    if(stop()) {
        _serviceState = ServiceState::INSTALLED;
        return true;
    } else {
        _serviceState = ServiceState::UNKNOWN;
    }

    return false;
}

Ichor::ServiceState Ichor::Service::getState() const noexcept {
    return _serviceState;
}

void Ichor::Service::setProperties(Ichor::IchorProperties&& properties) {
    _properties = std::move(properties);
}
