#pragma once

#include <bit>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace OptimizeEditBox
{
	// .ini からは 0xRRGGBB 形式で読み書きできる COLORREF のラッパー．
	// signed int からの変換に R と B を入れ替えるようにして，
	// .ini からの読み書きには必ず signed int を介するように書き直した．
	class colorref_wrap {
		COLORREF val;

	public:
		constexpr colorref_wrap(const COLORREF& c) : val{ c } {}
		constexpr colorref_wrap() : colorref_wrap{ COLORREF(-1) } {}
		constexpr colorref_wrap(int32_t c) : colorref_wrap{ static_cast<COLORREF>(swap_02bytes(static_cast<uint32_t>(c))) } {}

		constexpr operator int32_t() const { return static_cast<int32_t>(swap_02bytes(val)); }
		constexpr operator COLORREF() const { return val; }

		constexpr colorref_wrap(const colorref_wrap& c) { *this = c; }
		constexpr colorref_wrap& operator=(const colorref_wrap& c) {
			val = c.val;
			return *this;
		}

		constexpr operator bool() const { return is_valid(); }
		constexpr bool is_none() const { return val >= 1u << 24; }
		constexpr bool is_valid() const { return !is_none(); }

		constexpr static uint32_t swap_02bytes(uint32_t x) {
			return (0xff00ff00 & x) | (0x00ff00ff & std::rotl(x, 16));
		}
	};

}
