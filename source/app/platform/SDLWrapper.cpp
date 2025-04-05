#include "StdInc.h"

#ifdef NOTSA_USE_SDL3
#include <SDL3/SDL.h>
#include "SDLWrapper.hpp"
#include <bindings/imgui_impl_sdl3.h>

namespace notsa {
namespace SDLWrapper {
bool Initialize() {
    return true;
}

void Terminate() {

}

void ProcessEvents() {
    // Now process events
    const auto* const imCtx = ImGui::GetCurrentContext();
    const auto* const imIO  = imCtx ? &imCtx->IO : nullptr;
    for (SDL_Event e; SDL_PollEvent(&e);) {
        if (imIO) {
            ImGui_ImplSDL3_ProcessEvent(&e);
        }

        switch (e.type) {
        case SDL_EVENT_QUIT: {
            RsGlobal.quit = true;
            continue;
        }
        }

        if (CPad::ProcessEvent(e, imIO && imIO->WantCaptureMouse, imIO && imIO->WantCaptureKeyboard)) {
            continue;
        }

        //NOTSA_LOG_DEBUG("SDL: Unprocessed event: {}", e.type);
    }
}
}; // namespace SDLWrapper
}; // namespace notsa
#endif
