#pragma once

#include <tuple>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//---------------------------------------------------------------------

namespace OptimizeEditBox::delay_timer
{
	// 逐次延期しながら遅延するタイマーの実装．
	template<auto on_tick, class... TArgs>
		requires std::invocable<decltype(on_tick), TArgs...>
	struct delay_timer {
		using Id = UINT_PTR;
		using Cxt = std::tuple<Id, TArgs...>;

		inline static constinit Cxt cxt{ 0, TArgs{}... };
		static Id& id() { return std::get<0>(cxt); }
		static bool is_working() { return !is_idle(); }
		static bool is_idle() { return id() == 0; }
		inline static constinit bool is_active = false;

		static void set(uint32_t time, TArgs... args) {
			if (is_active) set_internal(time, args...);
		}
		static void discard() { if (is_working()) discard_internal(); }
		static void tick() {
			if (is_working() && is_active) tick_internal();
		}

		static void activate() { is_active = true; }
		static void deactivate()
		{
			if (is_active) {
				discard();
				is_active = false;
			}
		}

	private:
		static void set_internal(uint32_t time, TArgs... args)
		{
			// note: specifying the same id with the existing timer prolongs the delay,
			// even if it's not associated to a window handle.
			cxt = { ::SetTimer(nullptr, id(), time, TimerProc), args... };
		}
		static void discard_internal()
		{
			::KillTimer(nullptr, id());
			id() = 0;
		}
		static void tick_internal()
		{
			discard_internal();
			[]<size_t... I>(std::index_sequence<I...>) {
				on_tick(std::get<1 + I>(cxt)...);
			}(std::index_sequence_for<TArgs...>{});
		}

		static void CALLBACK TimerProc(auto, auto, Id call_id, auto) {
			if (is_active && call_id == id()) tick_internal();
			else ::KillTimer(nullptr, call_id);
		}
	};
}
