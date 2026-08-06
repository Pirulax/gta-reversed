#pragma once

#include <Base.h>
#include <Windows.h>

namespace ReversibleHooks {
namespace Utility {
struct ScopedVirtualProtectModify {
    ScopedVirtualProtectModify(LPVOID address, SIZE_T sz, DWORD newProtect = PAGE_EXECUTE_READWRITE) :
        m_Address{ address },
        m_Size{ sz }
    {
        if (VirtualProtect(address, sz, newProtect, &m_InitialProtect) == 0) {
            throw std::logic_error{ std::format("VirtualProtect failed, error code: {}", GetLastError()) };
        }
    }

    ~ScopedVirtualProtectModify() {
        DWORD prev{};
        if (VirtualProtect(m_Address, m_Size, m_InitialProtect, &prev) == 0) {
            NOTSA_LOG_ERR("VirtualProtect undo failed, error code: {}", GetLastError());
        }
    }

private:
    DWORD  m_InitialProtect{};
    LPVOID m_Address{};
    SIZE_T m_Size{};
};

/*!
 * @brief Copies memory from `src` to `dst`, temporarily changing the protection of the destination memory to allow writing.
 * @param dst The destination address to copy to.
 * @param src The source address to copy from.
 * @param nbytes The number of bytes to copy.
 */
inline void VirtualCopy(void* dst, void* src, size_t nbytes) {
    ScopedVirtualProtectModify g{ dst, nbytes };
    memcpy(dst, src, nbytes);
}

namespace detail {
template<typename T, typename... ConstructorArgs>
    requires std::is_class_v<T>
struct ConstructorWrapper : protected T {
    using T::T;
    T* Construct(ConstructorArgs... args) {
        this->ConstructorWrapper::ConstructorWrapper(std::forward<ConstructorArgs>(args)...);
        return static_cast<T*>(this);
    }
};
};

/*!
 * @tparam T The class to get the constructor's address of
 * @returns Address to the class's constructor
 */
template<typename T, typename... Args>
constexpr void* GetConstructorAddress() {
    return FunctionToVoidPtr(&detail::ConstructorWrapper<T, Args...>::Construct);
}

namespace detail {
template<typename T>
struct DestructorWrapper : protected T {
    T* Destruct() {
        this->T::~T();
        return static_cast<T*>(this);
    }
};
};

/*!
 * @note In case of virtual destructors, there's another one called virtual deliting destructor which is only available through the VMT
 * @tparam T The class to get the destructor's address of
 * @returns Address to the class's (base, eg.: `&Class::~Class`) destructor
 */
template<typename T>
constexpr void* GetScalarDestructorAddress() {
    return FunctionToVoidPtr(&detail::DestructorWrapper<T>::Destruct);
}

}; // namespace Utility
}; // namespace ReversibleHooks
