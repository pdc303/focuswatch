#include <stdio.h>
#include <assert.h>
#include "focuswatch_gui_thread.h"
#include "focuswatch_util.h"

static DWORD WINAPI worker_thread_func(LPVOID lpParameter)
{
	struct focuswatch_gui_thread_state *state = lpParameter;
	struct focus_info fi;

	init_focus_info(&fi);

	while(!state->quit) {
		LRESULT lres;
		int r;

		r = get_current_focus_info(&fi);

		if(r == 0) {
			// if focus has changed then send a message to the GUI thread
			if(fi.changed) {
				lres = SendMessage(state->hwindow,
										MSG_ID_THREAD_TO_MAIN,
										WPARAM_FOCUS_CHANGED,
										(LPARAM) &fi);
				assert(lres == 0);
			}
		}

		Sleep(*state->delay_ms);
	}

	ExitThread(0);
	return 0;
}

FOCUSWATCH_THREAD_HANDLE focuswatch_gui_thread_create_worker_thread(HWND hwindow,
																	volatile unsigned int *delay_ms)
{
	struct focuswatch_gui_thread_state *state;

	state = malloc(sizeof(*state));
	assert(state);

	state->hwindow = hwindow;
	state->delay_ms = delay_ms;

	state->hthread = CreateThread(NULL, 0, worker_thread_func, state, 0, NULL);
	assert(state->hthread);

	return state;
}

void focuswatch_gui_thread_finish(FOCUSWATCH_THREAD_HANDLE thread)
{
	struct focuswatch_gui_thread_state *state = thread;
	DWORD status;

	// tell the thread to finish
	state->quit = true;

	// wait for thread to exit
	status = WaitForSingleObject(state->hthread, 10000);
	assert(status == WAIT_OBJECT_0);

	free(thread);
}
