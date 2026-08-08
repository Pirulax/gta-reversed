#include "StdInc.h"
#include <reversiblehooks/HooksUtility.hpp>
#include <reversiblehooks/HookConstants.hpp>
#include "StaticOneWayHook.h"

namespace ReversibleHooks {
namespace ReversibleHook {
StaticOneWayHook::StaticOneWayHook(
    std::string name,
    void*       from,
    void*       to,
    size_t      numStackArgumentsToPreserve,
    bool        preserveRegisters
) :
    m_Name{ name },
    m_From{ from },
    m_To{ to },
    m_NumStackArgumentsToPreserve{ numStackArgumentsToPreserve },
    m_PreserveRegisters{ preserveRegisters }
{
}

StaticOneWayHook::~StaticOneWayHook() {
    State(false);
}

bool StaticOneWayHook::State(bool hooked) {
    if (hooked == std::exchange(m_IsHooked, hooked)) {
        return false; // No change
    }
    if (m_IsHooked) {
        ApplyHook();
    } else {
        RestoreHook();
    }
    return true;
}

void StaticOneWayHook::Check() {
    if (!m_IsHooked) {
        return; // Not hooked, nothing to check
    }
    // Edit & Continue in MSVC works by pointing the address of a function to a `jmp` instruction that jumps to the new function code.
    // We don't know if `from` or `to` is on "our" side, so we check both.
    // - If it's `from` then the hook might've been overwritten
    // - if it's `to`, then the `jmp` we followed (See @ref RecalculateAddressJumpTo) might point to a different address than before, so we need to re-generate the hook code.
    if (memcmp(m_From, m_HookData, m_HookSize) != 0) {
        ApplyHook();
    } else if (RecalculateAddressJumpTo()) {
        GenerateHookCode();
        ApplyHook(true);
    }
}

size_t StaticOneWayHook::GenerateHookCodeWithPreservation(size_t localBufferOffset, bool noLocalBuffer) {
    assert((m_NumStackArgumentsToPreserve || m_PreserveRegisters) && "GenerateHookCodeWithPreservation called without any preservation requested");

    const auto GenerateHookCode = [this](void* buf, size_t bufSize) {
        return Utility::GenerateHookCode(
            static_cast<std::byte*>(buf),
            bufSize,
            m_CalculatedJumpTo,
            m_From,
            m_NumStackArgumentsToPreserve,
            m_PreserveRegisters
        );
    };

    // Always try writing into local buffer first
    // We keep the buffer that might've been allocated anyways, just in case.
    if (!noLocalBuffer) {
        try {
            const auto off = notsa::round_up_to_multiple<size_t>(localBufferOffset, alignof(void*)); // Align offset, this isn't necessary on x86 per-se, but should make perf. better
            if (off < sizeof(m_HookData)) {
                return GenerateHookCode(m_HookData + off, sizeof(m_HookData) - off);
            }
        } catch (std::invalid_argument) {
            /* Buffer too small, we need to (re)allocate */
        }
    }

    // If we already have an allocated buffer, try writing to it directly
    if (m_PreservationCodeData) {
        try {
            Utility::ScopedVirtualProtectModify g{ m_PreservationCodeData.get(), m_PreservationCodeDataBufSize, PAGE_EXECUTE_READWRITE };
            return GenerateHookCode(m_PreservationCodeData.get(), m_PreservationCodeDataBufSize);
        } catch (std::invalid_argument) { // Buffer too small, we need to reallocate
            /* fall down and reallocate */
        }
    }

    // At this point we must reallocate
    std::tie(m_PreservationCodeData, m_PreservationCodeDataBufSize) = Utility::VirtualAllocForCode(
        GenerateHookCode(nullptr, 0) // Calculate required size
    );
    return GenerateHookCodeWithPreservation(localBufferOffset, true);
}

size_t StaticOneWayHook::GenerateHookCode() {
    assert(m_CalculatedJumpTo && "Must calculate jump address before generating hook code");

    const auto GenerateJumpCodeToBuffer = [this](void* to) {
        m_HookSize = Utility::GenerateHookCode(
            m_HookData,
            sizeof(m_HookData),
            m_From,
            m_CalculatedJumpTo,
            0,
            false
        );
        return m_HookSize;
    };

    if (m_NumStackArgumentsToPreserve || m_PreserveRegisters) {
        // Generate preservation code for registers/stack arguments first
        // if it fits into the local buffer then nothing more to do
        // as that buffer is directly copied to the hook address
        const auto size = GenerateHookCodeWithPreservation(m_HookSize);
        if (size < sizeof(m_HookData)) {
            return size;
        }

        // Otherwise we need to jump to that generated code, which then jumps to `m_AddressUsedTo`
        // This must fit into the local buffer, otherwise we have a problem :D
        return GenerateJumpCodeToBuffer(m_PreservationCodeData.get());
    }

    // Generate a simple jump to the target function, no preservation needed
    return GenerateJumpCodeToBuffer(m_CalculatedJumpTo);
}

void StaticOneWayHook::RestoreHook() {
    assert(m_HookSize > 0 && "Nothing to restore, was the hook applied at all?");
    Utility::ScopedVirtualProtectModify g{ m_From, m_HookSize, PAGE_EXECUTE_READWRITE };
    if (memcmp(m_From, m_HookData, m_HookSize) == 0) { // Only restore if our hook code is still there, otherwise it might've been overwritten by something already (Eg.: Edit & Continue)
        memcpy(m_From, m_RestoreHookData, m_HookSize);
    }
}

void StaticOneWayHook::ApplyHook(bool skipJumpToReacalculation) {
    if (!skipJumpToReacalculation && RecalculateAddressJumpTo()) {
        GenerateHookCode();
    }
    Utility::ScopedVirtualProtectModify g{ m_From, m_HookSize, PAGE_EXECUTE_READWRITE };
    memcpy(m_RestoreHookData, m_From, m_HookSize);
    memcpy(m_From, m_HookData, m_HookSize);
}

bool StaticOneWayHook::RecalculateAddressJumpTo() {
    const auto jumpTo = Utility::GetJumpToAddress(m_To);
    if (jumpTo) {
        if (jumpTo == m_From) {
            throw std::runtime_error(std::format("Infinite loop: Function at `{}` performs a jump to itself, most likely it was hooked already in the opposite direction", m_From));
        }
        if (jumpTo == m_To) {
            throw std::runtime_error(std::format("Address at `{}` already hooked to jump to target address at `{}`", m_From, m_To));
        }
    }
    return std::exchange(m_CalculatedJumpTo, jumpTo ? jumpTo : m_To) != m_CalculatedJumpTo;
}
}; // namespace ReversibleHook
}; // namespace ReversibleHooks
