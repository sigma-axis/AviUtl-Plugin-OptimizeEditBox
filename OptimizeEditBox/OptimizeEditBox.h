#pragma once

#include <cstdint>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using byte = uint8_t;
#pragma warning(push)
#pragma warning(disable : 4819)	// suppress SJIS warnings.
#include "aviutl_plugin_sdk/filter.h"
#pragma warning(pop)

#include "coloref_wrap.h"

//---------------------------------------------------------------------

namespace OptimizeEditBox
{
	class COptimizeEditBoxApp
	{
		int32_t* m_is_playing; // 0: not playing, 1: playing.

	public:
		inline bool is_playing() { return *m_is_playing != 0; }

		int m_editBoxDelay;
		bool m_usesUnicodeInput;
		bool m_usesCtrlA;
		bool m_usesGradientFill;

		colorref_wrap m_innerColor;
		int m_innerEdgeWidth;
		int m_innerEdgeHeight;

		colorref_wrap m_outerColor;
		int m_outerEdgeWidth;
		int m_outerEdgeHeight;

		colorref_wrap m_selectionColor;
		colorref_wrap m_selectionEdgeColor;
		colorref_wrap m_selectionBkColor;

		colorref_wrap m_layerBorderLeftColor;
		colorref_wrap m_layerBorderRightColor;
		colorref_wrap m_layerBorderTopColor;
		colorref_wrap m_layerBorderBottomColor;
		colorref_wrap m_layerSeparatorColor;

		int m_addTextEditBoxHeight;
		int m_addScriptEditBoxHeight;
		int m_tabstopTextEditBox;
		int m_tabstopScriptEditBox;

		HFONT m_font;

	public:

		COptimizeEditBoxApp();

		bool initHook(intptr_t exedit_auf);
		bool termHook();

		bool DllMain(HINSTANCE instance, DWORD reason, void* reserved);
		bool func_init(FILTER* fp);
		bool func_exit(FILTER* fp);
	};

	extern COptimizeEditBoxApp theApp;
}

//---------------------------------------------------------------------
