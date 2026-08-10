#pragma once

#include <string>

#include "Enums/HookMode.h"
#include "Enums/HookType.h"

namespace ReversibleHooks {
namespace ReversibleHook {
class Hook {
public:
    Hook(std::string name) :
        m_Name{ std::move(name) }
    {
    }

    virtual ~Hook() = default;

    virtual HookType Type() const noexcept = 0;
    virtual HookMode Mode() const noexcept = 0;
    virtual void Check() = 0;

    const auto& Name() const noexcept { return m_Name; }

protected:
    std::string m_Name{}; //!< Name of hook, ex.: `Add` (Referring to CEntity::Add)
};
}; // namespace ReversibleHook
}; // namespace ReversibleHooks

