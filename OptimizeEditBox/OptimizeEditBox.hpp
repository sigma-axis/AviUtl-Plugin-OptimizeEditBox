#pragma once

#include <cstdint>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using byte = uint8_t;
#include <aviutl/FilterPlugin.hpp>

#include "frame_color.hpp"

//---------------------------------------------------------------------

namespace OptimizeEditBox
{
	inline constinit class COptimizeEditBoxApp {
		int32_t* m_is_playing{ nullptr }; // 0: not playing, 1: playing.
		int32_t* m_SelectedObjectIndex{ nullptr };

	public:
		bool is_playing() { return *m_is_playing != 0; }
		int32_t SelectedObjectIndex() { return *m_SelectedObjectIndex; }

		int m_editBoxDelay{ 0 };
		bool m_usesUnicodeInput{ false };
		bool m_usesCtrlA{ false };
		bool m_usesGradientFill{ false };
		bool m_hideDotOutline{ false };

		int m_addTextEditBoxHeight{ 0 };
		int m_addScriptEditBoxHeight{ 0 };
		int m_tabstopTextEditBox{ 0 };
		int m_tabstopScriptEditBox{ 0 };

		HFONT m_font{ nullptr };

		timeline::obj_frame m_objectFrame{
			.outer{ 0x000000, 1, 1, 1, 1 },
			.inner{ 0xffffff, 1, 1, 1, 1 },
		}, m_selectedFrame{};

		int m_gradientSteps{ -1 };

		COLORREF m_selectionColor{ CLR_INVALID };
		COLORREF m_selectionEdgeColor{ CLR_INVALID };
		COLORREF m_selectionBkColor{ CLR_INVALID };

		COLORREF m_layerBorderLeftColor{ CLR_INVALID };
		COLORREF m_layerBorderRightColor{ CLR_INVALID };
		COLORREF m_layerBorderTopColor{ CLR_INVALID };
		COLORREF m_layerBorderBottomColor{ CLR_INVALID };
		COLORREF m_layerSeparatorColor{ CLR_INVALID };

	private:
		bool initHook(intptr_t exedit_auf);
		bool termHook();
		void loadSettings(const char* ini_file);

	public:
		bool DllMain(HINSTANCE instance, DWORD reason, void* reserved);
		bool func_init(AviUtl::FilterPlugin* fp);
		bool func_exit(AviUtl::FilterPlugin* fp);
	} theApp;
}

//---------------------------------------------------------------------
