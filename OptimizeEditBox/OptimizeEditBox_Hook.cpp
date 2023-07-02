#include <cstdint>

#include "OptimizeEditBox.h"
#include "OptimizeEditBox_Hook.h"
#include "delay_timer.h"
#include "editbox_predicatesh.h"

#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

namespace OptimizeEditBox
{
	//---------------------------------------------------------------------

	IMPLEMENT_HOOK_PROC(BOOL, WINAPI, GetMessageA, (MSG* msg, HWND hwnd, UINT msgFilterMin, UINT msgFilterMax))
	{
#if true
		BOOL result = ::GetMessageW(msg, hwnd, msgFilterMin, msgFilterMax);
#else
		BOOL result = true_GetMessageA(msg, hwnd, msgFilterMin, msgFilterMax);
#endif
#if true
		// 親ウィンドウを取得する。
		HWND dlg = ::GetParent(msg->hwnd);
		if (dlg == nullptr) return result;

		// ウィンドウが対象のエディットボックスか確認する。
		if (editbox_pred::check_classname(msg->hwnd, WC_EDITW) &&
			editbox_pred::check_style(msg->hwnd)) {

			if (msg->message == WM_KEYDOWN) {
				switch (msg->wParam)
				{
				case VK_ESCAPE:
					// ESC キーでダイアログが非表示になるのを防ぐ．
					goto discard_message;

				case 'A':
					if (theApp.m_usesCtrlA && ::GetKeyState(VK_CONTROL) < 0) {
						// Ctrl + A.
						::SendMessageW(msg->hwnd, EM_SETSEL, 0, -1);
						goto discard_message;
					}
					break;
				}
			}
			/*		// ↑ は，この処理 ↓ の分岐が少ない版．
					if (msg->message == WM_KEYDOWN && msg->wParam == VK_ESCAPE)
						goto discard_message;

					if (theApp.m_usesCtrlA) {
						if (msg->message == WM_KEYDOWN &&
							msg->wParam == 'A' &&
							::GetKeyState(VK_CONTROL) < 0) {

							::SendMessageW(msg->hwnd, EM_SETSEL, 0, -1);
							goto discard_message;
						}
					}
			*/

			// ダイアログメッセージを処理する。
			if (::IsDialogMessageW(dlg, msg)) {
				// このメッセージはディスパッチしてはならないので WM_NULL に置き換える。
				goto discard_message;
			}
		}
#endif
		return result;

	discard_message:
		msg->hwnd = nullptr;
		msg->message = WM_NULL;
		msg->wParam = 0;
		msg->lParam = 0;
		return result;
	}

	// http://iooiau.net/tips/web20back.html
	// 2色のグラデーションを描画する関数です
	static bool TwoColorsGradient(
		HDC hdc,            // 描画先のデバイスコンテキスト・ハンドルです
		const RECT* pRect,  // 描画する範囲の矩形です
		COLORREF Color1,    // 描画する一つ目の色です
		COLORREF Color2,    // 描画する二つ目の色です
		bool fHorizontal    // 水平のグラデーションを描画する場合は TRUE にします
	)
	{
		// 描画範囲と色を設定します
		TRIVERTEX vert[] = {
			{
				.x = pRect->left,
				.y = pRect->top,
				.Red = static_cast<uint16_t>(GetRValue(Color1) << 8),
				.Green = static_cast<uint16_t>(GetGValue(Color1) << 8),
				.Blue = static_cast<uint16_t>(GetBValue(Color1) << 8),
				.Alpha = 0,
			},
			{
				.x = pRect->right,
				.y = pRect->bottom,
				.Red = static_cast<uint16_t>(GetRValue(Color2) << 8),
				.Green = static_cast<uint16_t>(GetGValue(Color2) << 8),
				.Blue = static_cast<uint16_t>(GetBValue(Color2) << 8),
				.Alpha = 0,
			},
		};
		GRADIENT_RECT rect = { 0, 1 };

		// note: ::GradientFill() -> ::GdiGradientFill() に差し替え．.lib を減らせる．
		return ::GdiGradientFill(hdc, vert, 2, &rect, 1,
			fHorizontal ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V);
	}

	static void frameRect(HDC dc, const RECT* rc, COLORREF color, int edgeWidth, int edgeHeight)
	{
		int x = rc->left;
		int y = rc->top;
		int w = rc->right - rc->left;
		int h = rc->bottom - rc->top;

		auto oldBrush = ::SelectObject(dc, ::CreateSolidBrush(color));
		if (edgeHeight > 0)
		{
			::PatBlt(dc, x, y, w, edgeHeight, PATCOPY);
			::PatBlt(dc, x, y + h, w, -edgeHeight, PATCOPY);
		}
		if (edgeWidth > 0)
		{
			::PatBlt(dc, x, y, edgeWidth, h, PATCOPY);
			::PatBlt(dc, x + w, y, -edgeWidth, h, PATCOPY);
		}
		::DeleteObject(::SelectObject(dc, oldBrush));
	}

	IMPLEMENT_HOOK_PROC_NULL(void, CDECL, Exedit_FillGradation, (HDC dc, const RECT* rc, BYTE r, BYTE g, BYTE b, BYTE gr, BYTE gg, BYTE gb, int gs, int ge))
	{
		COLORREF color1 = RGB(r, g, b);
		COLORREF color2 = RGB(gr, gg, gb);
#if true
		// 大雑把なグラデーション。
		TwoColorsGradient(dc, rc, color1, color2, TRUE);
#else
		// デフォルト処理に近いグラデーション。
		if (gs == 0 && ge == 0)
		{
			TwoColorsGradient(dc, rc, color1, color1, TRUE);
		}
		else
		{
			RECT rc1 = *rc;
			rc1.right = gs;
			TwoColorsGradient(dc, &rc1, color1, color1, TRUE);

			RECT rc2 = *rc;
			rc2.left = gs;
			rc2.right = ge;
			TwoColorsGradient(dc, &rc2, color1, color2, TRUE);

			RECT rc3 = *rc;
			rc3.left = ge;
			TwoColorsGradient(dc, &rc3, color2, color2, TRUE);
		}
#endif
#if true
		// 枠も描画するならここを使う。
		RECT rcFrame = *rc;
		frameRect(dc, &rcFrame, theApp.m_outerColor, theApp.m_outerEdgeWidth, theApp.m_outerEdgeHeight);
		::InflateRect(&rcFrame, -theApp.m_outerEdgeWidth, -theApp.m_outerEdgeHeight);
		frameRect(dc, &rcFrame, theApp.m_innerColor, theApp.m_innerEdgeWidth, theApp.m_innerEdgeHeight);
#endif
	}

	static void draw_colored_rect(COLORREF color, HDC dc, int x, int y, int w, int h, HPEN pen)
	{
		if (pen != nullptr) ::SelectObject(dc, pen);
		if (color == CLR_NONE) return;
		auto oldBrush = ::SelectObject(dc, ::CreateSolidBrush(color));
		::PatBlt(dc, x, y, w, h, PATCOPY);
		::DeleteObject(::SelectObject(dc, oldBrush));
	}

	void Exedit_DrawLineLeft(HDC dc, int mx, int my, int lx, int ly, HPEN pen) {
		draw_colored_rect(theApp.m_layerBorderLeftColor, dc, mx, my, 1, ly - my, pen);
	}
	void Exedit_DrawLineRight(HDC dc, int mx, int my, int lx, int ly, HPEN pen) {
		draw_colored_rect(theApp.m_layerBorderRightColor, dc, mx, my, 1, ly - my, pen);
	}
	void Exedit_DrawLineTop(HDC dc, int mx, int my, int lx, int ly, HPEN pen) {
		draw_colored_rect(theApp.m_layerBorderTopColor, dc, mx, my, lx - my, 1, pen);
	}
	void Exedit_DrawLineBottom(HDC dc, int mx, int my, int lx, int ly, HPEN pen) {
		draw_colored_rect(theApp.m_layerBorderBottomColor, dc, mx, my, lx - my, 1, pen);
	}
	void Exedit_DrawLineSeparator(HDC dc, int mx, int my, int lx, int ly, HPEN pen) {
		draw_colored_rect(theApp.m_layerSeparatorColor, dc, mx, my, 1, ly - my, pen);
	}

	IMPLEMENT_HOOK_PROC_NULL(LRESULT, WINAPI, Exedit_SettingDialog_WndProc, (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam))
	{
		using namespace editbox_pred;

		// WM_COMMAND 経由で通知メッセージが一定範囲のエディットボックスから送られてきたかチェック．
		if (int id; message == WM_COMMAND && (id = LOWORD(wparam), check_id(id))) {
			switch (int code = HIWORD(wparam))
			{
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
		switch (msg)
		{
		case WM_SETFONT:
			wparam = reinterpret_cast<WPARAM>(theApp.m_font);
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

		if (theApp.m_font != nullptr) ::SetWindowSubclass(hwnd, subclassproc,
			reinterpret_cast<uintptr_t>(&theApp), 0);

		if (tabstop > 0) ::SendMessageW(hwnd, EM_SETTABSTOPS, 1, reinterpret_cast<LPARAM>(&tabstop));
		return hwnd;
	}

	HWND WINAPI Exedit_CreateTextEditBox(DWORD exStyle, LPCWSTR className, LPCWSTR windowName,
		DWORD style, int x, int y, int w, int h,
		HWND parent, HMENU menu, HINSTANCE instance, void* param)
	{
		return Exedit_CreateEditBox(theApp.m_tabstopTextEditBox, exStyle, className, windowName,
			style, x, y, w, h + theApp.m_addTextEditBoxHeight, parent, menu, instance, param);
	}

	HWND WINAPI Exedit_CreateScriptEditBox(DWORD exStyle, LPCWSTR className, LPCWSTR windowName,
		DWORD style, int x, int y, int w, int h,
		HWND parent, HMENU menu, HINSTANCE instance, void* param)
	{
		return Exedit_CreateEditBox(theApp.m_tabstopScriptEditBox, exStyle, className, windowName,
			style, x, y, w, h + theApp.m_addScriptEditBoxHeight, parent, menu, instance, param);
	}
}

//---------------------------------------------------------------------
