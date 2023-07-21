#pragma once

#include <tuple>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//---------------------------------------------------------------------

namespace OptimizeEditBox::delay_timer
{
	// 逐次延期しながら遅延するタイマーの実装．
	template<auto onTick, class... TArgs>
		requires std::is_invocable_v<decltype(onTick), TArgs...>
	struct delay_timer {
		using Id = UINT_PTR;
		using Cxt = std::tuple<Id, TArgs...>;

		Cxt cxt{ 0, TArgs{}... };
		Id& id() { return std::get<0>(cxt); }
		bool is_working() { return !is_idle(); }
		bool is_idle() { return id() == 0; }

		void set(uint32_t time, TArgs... args) {
			if (self == this) [[likely]] set_internal(time, args...);
		}
		void discard() { if (is_working()) discard_internal(); }
		void operator()() { tick(); }
		void tick() {
			if (is_working()) {
				if (self == this) [[likely]] tick_internal();
			}
		}

		void activate()
		{
			deactivate();
			self = this;
		}
		static void deactivate()
		{
			if (self != nullptr) {
				self->discard();
				self = nullptr;
			}
		}

	private:
		void set_internal(uint32_t time, TArgs... args)
		{
			// note: specifying the same id with the existing timer prolongs the delay,
			// even if it's not associated to a window handle.
			cxt = std::make_tuple(::SetTimer(nullptr, id(), time, TimerProc), args...);
		}
		void discard_internal()
		{
			::KillTimer(nullptr, id());
			id() = 0;
		}
		void tick_internal()
		{
			discard_internal();
			[&]<size_t... I>(std::index_sequence<I...>) {
				onTick(std::get<1 + I>(cxt)...);
			}(std::index_sequence_for<TArgs...>{});
		}

		// ideally, there desired a mapping {id's} -> {timers}.
		inline static delay_timer* self = nullptr; // also working as a "main switch".
		static void CALLBACK TimerProc(auto, auto, Id id, auto) {
			if (self != nullptr && id == self->id())
				[[likely]] self->tick_internal();
			else ::KillTimer(nullptr, id);
		}
	};
}
