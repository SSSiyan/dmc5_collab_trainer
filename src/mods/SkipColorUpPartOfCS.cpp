#include "SkipColorUpPartOfCS.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t SkipColorUpPartOfCS::jmp_ret{NULL};
bool SkipColorUpPartOfCS::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	    validation:
            cmp dword ptr [PlayerTracker::playerid], 0 //change this to the char number obviously
            jne code
            cmp byte ptr [SkipColorUpPartOfCS::cheaton], 0
            je code

            push rax
            mov rax, [rsi+0x20] // shell
            test rax, rax
            je popcode
            cmp dword ptr [rax+0x280], 6  // shell id (charge shot)
            pop rax
            jne code
            mov eax, 1 // if charge shot, skip all decs and start at 0
            jmp code
        popcode:
            pop rax
        code:
            dec eax
            mov [rsi+0x40], eax
            jmp qword ptr [SkipColorUpPartOfCS::jmp_ret]
	}
}

// clang-format on

void SkipColorUpPartOfCS::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> SkipColorUpPartOfCS::on_initialize() {
    init_check_box_info();

    m_is_enabled = &SkipColorUpPartOfCS::cheaton;
    m_on_page = Page_Nero;
    m_depends_on = { "PlayerTracker" };
    m_full_name_string = "Skip Color Up Part Of Charge Shots";
    m_author_string = "SSSiyan";
    m_description_string = "Skip the stunning part of Charge Shots.";

    set_up_hotkey();

    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
    auto addr = m_patterns_cache->find_addr(base, "FF C8 89 46 40 E9"); // DevilMayCry5.exe+618884 tu6
    if (!addr) {
        return "Unable to find SkipColorUpPartOfCS pattern.";
    }

    if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 5)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize SkipColorUpPartOfCS";
    }
    return Mod::on_initialize();
}

// void SkipColorUpPartOfCS::on_draw_ui() {}


// during load
//void SkipColorUpPartOfCS::on_config_load(const utility::Config &cfg) {}
// during save
//void SkipColorUpPartOfCS::on_config_save(utility::Config &cfg) {}
// do something every frame
//void SkipColorUpPartOfCS::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void SkipColorUpPartOfCS::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
//void SkipColorUpPartOfCS::on_draw_ui() {}
