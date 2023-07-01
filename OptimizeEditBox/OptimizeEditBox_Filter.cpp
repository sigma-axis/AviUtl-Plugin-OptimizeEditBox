#include "OptimizeEditBox.h"

using namespace OptimizeEditBox;

//---------------------------------------------------------------------
//		初期化
//---------------------------------------------------------------------

BOOL func_init(FILTER* fp)
{
	return theApp.func_init(fp) ? TRUE : FALSE;
}

//---------------------------------------------------------------------
//		終了
//---------------------------------------------------------------------
BOOL func_exit(FILTER* fp)
{
	return theApp.func_exit(fp) ? TRUE : FALSE;
}

//---------------------------------------------------------------------
//		フィルタ構造体のポインタを渡す関数
//---------------------------------------------------------------------
extern "C" __declspec(dllexport) FILTER_DLL* __stdcall GetFilterTable(void)
{
	constexpr auto filterName = "エディットボックス最適化";
	constexpr auto filterInformation = "エディットボックス最適化 by 蛇色, modified by sigma-axis, based on version 8.0.0 ";

	static FILTER_DLL filter =
	{
		.flag =
			FILTER_FLAG_NO_CONFIG | // このフラグを指定するとウィンドウが作成されなくなってしまう。
			FILTER_FLAG_ALWAYS_ACTIVE | // このフラグがないと「フィルタ」に ON/OFF を切り替えるための項目が追加されてしまう。
			//FILTER_FLAG_DISP_FILTER | // このフラグがないと「設定」の方にウィンドウを表示するための項目が追加されてしまう。
			FILTER_FLAG_EX_INFORMATION,
		.name = const_cast<char*>(filterName),
		.func_init = func_init,
		.func_exit = func_exit,
		.information = const_cast<char*>(filterInformation),
	};

	return &filter;
}

//---------------------------------------------------------------------
//		DllMain
//---------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, void* reserved)
{
	return theApp.DllMain(instance, reason, reserved) ? TRUE : FALSE;
}

//---------------------------------------------------------------------
