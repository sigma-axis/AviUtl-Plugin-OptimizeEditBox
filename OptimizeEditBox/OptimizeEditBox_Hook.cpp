#include <cstdint>

#include "OptimizeEditBox.h"
#include "OptimizeEditBox_Hook.h"
#include "delay_timer.h"
#include "editbox_predicates.h"

#include <CommCtrl.h>
#pragma comment(lib, "comctl32")

//---------------------------------------------------------------------

namespace OptimizeEditBox
{
	BOOL WINAPI hook_ctrlA_GetMessageA(MSG* msg, HWND hwnd, UINT msgFilterMin, UINT msgFilterMax)
	{
		auto result = ::GetMessageW(msg, hwnd, msgFilterMin, msgFilterMax);

		// Ctrl + A.
		if (msg->message == WM_KEYDOWN &&
			msg->wParam == 'A' &&
			::GetKeyState(VK_CONTROL) < 0 &&
			editbox_pred::check_classname(msg->hwnd, WC_EDITW)) {

			// テキスト全選択して次のメッセージまで待機．
			::SendMessageW(msg->hwnd, EM_SETSEL, 0, -1);
			return hook_ctrlA_GetMessageA(msg, hwnd, msgFilterMin, msgFilterMax);
		}

		return result;
	}

	IMPLEMENT_HOOK_PROC_NULL(LRESULT, WINAPI, Exedit_SettingDialog_WndProc, (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam))
	{
		using namespace editbox_pred;

		// WM_COMMAND 経由で通知メッセージが一定範囲のエディットボックスから送られてきたかチェック．
		if (int id; message == WM_COMMAND && (id = LOWORD(wparam), check_id(id))) {
			switch (int code = HIWORD(wparam)) {
			case EN_CHANGE:
				// 通知メッセージ EN_CHANGE が送られてきた場合，
				// それがユーザの現在入力対象か，そして最適化対象かをチェック．
				if (auto sender = reinterpret_cast<HWND>(lparam);
					::GetFocus() == sender && check_style(sender)) {

					if (!theApp.is_playing()) {
						// 通常の編集中のユーザの入力と判断，
						// スレッドタイマーを上書きセットする。
						// タイマー停止時に実行するコマンド用の変数をメンバ変数に格納しておく。
						delay_timer::Exedit_SettingDialog_WndProc
							.set(theApp.m_editBoxDelay, hwnd, wparam, lparam);
						return 0;
					}
					else {
						// プレビュー再生中のユーザの入力と判断，
						// 遅延させずに即座に効果を持たせる（再生が停止する）．遅延待機中の更新通知があるなら破棄．
						delay_timer::Exedit_SettingDialog_WndProc.discard();
					}
				}
				break;
			case EN_KILLFOCUS:
				// フォーカスが外れた場合，ユーザの連続入力の中断とみなして更新を確定する．
				// 先送りしていたメッセージのデフォルト処理を即時に実行。

				// 別のエディットボックスであったとしてもその場合 .tick() は何もしないため無害．
				// check_style() で確認するのは Win32 API を呼ぶことになって余計手間かも．
				delay_timer::Exedit_SettingDialog_WndProc();
				break;
			}
		}

		return true_Exedit_SettingDialog_WndProc(hwnd, message, wparam, lparam);
	}

	static LRESULT CALLBACK subclassproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR id, DWORD_PTR)
	{
		switch (msg) {
		case WM_SETFONT:
			wparam = reinterpret_cast<WPARAM>(theApp.m_font); // theApp.m_font is not null here.
			[[fallthrough]];
		case WM_DESTROY:
			::RemoveWindowSubclass(hwnd, subclassproc, id);
			break;
		}

		return ::DefSubclassProc(hwnd, msg, wparam, lparam);
	}

	static HWND Exedit_CreateEditBox(int tabstop,
		DWORD exStyle, LPCWSTR className, LPCWSTR windowName,
		DWORD style, int x, int y, int w, int h,
		HWND parent, HMENU menu, HINSTANCE instance, void* param)
	{
		HWND hwnd = ::CreateWindowExW(exStyle, className, windowName,
			style, x, y, w, h, parent, menu, instance, param);

		if (tabstop > 0) ::SendMessageW(hwnd, EM_SETTABSTOPS, 1, reinterpret_cast<LPARAM>(&tabstop));

		if (theApp.m_font != nullptr) ::SetWindowSubclass(hwnd, subclassproc,
			reinterpret_cast<uintptr_t>(&theApp), 0);
		return hwnd;
	}

	HWND WINAPI Exedit_CreateTextEditBox(DWORD exStyle, LPCWSTR className, LPCWSTR windowName,
		DWORD style, int32_t x, int32_t y, int32_t w, int32_t h,
		HWND parent, HMENU menu, HINSTANCE instance, void* param)
	{
		return Exedit_CreateEditBox(theApp.m_tabstopTextEditBox, exStyle, className, windowName,
			style, x, y, w, h + theApp.m_addTextEditBoxHeight, parent, menu, instance, param);
	}

	HWND WINAPI Exedit_CreateScriptEditBox(DWORD exStyle, LPCWSTR className, LPCWSTR windowName,
		DWORD style, int32_t x, int32_t y, int32_t w, int32_t h,
		HWND parent, HMENU menu, HINSTANCE instance, void* param)
	{
		return Exedit_CreateEditBox(theApp.m_tabstopScriptEditBox, exStyle, className, windowName,
			style, x, y, w, h + theApp.m_addScriptEditBoxHeight, parent, menu, instance, param);
	}
}

//---------------------------------------------------------------------
