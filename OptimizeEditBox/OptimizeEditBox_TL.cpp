#include <cmath>

#include "OptimizeEditBox.hpp"
#include "OptimizeEditBox_Hook.hpp"

//---------------------------------------------------------------------

namespace OptimizeEditBox
{
	// timeline::frame_color を受け取って枠を描画する基礎関数． ::FrameRect() のラッパー兼拡張．
	static void frame_rect(HDC dc, const RECT& rc, const timeline::frame_color& frame)
	{
		static const auto dc_brush = static_cast<HBRUSH>(::GetStockObject(DC_BRUSH));

		::SetDCBrushColor(dc, frame.color);
		if (frame.thick.is_uniform_one()) ::FrameRect(dc, &rc, dc_brush); // hopefully faster.
		else {
			int x = rc.left;
			int y = rc.top;
			int w = rc.right - rc.left;
			int h = rc.bottom - rc.top;

			auto oldBrush = ::SelectObject(dc, dc_brush);
			if (frame.thick.top > 0)
				::PatBlt(dc, x, y, w, std::min<int>(frame.thick.top, h), PATCOPY);
			if (frame.thick.bottom > 0)
				::PatBlt(dc, x, y + h, w, -std::min<int>(frame.thick.bottom, h), PATCOPY);
			y += frame.thick.top; h -= frame.thick.top + frame.thick.bottom;
			if (frame.thick.left > 0)
				::PatBlt(dc, x, y, std::min<int>(frame.thick.left, w), h, PATCOPY);
			if (frame.thick.right > 0)
				::PatBlt(dc, x + w, y, -std::min<int>(frame.thick.right, w), h, PATCOPY);
			::SelectObject(dc, oldBrush);

			// ::FillRect() と ::PatBlt() どっちがいいのかわからないけど引数指定しやすかったので．
		}
	}

	// 塗りつぶし四角形を描画する． ::FillRect() のラッパー．
	static void fill_rect(HDC dc, const RECT& rc, COLORREF color)
	{
		static const auto dc_brush = static_cast<HBRUSH>(::GetStockObject(DC_BRUSH));
		::SetDCBrushColor(dc, color);
		::FillRect(dc, &rc, dc_brush);
	}
	// wrapping the overload fill_rect(HDC, const RECT&, COLORREF).
	static inline void fill_rect(HDC dc, int top, int bottom, int left, int right, COLORREF color) {
		RECT rc{ left, top, right, bottom }; fill_rect(dc, rc, color);
	}

	// グラデーションを描画． ::GdiGradientFill() のラッパー．
	static void grad_fill(HDC hdc, int top, int bottom, int left, int right,
		uint16_t r1, uint16_t g1, uint16_t b1, uint16_t r2, uint16_t g2, uint16_t b2)
	{
		static constinit GRADIENT_RECT rects[] = { {.UpperLeft = 0, .LowerRight = 1 } };
		TRIVERTEX verts[] = {
			{.x = left,  .y = top,    .Red = r1, .Green = g1, .Blue = b1, .Alpha = 0 },
			{.x = right, .y = bottom, .Red = r2, .Green = g2, .Blue = b2, .Alpha = 0 },
		};
		::GdiGradientFill(hdc, verts, std::size(verts), rects, std::size(rects), GRADIENT_FILL_RECT_H);
	}

	// 現在選択オブジェクトを描画するとき，色設定の切り替えに使う変数．
	static constinit auto const* frame_data = &theApp.m_objectFrame;

	// オブジェクトの枠を描画する関数．
	static void draw_object_frame(HDC dc, const RECT& rc)
	{
		if (!frame_data->is_effective()) return;

		RECT frm = rc;
		if (frame_data->outer.is_visible())
			frame_rect(dc, frm, frame_data->outer);

		if (frame_data->inner.is_visible() &&
			frame_data->outer.thick.deflate_rect(frm))
			frame_rect(dc, frm, frame_data->inner);
	}

	//---------------------------------------------------------------------
	// 以下，フックの実装．

	// 現在選択オブジェクトの場合に色設定を一時的に切り替える．
	IMPLEMENT_HOOK_PROC_NULL(void, __cdecl, DrawObject, (HDC dc, int32_t ObjectIndex))
	{
		if (ObjectIndex == theApp.SelectedObjectIndex()) [[unlikely]] {
			frame_data = &theApp.m_selectedFrame;
			true_DrawObject(dc, ObjectIndex);
			frame_data = &theApp.m_objectFrame;
		}
		else [[likely]] true_DrawObject(dc, ObjectIndex);
	}

	namespace hooks_Exedit_FillGradation
	{
		// 呼び出された場合，rc->left < rc->right が成り立っていて，仮に幅が 0 になるくらいの
		// 小さい拡大率と短いオブジェクトであっても最低幅 1 ピクセルが保証されている模様．

		// 単色塗りつぶし目的でもこれらの関数が呼ばれる (中間点の間や Ctrl 選択時など).
		// その場合 r1 = r2, g1 = g2, b1 = b2, g_begin = 0, g_end = 0 という状況になっている．
		// また，rc->left は最小でも 64.

		// グラデーション幅は，端をつまんでドラッグ中のオブジェクトは 20 ピクセル，
		// 上述の単色塗りつぶしの場合は 0, それ以外はオブジェクト幅となっている模様．
		// 単色塗りつぶしさえ処理しておけば 0 除算の心配はない．

		// グラデーションの描画 パターン 0. 単色で塗りつぶす．theApp.m_gradientSteps == -2 の場合とする．
		void __cdecl solid(HDC dc, const RECT* rc,
			int32_t r1, int32_t g1, int32_t b1, int32_t r2, int32_t g2, int32_t b2, int32_t, int32_t)
		{
			fill_rect(dc, *rc, RGB((r1 + r2) >> 1, (g1 + g2) >> 1, (b1 + b2) >> 1));
			draw_object_frame(dc, *rc);
		}

		// グラデーションの描画 パターン 1. 単純なグラデーション．
		void __cdecl simple(HDC dc, const RECT* rc,
			int32_t r1, int32_t g1, int32_t b1, int32_t r2, int32_t g2, int32_t b2, int32_t, int32_t)
		{
			// 大雑把なグラデーション。
			grad_fill(dc, rc->top, rc->bottom, rc->left, rc->right,
				r1 << 8, g1 << 8, b1 << 8, r2 << 8, g2 << 8, b2 << 8);

			// 枠も描画するならここを使う。
			draw_object_frame(dc, *rc);
		}

		// グラデーションの描画 パターン 2. オリジナルの処理に近い．
		void __cdecl original(HDC dc, const RECT* rc,
			int32_t r1, int32_t g1, int32_t b1, int32_t r2, int32_t g2, int32_t b2, int32_t g_begin, int32_t g_end)
		{
			// デフォルト処理に近いグラデーション。
			// patch.aul のコードを参考に書き直した．
			// 参考にしました:
			// https://github.com/ePi5131/patch.aul/blob/d46adbf95c87e52fe021222aea02837fdd03a6ef/patch/patch_fast_exeditwindow.cpp#L28
			// オーバーフロー対策は 64 bit に伸長するのではなく，予め下位ビットを切り落としておく方針に．

			if (g_end <= rc->left) fill_rect(dc, *rc, RGB(r2, g2, b2));
			else if (rc->right <= g_begin) [[unlikely]] fill_rect(dc, *rc, RGB(r1, g1, b1));
			else {
				r1 <<= 8; g1 <<= 8; b1 <<= 8;
				r2 <<= 8; g2 <<= 8; b2 <<= 8;

				// こまごましたオブジェクトは殆どが単純グラデーション．
				if (g_begin != rc->left || g_end != rc->right) [[unlikely]] {
					// denominate some bits to prevent overflows in later calculations.
					int c_len = g_end - g_begin,
						c_left = rc->left - g_begin, c_right = rc->right - g_begin;
					if (c_len >= (1 << 15)) [[unlikely]] {
						auto l = std::bit_width(static_cast<uint32_t>(c_len)) - 15;
						c_len = c_len >> l;
						c_left = c_left >> l; c_right = c_right >> l;

						// note: since one frame renders at most 10-pixel-wide,
						// c_len could only overflow (>= 2^31)
						// if the object is ~994 hours long or over at 60 FPS.
						// this code assumes those vastly long objects won't exist.
					}
					// hereafter, c_len < 2^15.

					// 仮に g_end <= g_begin の場合であっても，c_len で割る操作は発生しない;
					// このブロック以前の if / else if 句で処理されるか，
					// 以下の分岐は2つとも fill_rect() 呼び出し側を通る．
					// そのため g_begin < g_end の仮説がなくても 0 除算の心配はない．

					const int dR = r2 - r1, dG = g2 - g1, dB = b2 - b1;
					if (rc->left < g_begin)
						fill_rect(dc, rc->top, rc->bottom, rc->left, g_begin, (r1 >> 8) | g1 | (b1 << 8));
					else {
						r1 += dR * c_left / c_len; // won't overflow.
						g1 += dG * c_left / c_len;
						b1 += dB * c_left / c_len;
						g_begin = rc->left;
					}

					if (g_end < rc->right)
						fill_rect(dc, rc->top, rc->bottom, g_end, rc->right, (r2 >> 8) | g2 | (b2 << 8));
					else {
						c_right -= c_len;
						r2 += dR * c_right / c_len; // won't overflow.
						g2 += dG * c_right / c_len;
						b2 += dB * c_right / c_len;
						g_end = rc->right;
					}
				}

				grad_fill(dc, rc->top, rc->bottom,
					g_begin, g_end, r1, g1, b1, r2, g2, b2);
			}

			// 枠を描画．
			draw_object_frame(dc, *rc);
		}

		// グラデーションの描画 パターン 3. 階段状のグラデーション．
		void __cdecl steps(HDC dc, const RECT* rc,
			int32_t r1, int32_t g1, int32_t b1, int32_t r2, int32_t g2, int32_t b2, int32_t g_begin, int32_t g_end)
		{
			// 参考にしました:
			// https://github.com/ePi5131/patch.aul/blob/d46adbf95c87e52fe021222aea02837fdd03a6ef/patch/patch_fast_exeditwindow.cpp#L138

			const int grad_steps = theApp.m_gradientSteps; // >= 1.

			if (g_end <= rc->left) fill_rect(dc, *rc, RGB(r2, g2, b2));
			else if (rc->right <= g_begin) [[unlikely]] fill_rect(dc, *rc, RGB(r1, g1, b1));
			else {
				RECT step{ .top = rc->top, .right = rc->left, .bottom = rc->bottom };
				const auto [i_len, i_rem] = std::div(g_end - g_begin, grad_steps); // to avoid overflowing.
				const auto dr = r2 - r1, dg = g2 - g1, db = b2 - b1;
				for (int i = 0; step.right < rc->right; i++) {
					step.left = std::max(step.right, rc->left);

					if (i <= grad_steps)
						step.right = std::min<int32_t>(rc->right,
							g_begin + i_len * i + (i_rem * i) / grad_steps);
					else step.right = rc->right;

					if (step.left >= step.right) continue;
					if (step.right <= rc->left) continue;

					int I = 2 * i - 1;
					fill_rect(dc, step,
						I < 0 ? RGB(r1, g1, b1) : I > 2 * grad_steps ? RGB(r2, g2, b2) :
						RGB(r1 + dr * I / (2 * grad_steps),
							g1 + dg * I / (2 * grad_steps),
							b1 + db * I / (2 * grad_steps)));
				}
			}

			// 枠を描画．
			draw_object_frame(dc, *rc);
		}
	}

	// レイヤーを囲む矩形の縁色指定．
	static void Exedit_DrawLine(HDC dc, int x, int y, int w, int h, HPEN pen, COLORREF color)
	{
		if (pen != nullptr) ::SelectObject(dc, pen);

		// color is not CLR_INVALID here.
		fill_rect(dc, y, y + h, x, x + w, color);
	}

	void Exedit_DrawLineLeft(HDC dc, int32_t mx, int32_t my, int32_t lx, int32_t ly, HPEN pen) {
		Exedit_DrawLine(dc, mx, my, 1, ly - my, pen, theApp.m_layerBorderLeftColor);
	}
	void Exedit_DrawLineRight(HDC dc, int32_t mx, int32_t my, int32_t lx, int32_t ly, HPEN pen) {
		Exedit_DrawLine(dc, mx, my, 1, ly - my, pen, theApp.m_layerBorderRightColor);
	}
	void Exedit_DrawLineTop(HDC dc, int32_t mx, int32_t my, int32_t lx, int32_t ly, HPEN pen) {
		Exedit_DrawLine(dc, mx, my, lx - mx, 1, pen, theApp.m_layerBorderTopColor);
	}
	void Exedit_DrawLineBottom(HDC dc, int32_t mx, int32_t my, int32_t lx, int32_t ly, HPEN pen) {
		Exedit_DrawLine(dc, mx, my, lx - mx, 1, pen, theApp.m_layerBorderBottomColor);
	}
	void Exedit_DrawLineSeparator(HDC dc, int32_t mx, int32_t my, int32_t lx, int32_t ly, HPEN pen) {
		Exedit_DrawLine(dc, mx, my, 1, ly - my, pen, theApp.m_layerSeparatorColor);
	}
}
