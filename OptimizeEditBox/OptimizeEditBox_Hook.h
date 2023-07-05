#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../Detours.4.0.1/detours.h"
#pragma comment(lib, "../Detours.4.0.1/detours.lib")

//---------------------------------------------------------------------
// Define and Const

#define DECLARE_INTERNAL_PROC(resultType, callType, procName, args) \
	using Type_##procName = resultType (callType*) args; \
	Type_##procName procName = nullptr

#define DECLARE_HOOK_PROC(resultType, callType, procName, args) \
	using Type_##procName = resultType (callType*) args; \
	extern Type_##procName true_##procName; \
	resultType callType hook_##procName args

#define IMPLEMENT_HOOK_PROC(resultType, callType, procName, args) \
	Type_##procName true_##procName = procName; \
	resultType callType hook_##procName args

#define IMPLEMENT_HOOK_PROC_NULL(resultType, callType, procName, args) \
	Type_##procName true_##procName = nullptr; \
	resultType callType hook_##procName args

#define GET_INTERNAL_PROC(module, procName) \
	procName = reinterpret_cast<Type_##procName>(::GetProcAddress(module, #procName))

#define GET_HOOK_PROC(module, procName) \
	true_##procName = reinterpret_cast<Type_##procName>(::GetProcAddress(module, #procName))

#define ATTACH_HOOK_PROC(name) ::DetourAttach(reinterpret_cast<void**>(&true_##name), hook_##name)
#define DETACH_HOOK_PROC(name) ::DetourDetach(reinterpret_cast<void**>(&true_##name), hook_##name)

namespace OptimizeEditBox
{
	//---------------------------------------------------------------------
	// Api Hook

	inline auto true_GetMessageA = GetMessageA, hook_GetMessageA = GetMessageW;
	decltype(GetMessageA) hook_ctrlA_GetMessageA;
	inline auto true_DispatchMessageA = DispatchMessageA;
	constexpr auto hook_DispatchMessageA = DispatchMessageW;
	DECLARE_HOOK_PROC(void, CDECL, Exedit_FillGradation, (HDC dc, const RECT* rc, BYTE r, BYTE g, BYTE b, BYTE gr, BYTE gg, BYTE gb, int gs, int ge));
	DECLARE_HOOK_PROC(LRESULT, WINAPI, Exedit_SettingDialog_WndProc, (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam));

	void Exedit_DrawLineLeft(HDC dc, int mx, int my, int lx, int ly, HPEN pen);
	void Exedit_DrawLineRight(HDC dc, int mx, int my, int lx, int ly, HPEN pen);
	void Exedit_DrawLineTop(HDC dc, int mx, int my, int lx, int ly, HPEN pen);
	void Exedit_DrawLineBottom(HDC dc, int mx, int my, int lx, int ly, HPEN pen);
	void Exedit_DrawLineSeparator(HDC dc, int mx, int my, int lx, int ly, HPEN pen);

	HWND WINAPI Exedit_CreateTextEditBox(DWORD exStyle, LPCWSTR className, LPCWSTR windowName, DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param);
	HWND WINAPI Exedit_CreateScriptEditBox(DWORD exStyle, LPCWSTR className, LPCWSTR windowName, DWORD style, int x, int y, int w, int h, HWND parent, HMENU menu, HINSTANCE instance, LPVOID param);

	//---------------------------------------------------------------------
	// Function

	namespace memory
	{
		// CALL を書き換える。
		template<class T>
		inline void hookCall(intptr_t address, T hookProc)
		{
			uint8_t code[5];
			code[0] = 0xE8; // CALL
			*reinterpret_cast<intptr_t*>(&code[1]) = reinterpret_cast<intptr_t>(hookProc) - (address + 5);

			// CALL を書き換える。そのあと命令キャッシュをフラッシュする。
			::WriteProcessMemory(::GetCurrentProcess(), reinterpret_cast<void*>(address), code, sizeof(code), nullptr);
			::FlushInstructionCache(::GetCurrentProcess(), reinterpret_cast<void*>(address), sizeof(code));
		}

		// CALL を書き換える。
		template<class T>
		inline void hookAbsoluteCall(intptr_t address, T& hookProc)
		{
			uint8_t code[6];
			code[0] = 0xE8; // CALL
			*reinterpret_cast<intptr_t*>(&code[1]) = reinterpret_cast<intptr_t>(hookProc) - (address + 5);
			code[5] = 0x90; // NOP

			// CALL を書き換える。そのあと命令キャッシュをフラッシュする。
			::WriteProcessMemory(::GetCurrentProcess(), reinterpret_cast<void*>(address), code, sizeof(code), nullptr);
			::FlushInstructionCache(::GetCurrentProcess(), reinterpret_cast<void*>(address), sizeof(code));
		}

		// 絶対アドレスを書き換える。
		template<class T>
		inline T writeAbsoluteAddress(intptr_t address, T x)
		{
			// 絶対アドレスから読み込む。
			T retValue = 0;
			::ReadProcessMemory(::GetCurrentProcess(), reinterpret_cast<void*>(address), &retValue, sizeof(retValue), nullptr);
			// 絶対アドレスを書き換える。
			::WriteProcessMemory(::GetCurrentProcess(), reinterpret_cast<void*>(address), &x, sizeof(x), nullptr);
			// 命令キャッシュをフラッシュする。
			::FlushInstructionCache(::GetCurrentProcess(), reinterpret_cast<void*>(address), sizeof(x));
			return retValue;
		}

		// 指定アドレスの値に x を加算する。
		inline void addInt32(intptr_t address, int value)
		{
			// プロセスハンドルを取得する。
			HANDLE process = ::GetCurrentProcess();

			// アドレスの値を取得する。
			int x = 0;
			::ReadProcessMemory(process, reinterpret_cast<void*>(address), &x, sizeof(x), nullptr);

			// value を加算する。
			x += value;

			// アドレスの値を書き換える。
			::WriteProcessMemory(process, reinterpret_cast<void*>(address), &x, sizeof(x), nullptr);

			// 命令キャッシュをフラッシュする。
			::FlushInstructionCache(process, reinterpret_cast<void*>(address), sizeof(x));
		}
	}

	//---------------------------------------------------------------------
}
