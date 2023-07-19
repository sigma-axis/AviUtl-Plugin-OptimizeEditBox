#pragma once

#include <cstdint>
#include <bit>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//---------------------------------------------------------------------

namespace OptimizeEditBox::timeline
{
	struct thickness {
		uint8_t left, top, right, bottom;
		constexpr bool operator==(const thickness& other) const {
			return std::bit_cast<uint32_t>(*this) == std::bit_cast<uint32_t>(other);
		}
		constexpr bool is_uniform_one() const {
			constexpr uint32_t uniform_one = std::bit_cast<uint32_t>(thickness{ 1,1,1,1 });
			return std::bit_cast<uint32_t>(*this) == uniform_one;
		}
		constexpr bool is_empty() const {
			constexpr uint32_t empty = std::bit_cast<uint32_t>(thickness{ 0,0,0,0 });
			return std::bit_cast<uint32_t>(*this) == empty;
		}
		constexpr bool deflate_rect(RECT& rc) const {
			rc.left += left; rc.right -= right;
			rc.top += top; rc.bottom -= bottom;
			return rc.left < rc.right && rc.top < rc.bottom;
		}
	};

	struct frame_color {
		COLORREF color;
		thickness thick;

		constexpr bool is_visible() const { return color != CLR_INVALID; }
		template<bool inner_most = false>
		void normalize() {
			if (thick.is_empty()) color = CLR_INVALID;
			else if constexpr (inner_most) {
				if (color == CLR_INVALID) thick = { 0,0,0,0 };
			}
		}

		constexpr bool operator==(const frame_color& other) const {
			return std::bit_cast<uint64_t>(*this) == std::bit_cast<uint64_t>(other);
		}
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
