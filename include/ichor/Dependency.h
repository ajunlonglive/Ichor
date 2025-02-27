#pragma once

#include <ichor/Common.h>

namespace Ichor {
    struct Dependency {
        Dependency(uint64_t _interfaceNameHash, bool _required, uint64_t _satisfied) noexcept : interfaceNameHash(_interfaceNameHash), required(_required), satisfied(_satisfied) {}
        Dependency(const Dependency &other) noexcept = default;
        Dependency(Dependency &&other) noexcept = default;
        Dependency& operator=(const Dependency &other) noexcept = default;
        Dependency& operator=(Dependency &&other) noexcept = default;
        bool operator==(const Dependency &other) const noexcept {
            return interfaceNameHash == other.interfaceNameHash && required == other.required;
        }

        uint64_t interfaceNameHash;
        bool required;
        uint64_t satisfied;
    };
}