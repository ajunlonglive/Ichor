#pragma once

#include <ichor/Service.h>
#include "HttpCommon.h"

namespace Ichor {
    class HttpRouteRegistration;

    class IHttpService {
    public:
        virtual std::unique_ptr<HttpRouteRegistration> addRoute(HttpMethod method, std::string_view route, std::function<AsyncGenerator<HttpResponse>(HttpRequest&)> handler) = 0;
        virtual void removeRoute(HttpMethod method, std::string_view route) = 0;
        virtual void setPriority(uint64_t priority) = 0;
        virtual uint64_t getPriority() = 0;

    protected:
        ~IHttpService() = default;
    };

    class HttpRouteRegistration final {
    public:
        HttpRouteRegistration(HttpMethod method, std::string_view route, IHttpService *service) : _method(method), _route(route), _service(service) {}
        HttpRouteRegistration(const HttpRouteRegistration&) = delete;
        HttpRouteRegistration(HttpRouteRegistration&&) = default;
        ~HttpRouteRegistration() {
            _service->removeRoute(_method, _route);
        }

        HttpRouteRegistration& operator=(const HttpRouteRegistration&) = delete;
        HttpRouteRegistration& operator=(HttpRouteRegistration&&) noexcept = default;

    private:
        HttpMethod _method;
        std::string _route;
        IHttpService *_service;
    };
}