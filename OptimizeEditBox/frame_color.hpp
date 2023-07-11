#pragma once

#include <cstdint>
#include <bit>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//---------------------------------------------------------------------

namespace OptimizeEditBox::timeline
{
	struct frame_color {
		COLORREF color;
		union {
			struct { uint8_t left, top, right, bottom; };
			uint32_t _thickness;
		};

		constexpr bool is_visible() const { return color != CLR_INVALID; }
		constexpr bool deflate_rect(RECT& rc) const {
			rc.left += left; rc.right -= right;
			rc.top += top; rc.bottom -= bottom;
			return rc.left < rc.right && rc.top < rc.bottom;
		}
		void normalize() { if (_thickness == 0) color = CLR_INVALID; }
		constexpr bool is_uniform_one() const { return _thickness == uniform_one; }

		constexpr bool operator==(const frame_color& other) const {
			return std::bit_cast<uint64_t>(*this) == std::bit_cast<uint64_t>(other);
		}
	private:
		static constexpr uint32_t uniform_one = 0x01010101;
	};

	struct obj_frame {
		frame_color outer;
		frame_color inner;

		constexpr bool is_effective() const { return outer.is_visible() || inner.is_visible(); }
		void load(const char* ini_file, const char* section);

		constexpr bool operator==(const obj_frame& other) const {
			return (!is_effective() && !other.is_effective())
				|| (outer == other.outer && inner == other.inner);
		}
	};
}
