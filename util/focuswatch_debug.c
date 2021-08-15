#ifdef DEBUG
#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <synchapi.h>

struct focuswatch_debug_state {
	HANDLE hmutex;
	FILE *fp;
};

struct focuswatch_debug_state g_debug_state = { 0 };

void focuswatch_debug_init(void)
{
	g_debug_state.hmutex = CreateMutex(NULL, FALSE, L"focuswatch_debug");
	assert(g_debug_state.hmutex);
}

void focuswatch_debug_deinit(void)
{
	CloseHandle(g_debug_state.hmutex);
	if(g_debug_state.fp) {
		fclose(g_debug_state.fp);
	}
}

void focuswatch_debug_log(const wchar_t *fmt, ...)
{
	DWORD dw;
	va_list ap;

	assert(g_debug_state.hmutex);

	dw = WaitForSingleObject(g_debug_state.hmutex, 10000);
	assert(dw == WAIT_OBJECT_0);

	// lazy init the fp
	if(!g_debug_state.fp) {
		g_debug_state.fp = fopen("/temp/focuswatch_debug.log", "w");
		assert(g_debug_state.fp);
	}

	va_start(ap, fmt);
	vfwprintf(g_debug_state.fp, fmt, ap);
	fflush(g_debug_state.fp);
	va_end(ap);

	ReleaseMutex(g_debug_state.hmutex);
}
#endif
