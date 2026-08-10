#pragma once

#include <cstdlib>
#include <windows.h>
#include <DbgHelp.h>
#include <format>
#include <extensions/CommandLine.h>


namespace notsa {
namespace debug {
std::string GetFunctionInfoAtAddress(uintptr_t address, HANDLE hProcess) {
    hProcess  = hProcess == INVALID_HANDLE_VALUE
        ? GetCurrentProcess()
        : hProcess;

    const auto moduleBase = SymGetModuleBase(hProcess, address);
    if (moduleBase == NULL) {
        return "<unknown>";
    }

    DWORD         lineDisplacement = 0; // Offset from the start address of the function
    IMAGEHLP_LINE lineInfo     = { sizeof(IMAGEHLP_LINE) };
    const auto    hasLineInfo  = SymGetLineFromAddr(hProcess, address, &lineDisplacement, &lineInfo);

    IMAGEHLP_MODULE moduleInfo{ sizeof(IMAGEHLP_MODULE) };
    const auto      hasModuleInfo = SymGetModuleInfo(hProcess, moduleBase, &moduleInfo);

    std::byte   symbuf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME]{};
    auto* const sym = new (symbuf) SYMBOL_INFO{
        .SizeOfStruct = sizeof(SYMBOL_INFO),
        .MaxNameLen   = MAX_SYM_NAME,
    };
    DWORD64    symDisplacement = 0;
    const auto hasSym = SymFromAddr(hProcess, address, &symDisplacement, sym);

    char undecoratedName[MAX_SYM_NAME]{};
    const auto hasUndecoratedName = hasSym && UnDecorateSymbolName(sym->Name, undecoratedName, sizeof(undecoratedName), UNDNAME_COMPLETE) != 0;

    return std::format(
        "0x{:010x}+0x{:04x}: {}!{}:{}",
        static_cast<uintptr_t>(address),
        symDisplacement,
        hasModuleInfo ? moduleInfo.ModuleName : "<unknown>",
        hasUndecoratedName
            ? undecoratedName
            : hasSym
                ? sym->Name
                : "<unknown>",
        hasLineInfo ? lineInfo.LineNumber : 0
    );
}

void InitializeSymbols() {
    SymInitialize(
        GetCurrentProcess(),
        CommandLine::GetExePath().parent_path().string().c_str(),
        TRUE
    );
    std::atexit([]() {
        SymCleanup(GetCurrentProcess());
    });
}
};
};
