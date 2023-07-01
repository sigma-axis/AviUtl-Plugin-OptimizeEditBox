﻿#pragma once

#include <cstdint>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "OptimizeEditBox_Hook.h"

namespace OptimizeEditBox::delay_timer
{
	// 逐次延期しながら遅延するタイマーの実装．
	inline struct delay_timer {
		using Id = UINT_PTR;

		Id id = 0;
		HWND hwnd = nullptr;
		static constexpr UINT message = WM_COMMAND;
		WPARAM wparam = 0;
		LPARAM lparam = 0;
		inline bool is_working() { return !is_idle(); }
		inline bool is_idle() { return id == 0; }

		inline void set(uint32_t time, HWND hwnd, WPARAM wparam, LPARAM lparam) {
			if (self == this) set_internal(time, hwnd, wparam, lparam);
		}
		inline void discard() { if (is_working()) discard_internal(); }
		inline void operator()() { tick(); }
		inline void tick() { if (is_working() && self == this) tick_internal(); }

		void activate()
		{
			deactivate();
			self = this;
		}
		static void deactivate()
		{
			if (self != nullptr) self->discard();
			self = nullptr;
		}

	private:
		inline void set_internal(uint32_t time, HWND hw, WPARAM wp, LPARAM lp)
		{
			// note: specifying the same id with the existing timer prolongs the delay,
			// even if it's not associated to a window handle.
			hwnd = hw; wparam = wp; lparam = lp;
			id = ::SetTimer(nullptr, id, time, TimerProc);
		}
		inline void discard_internal()
		{
			::KillTimer(nullptr, id);
			id = 0;
		}
		void tick_internal()
		{
			discard_internal();
			true_Exedit_SettingDialog_WndProc(hwnd, message, wparam, lparam);
		}

		// ideally, there desired a mapping {id's} -> {timers}.
		inline static delay_timer* self = nullptr; // also working as a "main switch".
		static void CALLBACK TimerProc(auto, auto, Id id, auto) {
			if (self != nullptr && id == self->id) self->tick_internal();
			else ::KillTimer(nullptr, id);
		}
	} Exedit_SettingDialog_WndProc;
}