#ifndef __FOCUSWATCH_GUI_H__
#define __FOCUSWATCH_GUI_H__

#include <windows.h>
#include "focuswatch_gui_thread.h"

static const wchar_t *g_classname = L"myWindowClass";

struct focuswatch_gui_state {
	HWND hwindow;
	HWND hstatic_rate;
	HWND hcombobox_rate;
	HWND hclear_button;
	HWND hlistview;
	volatile unsigned int delay_ms;

	FOCUSWATCH_THREAD_HANDLE thread;
};

enum GUI_ELEMENT_ID {
	GUI_ELEMENT_ID_NONE,
	GUI_ELEMENT_ID_COMBO_BOX_RATE,
	GUI_ELEMENT_ID_BUTTON_CLEAR,
};

struct check_rate {
	const wchar_t *description;
	unsigned int delay_ms;
};

struct check_rate check_rates[] = {
	{ L"1 ms (Fastest)", 1 },
	{ L"10 ms", 10 },
	{ L"100 ms", 100 },
	{ L"1000 ms (Slowest)", 1000 }
};

#endif // __FOCUSWATCH_GUI_H__
