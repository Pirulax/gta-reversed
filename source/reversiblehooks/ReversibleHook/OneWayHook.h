#pragma once

#include <Base.h>

namespace ReversibleHooks {
namespace ReversibleHook {
/*!
 * @brief Hook that does hooking in 1 way only
 */
class OneWayHook : public BaseHook {
    using HookData = std::byte[Constants::MAX_HOOK_DATA_SIZE];

public:
    /*!
     * @param name Name of the hook (eg.: `Add` for `CEntity::Add`)
     * @param from Address we redirect from, this is where the hook is installed
     * @param to The address the hook jumps to
     * @param numStackArgumentsToPreserve If present, the hook code will preserve this many stack arguments (by re-pushing them) before jumping to `to`
     * @param preserveRegisters If true, the hook code will preserve registers (using pushad/popad) before jumping to `to` (`numStackArgumentsToPreserve` must be specified in this case)
     */
    OneWayHook(
        std::string           name,
        void*                 from,
        void*                 to,
        std::optional<size_t> numStackArgumentsToPreserve = std::nullopt,
        bool                  preserveRegisters           = false
    );
    ~OneWayHook() override = default;

    void Switch() override;
    void Check() override;
    const char* Symbol() const override { return "1"; }

    void* GetTo() const noexcept { return m_To; }
    void* GetFrom() const noexcept { return m_From; }

protected:
    /*!
     * @brief Generates the hook code with preservation of registers and stack arguments if needed.
     * @return The size of the generated hook code.
     */
    size_t GenerateHookCodeWithPreservation(size_t localBufferOffset, bool noLocalBuffer = false);

    /*!
     * @brief Generates the hook code that will be written to the hook address.
     * @return The size of the generated hook code.
     */
    size_t GenerateHookCode();

    /*!
     * @brief Restores the original code at the hook address.
     */
    void RestoreHook();

    /*!
     * @brief Applies the hook by copying the generated hook code to the hook address.
     */
    void ApplyHook();

    /*!
     * @brief Re-calculate jump-to address
     * @returns If the address has changed
     */
    bool RecalculateAddressJumpTo();

protected:
    void*                 m_From{};                        //!< Address we redirect from, this is where the hook is installed
    void*                 m_To{};                          //!< The addres we want to jump to (If this itself is just a jump instruction, we will optimize it, see @ref Utility::GetJumpToAddress)
    void*                 m_CalculatedJumpTo{};               //!< The address the hook actually jumps to (See @ref Utility::GetJumpToAddress)
    size_t                m_HookSize{};                    //!< Size of the data overwritten at the hook address
    HookData              m_HookData{};                    //!< Stores the code that is written to the hook address.
    HookData              m_RestoreHookData{};             //!< Data overwritten at the hook address (before we installed the hook)
    Utility::VirtualPtr   m_PreservationCodeData{};        //!< Generated hook code if any (If present the code is first redirected here, then to `m_AddressUsedTo`)
    size_t                m_PreservationCodeDataBufSize{}; //!< Size of the generated hook code's buffer
    std::optional<size_t> m_NumStackArgumentsToPreserve{}; //!< If present, the hook code will preserve this many stack arguments (by re-pushing them) before jumping to `m_AddressUsedTo`
    bool                  m_PreserveRegisters{};           //!< If true, the hook code will preserve registers (using pushad/popad) before jumping to `m_AddressUsedTo`
};
}; // namespace ReversibleHook
}; // namespace ReversibleHooks
