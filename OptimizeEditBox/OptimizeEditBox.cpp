#include <array>

#include "OptimizeEditBox.hpp"
#include "OptimizeEditBox_Hook.hpp"
#include "delay_timer.hpp"
#include "frame_color.hpp"

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi")

//---------------------------------------------------------------------

namespace OptimizeEditBox
{
	inline bool COptimizeEditBoxApp::initHook(intptr_t exedit_auf)
	{
		using namespace memory;
		if (m_editBoxDelay > 0) {
			true_Exedit_SettingDialog_WndProc = writeAbsoluteAddress(
				exedit_auf + 0x0002E800 + 4, hook_Exedit_SettingDialog_WndProc);
			m_is_playing = reinterpret_cast<decltype(m_is_playing)>(exedit_auf + 0x1a52ec);

			// turn on the timer.
			timer_Exedit_SettingDialog_WndProc::activate();
		}

		if (m_addTextEditBoxHeight != 0 || m_tabstopTextEditBox > 0 || m_font != nullptr) {
			hookAbsoluteCall(exedit_auf + 0x0008C46E, Exedit_CreateTextEditBox);
			if (m_addTextEditBoxHeight != 0)
				addInt32(exedit_auf + 0x0008CC56 + 1, m_addTextEditBoxHeight);
		}

		if (m_addScriptEditBoxHeight != 0 || m_tabstopScriptEditBox > 0 || m_font != nullptr) {
			hookAbsoluteCall(exedit_auf + 0x00087658, Exedit_CreateScriptEditBox);
			if (m_addScriptEditBoxHeight != 0)
				addInt32(exedit_auf + 0x000876DE + 1, m_addScriptEditBoxHeight);
		}

		if (m_usesGradientFill) {
			m_SelectedObjectIndex = reinterpret_cast<decltype(m_SelectedObjectIndex)>(exedit_auf + 0x177a10);
			true_Exedit_FillGradation = reinterpret_cast<decltype(true_Exedit_FillGradation)>(exedit_auf + 0x00036a70);

			// 参考にしました:
			// 蛇色様の Common-Library より当該関数のアドレス．
			// https://github.com/hebiiro/Common-Library/blob/62e8d04319ab6934d6aec6028f95a8ae69a05355/Common/AviUtlInternal.h#L103
			// m_DrawObject = (Type_DrawObject)(m_exedit + 0x00037060);
			true_DrawObject = reinterpret_cast<decltype(true_DrawObject)>(exedit_auf + 0x00037060);
		}

		if (m_hideDotOutline) {
			// 参考にしました:
			// patch.aul の選択中オブジェクトの点線の色を変える機能．
			// https://github.com/nazonoSAUNA/patch.aul/blob/9f7aa96871b29556e228a847356be9ee53acde4e/patch/patch_theme_cc.hpp#L461
			// OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x375fb, 4).store_i32(0, &disp_dialog_pen);
			static auto null_pen = ::GetStockObject(NULL_PEN);
			writeAbsoluteAddress(exedit_auf + 0x375fb, &null_pen);
		}

		if (m_layerBorderLeftColor	!= CLR_INVALID) hookCall(exedit_auf + 0x00038845, Exedit_DrawLineLeft);
		if (m_layerBorderRightColor	!= CLR_INVALID) hookCall(exedit_auf + 0x000388AA, Exedit_DrawLineRight);
		if (m_layerBorderTopColor	!= CLR_INVALID) hookCall(exedit_auf + 0x00038871, Exedit_DrawLineTop);
		if (m_layerBorderBottomColor!= CLR_INVALID) hookCall(exedit_auf + 0x000388DA, Exedit_DrawLineBottom);
		if (m_layerSeparatorColor	!= CLR_INVALID) hookCall(exedit_auf + 0x00037A1F, Exedit_DrawLineSeparator);

		if (m_selectionColor		!= CLR_INVALID) writeAbsoluteAddress(exedit_auf + 0x0003807E, &m_selectionColor);
		if (m_selectionEdgeColor	!= CLR_INVALID) writeAbsoluteAddress(exedit_auf + 0x00038076, &m_selectionEdgeColor);
		if (m_selectionBkColor		!= CLR_INVALID) writeAbsoluteAddress(exedit_auf + 0x00038087, &m_selectionBkColor);

		DetourTransactionBegin();
		DetourUpdateThread(::GetCurrentThread());

		if (m_usesUnicodeInput) {
			if (m_usesCtrlA) hook_GetMessageA = hook_ctrlA_GetMessageA;

			ATTACH_HOOK_PROC(GetMessageA);
			ATTACH_HOOK_PROC(DispatchMessageA);
		}
		if (m_usesGradientFill) {
			using namespace hooks_Exedit_FillGradation;
			hook_Exedit_FillGradation =
				m_gradientSteps < -1 ? solid :
				m_gradientSteps == -1 ? simple :
				m_gradientSteps == 0 ? original : steps;

			ATTACH_HOOK_PROC(Exedit_FillGradation);
			if (m_objectFrame != m_selectedFrame)
				// 参考にしました:
				// ShowWaveform で描画対象オブジェクトの情報を事前取得するためのフック． 
				// https://github.com/hebiiro/AviUtl-Plugin-ShowWaveform/blob/b932a02f4231b1fb6b6f7327ea69d41124c302aa/ShowWaveform/App.cpp#L80
				ATTACH_HOOK_PROC(DrawObject);
		}

		if (DetourTransactionCommit() != NO_ERROR) return false;

		return true;
	}

	inline bool COptimizeEditBoxApp::termHook()
	{
		DetourTransactionBegin();
		DetourUpdateThread(::GetCurrentThread());

		if (m_usesUnicodeInput) {
			DETACH_HOOK_PROC(GetMessageA);
			DETACH_HOOK_PROC(DispatchMessageA);
		}
		if (m_usesGradientFill) {
			DETACH_HOOK_PROC(Exedit_FillGradation);
			if (m_objectFrame != m_selectedFrame)
				DETACH_HOOK_PROC(DrawObject);
		}

		if (DetourTransactionCommit() != NO_ERROR) return false;

		return true;
	}

	static constexpr inline auto swap_byte_02(auto x) -> decltype(x) {
		return (0xff00ff00 & x) | (0x00ff00ff & std::rotl(x, 16));
	}
	void timeline::obj_frame::load(const char* ini_file, const char* section)
	{
		auto read_val = [&](const char* key, auto def) -> decltype(def) {
			return ::GetPrivateProfileIntA(section, key, static_cast<int32_t>(def), ini_file);
		};
		auto coerce_byte = [](int v) -> uint8_t { return std::min(std::max(v, 0), 255); };
#define load_gen(var, key, read, write)	var = read(read_val(key, write(var)))
#define load_color(var, key)	load_gen(var, key, swap_byte_02, swap_byte_02)
#define load_byte(var, key)		load_gen(var, key, coerce_byte, )

		load_color(outer.color,			"outerColor");
		load_byte(outer.thick.left,		"outerLeft");
		load_byte(outer.thick.right,	"outerRight");
		load_byte(outer.thick.top,		"outerTop");
		load_byte(outer.thick.bottom,	"outerBottom");
		outer.normalize();

		load_color(inner.color,			"innerColor");
		load_byte(inner.thick.left,		"innerLeft");
		load_byte(inner.thick.right,	"innerRight");
		load_byte(inner.thick.top,		"innerTop");
		load_byte(inner.thick.bottom,	"innerBottom");
		inner.normalize<true>();

#undef load_byte
#undef load_color
#undef load_gen
	}

	constexpr uint32_t CP_SHIFT_JIS = 932; // CP_UTF8 = 65001
	template<uint32_t codepage, size_t N>
	constexpr auto read_wide_str(wchar_t (&output)[N], auto ansi_reader) {
		static_assert(codepage == CP_SHIFT_JIS || codepage == CP_UTF8);

		constexpr int L = (codepage == CP_UTF8 ? 3 : 2) * N;
		char tmp[L + 1];
		int len = ansi_reader(tmp, L + 1); // len doesn't count the terminating null.
		if (!(0 < len && len <= L - 1)) return false;

		len = ::MultiByteToWideChar(codepage, 0, tmp, -1, nullptr, 0); // len counts the terminating null.
		return 0 < len && len <= N &&
			::MultiByteToWideChar(codepage, 0, tmp, -1, output, N) > 0;
	}
	inline void COptimizeEditBoxApp::loadSettings(const char* ini_file)
	{
		auto read_ini_val = [&](const char* key, auto def) -> decltype(def) {
			return ::GetPrivateProfileIntA("Settings", key, static_cast<int32_t>(def), ini_file);
		};
#define load_integer(name)	m_##name = read_ini_val(#name, m_##name)
#define load_boolean(name)	m_##name = read_ini_val(#name, m_##name ? 1 : 0) != 0
#define load_color(name)	m_##name = swap_byte_02(read_ini_val(#name, swap_byte_02(m_##name)))

		// エディットボックスカスタマイズ関連．
		load_integer(editBoxDelay);
		load_boolean(usesUnicodeInput);
		load_boolean(usesCtrlA);

		load_integer(addTextEditBoxHeight);
		load_integer(addScriptEditBoxHeight);
		load_integer(tabstopTextEditBox);
		m_tabstopTextEditBox = std::max(0, m_tabstopTextEditBox);
		load_integer(tabstopScriptEditBox);
		m_tabstopScriptEditBox = std::max(0, m_tabstopScriptEditBox);

		// フォント名はワイド文字に変換してから CreateFontW() する．
		// UTF8 でフォント名が指定されているものとする．
		if (wchar_t fontName[LF_FACESIZE]; read_wide_str<CP_UTF8>(fontName, [&](char* str, size_t len) {
			return ::GetPrivateProfileStringA("Settings", "fontName", "", str, len, ini_file);
		})) {
			int fontSize = read_ini_val("fontSize", 0);
			int fontPitch = read_ini_val("fontPitch", 0);
			int dpi = ::GetSystemDpiForProcess(::GetCurrentProcess());
			fontSize = fontSize * dpi / 96;

#pragma warning(suppress : 6054) // fontName is null-terminated for sure.
			m_font = ::CreateFontW(fontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, fontPitch, fontName);
		}

		// タイムライン描画関連．
		if (load_boolean(usesGradientFill)) {
			m_objectFrame.load(ini_file, "ObjectFrame");
			m_selectedFrame = m_objectFrame; // 項目が存在しないなら [ObjectFrame] で取得した値が既定値．
			m_selectedFrame.load(ini_file, "SelectedObjectFrame");

			load_integer(gradientSteps);
			if (m_gradientSteps >= 256) m_gradientSteps = 0; // would be equivalent to using GradientFill().
		}

		load_boolean(hideDotOutline);

		load_color(selectionColor);
		load_color(selectionEdgeColor);
		load_color(selectionBkColor);

		load_color(layerBorderLeftColor);
		load_color(layerBorderRightColor);
		load_color(layerBorderTopColor);
		load_color(layerBorderBottomColor);
		load_color(layerSeparatorColor);

#undef load_color
#undef load_boolean
#undef load_integer
	}

	bool COptimizeEditBoxApp::DllMain(HINSTANCE instance, DWORD reason, void* reserved)
	{
		switch (reason) {
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
		char ini_file[MAX_PATH];
		::GetModuleFileNameA(fp->dll_hinst, ini_file, std::size(ini_file));
		::PathRenameExtensionA(ini_file, ".ini");

		loadSettings(ini_file);

		return initHook(exedit_auf);
	}

	bool COptimizeEditBoxApp::func_exit(AviUtl::FilterPlugin* fp)
	{
		if (m_editBoxDelay > 0)
			timer_Exedit_SettingDialog_WndProc::deactivate();

		if (m_font != nullptr) {
			::DeleteObject(m_font);
			m_font = nullptr;
		}

		return termHook();
	}
}

//---------------------------------------------------------------------
