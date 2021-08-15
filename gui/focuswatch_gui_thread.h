#ifndef __FOCUSWATCH_GUI_THREAD_H__
#define __FOCUSWATCH_GUI_THREAD_H__

#include <windows.h>
#include <stdbool.h>

#define MSG_ID_THREAD_TO_MAIN WM_USER
#define WPARAM_FOCUS_CHANGED 1

// state structure used by the worker thread
struct focuswatch_gui_thread_state {
	// window handle to send messages to
	HWND hwindow;
	// pointer to the current check delay
	volatile const unsigned int *delay_ms;
	// main sets this TRUE to indicate the thread must finish
	volatile bool quit;

	// variables owned by focuswatch_gui_thread

	HANDLE hthread;
	HANDLE hmutex;
};

typedef void *FOCUSWATCH_THREAD_HANDLE;

FOCUSWATCH_THREAD_HANDLE focuswatch_gui_thread_create_worker_thread(HWND hwindow,
																	volatile unsigned int *delay_ms);
void focuswatch_gui_thread_finish(FOCUSWATCH_THREAD_HANDLE thread);

#endif // __FOCUSWATCH_GUI_THREAD_H__
