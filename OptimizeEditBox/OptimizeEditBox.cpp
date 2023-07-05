#include <array>

#include "OptimizeEditBox.h"
#include "OptimizeEditBox_Hook.h"
#include "delay_timer.h"

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

//---------------------------------------------------------------------

namespace OptimizeEditBox
{
	COptimizeEditBoxApp theApp;

	COptimizeEditBoxApp::COptimizeEditBoxApp() :
		// 初期化。基本 0。
		m_is_playing{ nullptr },

		m_editBoxDelay{ 0 },
		m_usesUnicodeInput{ false },
		m_usesCtrlA{ false },
		m_usesGradientFill{ false },

		m_innerColor{ 0xffffff }, // white
		m_innerEdgeWidth{ 1 },
		m_innerEdgeHeight{ 1 },

		m_outerColor{ 0x000000 }, // black
		m_outerEdgeWidth{ 1 },
		m_outerEdgeHeight{ 1 },

		m_selectionColor{},
		m_selectionEdgeColor{},
		m_selectionBkColor{},

		m_layerBorderLeftColor{},
		m_layerBorderRightColor{},
		m_layerBorderTopColor{},
		m_layerBorderBottomColor{},
		m_layerSeparatorColor{},

		m_addTextEditBoxHeight{ 0 },
		m_addScriptEditBoxHeight{ 0 },
		m_tabstopTextEditBox{ 0 },
		m_tabstopScriptEditBox{ 0 },

		m_font{ nullptr }
	{}

	bool COptimizeEditBoxApp::initHook(intptr_t exedit_auf)
	{
		using namespace memory;
		if (m_editBoxDelay > 0) {
			true_Exedit_SettingDialog_WndProc = writeAbsoluteAddress(
				exedit_auf + 0x2E800 + 4, hook_Exedit_SettingDialog_WndProc);
			m_is_playing = reinterpret_cast<decltype(m_is_playing)>(exedit_auf + 0x1a52ec);

			// turn on the timer.
			delay_timer::Exedit_SettingDialog_WndProc.activate();
		}

		if (m_usesGradientFill)
			true_Exedit_FillGradation = reinterpret_cast<decltype(true_Exedit_FillGradation)>(exedit_auf + 0x00036a70);

		if (m_layerBorderLeftColor.is_valid()) hookCall(exedit_auf + 0x00038845, Exedit_DrawLineLeft);
		if (m_layerBorderRightColor.is_valid()) hookCall(exedit_auf + 0x000388AA, Exedit_DrawLineRight);
		if (m_layerBorderTopColor.is_valid()) hookCall(exedit_auf + 0x00038871, Exedit_DrawLineTop);
		if (m_layerBorderBottomColor.is_valid()) hookCall(exedit_auf + 0x000388DA, Exedit_DrawLineBottom);
		if (m_layerSeparatorColor.is_valid()) hookCall(exedit_auf + 0x00037A1F, Exedit_DrawLineSeparator);

		if (m_selectionColor.is_valid()) writeAbsoluteAddress(exedit_auf + 0x0003807E, &m_selectionColor);
		if (m_selectionEdgeColor.is_valid()) writeAbsoluteAddress(exedit_auf + 0x00038076, &m_selectionEdgeColor);
		if (m_selectionBkColor.is_valid()) writeAbsoluteAddress(exedit_auf + 0x00038087, &m_selectionBkColor);

		if (m_addTextEditBoxHeight != 0 || m_tabstopTextEditBox > 0 || m_font != nullptr)
		{
			hookAbsoluteCall(exedit_auf + 0x0008C46E, Exedit_CreateTextEditBox);
			if (m_addTextEditBoxHeight != 0)
				addInt32(exedit_auf + 0x0008CC56 + 1, m_addTextEditBoxHeight);
		}

		if (m_addScriptEditBoxHeight != 0 || m_tabstopScriptEditBox > 0 || m_font != nullptr)
		{
			hookAbsoluteCall(exedit_auf + 0x00087658, Exedit_CreateScriptEditBox);
			if (m_addScriptEditBoxHeight != 0)
				addInt32(exedit_auf + 0x000876DE + 1, m_addScriptEditBoxHeight);
		}

		DetourTransactionBegin();
		DetourUpdateThread(::GetCurrentThread());

		if (m_usesUnicodeInput) {
			if (m_usesCtrlA) hook_GetMessageA = hook_ctrlA_GetMessageA;
			ATTACH_HOOK_PROC(GetMessageA);
			ATTACH_HOOK_PROC(DispatchMessageA);
		}
		if (m_usesGradientFill) ATTACH_HOOK_PROC(Exedit_FillGradation);

		if (DetourTransactionCommit() != NO_ERROR) return false;

		return true;
	}

	bool COptimizeEditBoxApp::termHook()
	{
		DetourTransactionBegin();
		DetourUpdateThread(::GetCurrentThread());

		if (m_usesUnicodeInput) {
			DETACH_HOOK_PROC(GetMessageA);
			DETACH_HOOK_PROC(DispatchMessageA);
		}
		if (m_usesGradientFill) DETACH_HOOK_PROC(Exedit_FillGradation);

		if (DetourTransactionCommit() != NO_ERROR) return false;

		return true;
	}

	bool COptimizeEditBoxApp::DllMain(HINSTANCE instance, DWORD reason, void* reserved)
	{
		switch (reason)
		{
		case DLL_PROCESS_ATTACH:
			{
				// この DLL の参照カウンタを増やしておく。
				char moduleFileName[MAX_PATH];
				::GetModuleFileNameA(instance, moduleFileName, std::size(moduleFileName));
				::LoadLibraryA(moduleFileName);

				break;
			}
		case DLL_PROCESS_DETACH:
			{
				break;
			}
		}

		return true;
	}

	// exedit.auf を探してそのバージョンが 0.92 であることを確認，
	// そのハンドルの数値化を返す．失敗したなら 0 を返す．		
	static inline intptr_t find_exedit_auf_092(AviUtl::FilterPlugin* fp)
	{
		constexpr const char* exedit_092_info = "拡張編集(exedit) version 0.92 by ＫＥＮくん";
		AviUtl::SysInfo si;
		fp->exfunc->get_sys_info(nullptr, &si);
		for (int i = 0; i < si.filter_n; i++) {
			if (auto fp_other = fp->exfunc->get_filterp(i);
				fp_other->information != nullptr &&
				std::strcmp(fp_other->information, exedit_092_info) == 0)
				return reinterpret_cast<intptr_t>(fp_other->dll_hinst);
		}
		return 0;
	}

	bool COptimizeEditBoxApp::func_init(AviUtl::FilterPlugin* fp)
	{
		auto exedit_auf = find_exedit_auf_092(fp);
		if (exedit_auf == 0) {
			::MessageBoxA(fp->hwnd, "拡張編集 0.92 が見つかりませんでした．",
				fp->name, MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		// ini ファイルから設定を読み込む。
		char path[MAX_PATH];
		::GetModuleFileNameA(fp->dll_hinst, path, std::size(path));
		::PathRenameExtensionA(path, ".ini");

#define read_ini_val(key, def)	static_cast<int32_t>(			\
	::GetPrivateProfileIntA("Settings", key, static_cast<int32_t>(def), path))
#define read_config(name)										\
	if constexpr (std::is_same_v<bool, decltype(m_##name)>)		\
		m_##name = read_ini_val(#name, m_##name ? 1 : 0) != 0;	\
	else m_##name = static_cast<decltype(m_##name)>(read_ini_val(#name, m_##name))

		read_config(editBoxDelay);
		read_config(usesUnicodeInput);
		read_config(usesCtrlA);
		read_config(usesGradientFill);

		if (m_usesGradientFill) {
			read_config(innerColor);
			read_config(innerEdgeWidth);
			read_config(innerEdgeHeight);

			read_config(outerColor);
			read_config(outerEdgeWidth);
			read_config(outerEdgeHeight);
		}

		read_config(selectionColor);
		read_config(selectionEdgeColor);
		read_config(selectionBkColor);

		read_config(layerBorderLeftColor);
		read_config(layerBorderRightColor);
		read_config(layerBorderTopColor);
		read_config(layerBorderBottomColor);
		read_config(layerSeparatorColor);

		read_config(addTextEditBoxHeight);
		read_config(addScriptEditBoxHeight);
		read_config(tabstopTextEditBox);
		m_tabstopTextEditBox = std::max(0, m_tabstopTextEditBox);
		read_config(tabstopScriptEditBox);
		m_tabstopScriptEditBox = std::max(0, m_tabstopScriptEditBox);

		// フォント名はワイド文字に変換してから CreateFontW() する．
		constexpr uint32_t CP_SHIFT_JIS = 932;
		constexpr auto read_wide_str = []<uint32_t codepage, size_t N>(wchar_t (&output)[N], auto ansi_reader) {
			static_assert(codepage == CP_SHIFT_JIS || codepage == CP_UTF8);

			constexpr int L = (codepage == CP_UTF8 ? 3 : 2) * N;
			char tmp[L + 1];
			int len = ansi_reader(tmp, L + 1); // len doesn't count the terminating null.
			if (!(0 < len && len <= L - 1)) return false;

			len = ::MultiByteToWideChar(codepage, 0, tmp, -1, nullptr, 0); // len counts the terminating null.
			return 0 < len && len <= N &&
				::MultiByteToWideChar(codepage, 0, tmp, -1, output, N) > 0;
		};
		// UTF8 でフォント名が指定されているものとする．
		if (wchar_t fontName[LF_FACESIZE]; read_wide_str.operator()<CP_UTF8>(fontName, [&](char* str, size_t len) {
			return ::GetPrivateProfileStringA("Settings", "fontName", "", str, len, path);
		})) {
			int fontSize = read_ini_val("fontSize", 0);
			int fontPitch = read_ini_val("fontPitch", 0);
			int dpi = ::GetSystemDpiForProcess(::GetCurrentProcess());
			fontSize = fontSize * dpi / 96;

#pragma warning(suppress : 6054) // fontName is null-terminated for sure.
			m_font = ::CreateFontW(fontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, fontPitch, fontName);
		}
#undef read_config
#undef read_ini_val

		return initHook(exedit_auf);
	}

	bool COptimizeEditBoxApp::func_exit(AviUtl::FilterPlugin* fp)
	{
		if (m_editBoxDelay > 0)
			delay_timer::delay_timer::deactivate();

		if (m_font != nullptr) {
			::DeleteObject(m_font);
			m_font = nullptr;
		}

		return termHook();
	}
}

//---------------------------------------------------------------------
