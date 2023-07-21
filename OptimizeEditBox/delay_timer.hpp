#pragma once

#include <tuple>
//#include <memory>
//#include <map>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//---------------------------------------------------------------------

namespace OptimizeEditBox::delay_timer
{
	// 逐次延期しながら遅延するタイマーの実装．
	template<auto on_tick, class... TArgs>
		requires std::is_invocable_v<decltype(on_tick), TArgs...>
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
			[&]<size_t... I>(std::index_sequence<I...>) {
				on_tick(std::get<1 + I>(cxt)...);
			}(std::index_sequence_for<TArgs...>{});
		}

		static void CALLBACK TimerProc(auto, auto, Id call_id, auto) {
			if (is_active && call_id == id()) tick_internal();
			else ::KillTimer(nullptr, call_id);
		}
	};

	//// 没案．他のクラスのメンバ変数にも使える形を目標に設計．
	//template<class... TArgs>
	//class delay_timer_inst {
	//	struct func_base {
	//		virtual void invoke(TArgs... args) = 0;
	//		virtual ~func_base() = default;
	//	};
	//	template<std::invocable<TArgs...> TFunc>
	//	struct func_wrap : func_base {
	//		TFunc func;
	//		func_wrap(TFunc&& func) : func{ std::forward<TFunc>(func) } {}
	//		void invoke(TArgs... args) override { func(args...); }
	//	};
	//	std::unique_ptr<func_base> on_tick{};

	//public:
	//	using Id = UINT_PTR;
	//	using Cxt = std::tuple<Id, TArgs...>;

	//	Cxt cxt{ 0, TArgs{}... };
	//	Id& id() { return std::get<0>(cxt); }
	//	bool is_working() { return !is_idle(); }
	//	bool is_idle() { return id() == 0; }
	//	bool is_active() { return on_tick.get() != nullptr; }

	//	void set(uint32_t time, TArgs... args) {
	//		if (is_active()) set_internal(time, args...);
	//	}
	//	void discard() { if (is_working()) discard_internal(); }
	//	void operator()() { tick(); }
	//	void tick() {
	//		if (is_working() && is_active()) tick_internal();
	//	}

	//	template<std::invocable<TArgs...> TFunc>
	//	void activate(TFunc&& onTick)
	//	{
	//		deactivate();
	//		on_tick.reset(new func_wrap(std::forward<TFunc>(onTick)));
	//	}
	//	void deactivate()
	//	{
	//		if (is_active()) {
	//			discard();
	//			on_tick.reset();
	//		}
	//	}

	//	~delay_timer_inst() { deactivate(); }

	//private:
	//	void set_internal(uint32_t time, TArgs... args)
	//	{
	//		bool is_registered = active_timers.contains(id());
	//		cxt = { ::SetTimer(nullptr, id(), time, TimerProc), args... };
	//		if (!is_registered) active_timers[id()] = this;
	//	}
	//	void discard_internal()
	//	{
	//		::KillTimer(nullptr, id());
	//		active_timers.erase(id());
	//		id() = 0;
	//	}
	//	void tick_internal()
	//	{
	//		discard_internal();
	//		[&] <size_t... I>(std::index_sequence<I...>) {
	//			on_tick->invoke(std::get<1 + I>(cxt)...);
	//		}(std::index_sequence_for<TArgs...>{});
	//	}

	//	inline static std::map<Id, delay_timer_inst*> active_timers{};
	//	static void CALLBACK TimerProc(auto, auto, Id call_id, auto) {
	//		if (auto pair = active_timers.find(call_id);
	//			pair != active_timers.end()) pair->second->tick_internal();
	//		else ::KillTimer(nullptr, call_id);
	//	}
	//};
}
