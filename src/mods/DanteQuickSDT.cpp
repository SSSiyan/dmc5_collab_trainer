#include "DanteQuickSDT.hpp"
#include "PlayerTracker.hpp"

uintptr_t DanteQuickSDT::jmp_ret{NULL};
uintptr_t DanteQuickSDT::jmp_ret2{NULL};

bool DanteQuickSDT::cheaton{NULL};
bool DanteQuickSDT::danteEvenFasterSDT{true};

static const float defaultDTTapSpeed = 20.0f;
static float currentSDTSpeedup = 5.0f;

    // clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code

		cmp byte ptr [DanteQuickSDT::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [DanteQuickSDT::danteEvenFasterSDT], 1
        je cheatcode2
        mov [currentSDTSpeedup], 0x40400000 // 3.0f
        jmp cheatcont

    cheatcode2:
        mov [currentSDTSpeedup], 0x40a00000 // 5.0f
    cheatcont:
        movss xmm0, [rdi+00000128h]
        mulss xmm0, [currentSDTSpeedup]
		jmp qword ptr [DanteQuickSDT::jmp_ret]

    code:
        movss xmm0, [rdi+00000128h]
        jmp qword ptr [DanteQuickSDT::jmp_ret]
	}
}

// edit how lenient entering dt is when speeding up SDT charge to fix entering DT not breaking you out of grabs
static naked void detour2() { // rdi+0x1a14 is sdt meter/10000.0f
	__asm {
        cmp [PlayerTracker::playerid], 1 // dante
        jne code

		cmp byte ptr [DanteQuickSDT::cheaton], 1
        je cheatcode
    code:
        movss xmm0, [defaultDTTapSpeed]
        jmp retcode

    cheatcode:
        mulss xmm0, [currentSDTSpeedup]
    retcode:
        jmp qword ptr [DanteQuickSDT::jmp_ret2]
	}
}

// clang-format on

void DanteQuickSDT::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteQuickSDT::on_initialize() {
  init_check_box_info();

  m_is_enabled           = &DanteQuickSDT::cheaton;
  m_on_page              = Page_DanteSDT;
  m_depends_on           = { "PlayerTracker" };
  m_full_name_string     = "Quick SDT (+)";
  m_author_string        = "SSSiyan";
  m_description_string   = "Reduces the time you have to hold DT to enter SDT.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = m_patterns_cache->find_addr(base, "FF F3 0F 10 8F 24 11 00 00 F3 0F 10 87 28 01 00 00");
  if (!addr) {
    return "Unable to find DanteQuickSDT pattern.";
  }
  if (!install_new_detour(addr.value() + 9, m_detour, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteQuickSDT";
  }

  auto addr2 = m_patterns_cache->find_addr(base, "F3 0F 10 05 0F 81 4F 06");
  if (!addr2) {
      return "Unable to find DanteQuickSDT pattern 2.";
  }
  if (!install_new_detour(addr2.value(), m_detour2, &detour2, &jmp_ret2, 8)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] 2 failed to initialize", get_name());
      return "Failed to initialize DanteQuickSDT 2";
  }

  return Mod::on_initialize();
}

void DanteQuickSDT::on_config_load(const utility::Config& cfg) {
  danteEvenFasterSDT = cfg.get<bool>("dante_even_faster_sdt").value_or(false);
}

void DanteQuickSDT::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("dante_even_faster_sdt", danteEvenFasterSDT);
}

void DanteQuickSDT::on_draw_ui() {
  ImGui::Checkbox("Even Quicker SDT", &danteEvenFasterSDT);
}
