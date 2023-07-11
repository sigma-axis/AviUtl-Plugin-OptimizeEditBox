#include <cmath>

#include "OptimizeEditBox.h"
#include "OptimizeEditBox_Hook.h"

//---------------------------------------------------------------------

namespace OptimizeEditBox
{
	// timeline::frame_color を受け取って枠を描画する基礎関数． ::FrameRect() のラッパー兼拡張．
	static void frame_rect(HDC dc, const RECT& rc, const timeline::frame_color& frame)
	{
		static const auto dc_brush = static_cast<HBRUSH>(::GetStockObject(DC_BRUSH));

		::SetDCBrushColor(dc, frame.color);
		if (frame.is_uniform_one()) ::FrameRect(dc, &rc, dc_brush); // hopefully faster.
		else {
			int x = rc.left;
			int y = rc.top;
			int w = rc.right - rc.left;
			int h = rc.bottom - rc.top;

			auto oldBrush = ::SelectObject(dc, dc_brush);
			if (frame.top > 0)
				::PatBlt(dc, x, y, w, std::min<int>(frame.top, h), PATCOPY);
			if (frame.bottom > 0)
				::PatBlt(dc, x, y + h, w, -std::min<int>(frame.bottom, h), PATCOPY);
			y += frame.top; h -= frame.top + frame.bottom;
			if (frame.left > 0)
				::PatBlt(dc, x, y, std::min<int>(frame.left, w), h, PATCOPY);
			if (frame.right > 0)
				::PatBlt(dc, x + w, y, -std::min<int>(frame.right, w), h, PATCOPY);
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
		static constinit GRADIENT_RECT gRect = { .UpperLeft = 0, .LowerRight = 1 };
		TRIVERTEX verts[] = {
			{.x = left,  .y = top,    .Red = r1, .Green = g1, .Blue = b1, .Alpha = 0 },
			{.x = right, .y = bottom, .Red = r2, .Green = g2, .Blue = b2, .Alpha = 0 },
		};
		// note: ::GradientFill() -> ::GdiGradientFill() に差し替え．.lib を減らせる．
		::GdiGradientFill(hdc, verts, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
	}

	// グラデーション幅が 0 以下の例外的な場面で呼ぶ関数．2色に塗り分けるだけ．本当に必要かは不明．
	static void split_fill(HDC dc, const RECT& rc, int middle, COLORREF color1, COLORREF color2)
	{
		// for the case of `g_begin >= g_end`.
		fill_rect(dc, rc.top, rc.bottom, rc.left, middle, color1);
		fill_rect(dc, rc.top, rc.bottom, middle, rc.right, color2);
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
			frame_data->outer.deflate_rect(frm))
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

			if (rc->right <= g_begin) [[unlikely]] fill_rect(dc, *rc, RGB(r1, g1, b1));
			else if (g_end <= rc->left) [[unlikely]] fill_rect(dc, *rc, RGB(r2, g2, b2));
			else if (g_begin >= g_end) [[unlikely]]
				split_fill(dc, *rc, g_begin, RGB(r1, g1, b1), RGB(r2, g2, b2));
			else [[likely]] {
				// denominate some bits to prevent overflows in later calculations.
				int c_left = rc->left, c_right = rc->right,
					c_begin = g_begin, c_end = g_end;
				if (auto l = std::bit_width(static_cast<uint32_t>(c_end - c_begin)); l > 15) [[unlikely]] {
					l -= 15;
					c_left >>= l; c_right >>= l;
					c_begin >>= l; c_end >>= l;
				}
				// hereafter, c_end - c_begin <= 2^15. (possibly equals.)

				int R1 = r1 << 8, G1 = g1 << 8, B1 = b1 << 8,
					R2 = r2 << 8, G2 = g2 << 8, B2 = b2 << 8;
				if (c_left < c_begin)
					fill_rect(dc, rc->top, rc->bottom, rc->left, g_begin, RGB(r1, g1, b1));
				else {
					int c_len = c_end - c_begin;
					R1 += (R2 - R1) * (c_left - c_begin) / c_len; // won't overflow.
					G1 += (G2 - G1) * (c_left - c_begin) / c_len;
					B1 += (B2 - B1) * (c_left - c_begin) / c_len;
					g_begin = rc->left;
					c_begin = c_left; // needs to update c_begin.
				}

				if (c_end < c_right)
					fill_rect(dc, rc->top, rc->bottom, g_end, rc->right, RGB(r2, g2, b2));
				else if (c_end > c_right) { // later c_len could be zero without this condition.
					int c_len = c_end - c_begin;
					R2 += (R2 - R1) * (c_right - c_end) / c_len; // won't overflow.
					G2 += (G2 - G1) * (c_right - c_end) / c_len;
					B2 += (B2 - B1) * (c_right - c_end) / c_len;
					g_end = rc->right;
					// no need to update c_end.
				}

				if (g_begin < g_end)
					grad_fill(dc, rc->top, rc->bottom,
						g_begin, g_end, R1, G1, B1, R2, G2, B2);
			}

			// 枠を描画．
			draw_object_frame(dc, *rc);
		}

		// グラデーションの描画 パターン 3. 単色で塗りつぶす．
		void __cdecl solid(HDC dc, const RECT* rc,
			int32_t r1, int32_t g1, int32_t b1, int32_t r2, int32_t g2, int32_t b2, int32_t, int32_t)
		{
			// case of steps == 1.
			fill_rect(dc, *rc, RGB((r1 + r2) >> 1, (g1 + g2) >> 1, (b1 + b2) >> 1));

			// 枠を描画．
			draw_object_frame(dc, *rc);
		}

		// グラデーションの描画 パターン 4. 階段状のグラデーション．
		void __cdecl steps(HDC dc, const RECT* rc,
			int32_t r1, int32_t g1, int32_t b1, int32_t r2, int32_t g2, int32_t b2, int32_t g_begin, int32_t g_end)
		{
			// 参考にしました:
			// https://github.com/ePi5131/patch.aul/blob/d46adbf95c87e52fe021222aea02837fdd03a6ef/patch/patch_fast_exeditwindow.cpp#L138

			// case of steps >= 2.
			const int grad_steps = theApp.m_gradientSteps;

			if (rc->right <= g_begin) [[unlikely]] fill_rect(dc, *rc, RGB(r1, g1, b1));
			else if (g_end <= rc->left) [[unlikely]] fill_rect(dc, *rc, RGB(r2, g2, b2));
			else if (g_end <= g_begin) [[unlikely]]
				split_fill(dc, *rc, g_begin, RGB(r1, g1, b1), RGB(r2, g2, b2));
			else [[likely]] {
				RECT step{ .top = rc->top, .right = rc->left, .bottom = rc->bottom };
				auto g_len = static_cast<int64_t>(g_end) - g_begin; // extend to 64 bits to avoid overflowing.
				auto r_diff = r2 - r1, g_diff = g2 - g1, b_diff = b2 - b1;
				for (int i = 0; step.right < rc->right; i++) {
					step.left = std::max(step.right, rc->left);

					if (i <= grad_steps)
						step.right = std::min<int32_t>(rc->right,
							g_begin + static_cast<int32_t>(g_len * i / grad_steps));
					else step.right = rc->right;

					if (step.left >= step.right) continue;
					if (step.right <= rc->left) continue;

					auto I = std::min(std::max(2 * i - 1, 0), 2 * grad_steps);
					fill_rect(dc, step, RGB(
						r1 + r_diff * I / (2 * grad_steps),
						g1 + g_diff * I / (2 * grad_steps),
						b1 + b_diff * I / (2 * grad_steps)));
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
