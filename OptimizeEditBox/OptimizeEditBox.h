#pragma once

#include <cstdint>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using byte = uint8_t;
#include <aviutl/FilterPlugin.hpp>

//---------------------------------------------------------------------

namespace OptimizeEditBox
{
	class COptimizeEditBoxApp {
		int32_t* m_is_playing; // 0: not playing, 1: playing.

	public:
		bool is_playing() { return *m_is_playing != 0; }

		int m_editBoxDelay;
		bool m_usesUnicodeInput;
		bool m_usesCtrlA;
		bool m_usesGradientFill;

		COLORREF m_innerColor;
		int m_innerEdgeWidth;
		int m_innerEdgeHeight;

		COLORREF m_outerColor;
		int m_outerEdgeWidth;
		int m_outerEdgeHeight;

		COLORREF m_selectionColor;
		COLORREF m_selectionEdgeColor;
		COLORREF m_selectionBkColor;

		COLORREF m_layerBorderLeftColor;
		COLORREF m_layerBorderRightColor;
		COLORREF m_layerBorderTopColor;
		COLORREF m_layerBorderBottomColor;
		COLORREF m_layerSeparatorColor;

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
		bool func_init(AviUtl::FilterPlugin* fp);
		bool func_exit(AviUtl::FilterPlugin* fp);
	};

	extern COptimizeEditBoxApp theApp;
}

//---------------------------------------------------------------------
