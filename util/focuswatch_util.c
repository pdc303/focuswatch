#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <winuser.h>
#include <winbase.h>
#include <psapi.h>
#include <time.h>
#include <assert.h>
#include "focuswatch_util.h"

void init_focus_info(struct focus_info *fi)
{
	memset(fi, 0, sizeof(*fi));
}

int get_current_focus_info(struct focus_info *fi)
{
	HWND hwindow;
	DWORD new_pid;
	wchar_t new_name[PATH_MAX];
	wchar_t new_exe_path[PATH_MAX];
	HANDLE hprocess;
	BOOL b;
	DWORD bufsz;
	int r;
	int ret;

	hwindow = GetForegroundWindow();

	if(fi->hwindow == hwindow) {
		fi->changed = false;
		return 0;
	}

	GetWindowThreadProcessId(hwindow, &new_pid);

	hprocess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, new_pid);

	if(hprocess == NULL) {
		return -EAGAIN;
	}

	/* set the last-error code to zero because for some windows (e.g. the desktop)
	 * GetWindowText() will return zero to indicate an empty window title but
	 * does not set the last-error code to zero to let us differentiate between
	 * an empty window title and failure to get the window title
	 */
	SetLastError(0);
	r = GetWindowText(hwindow, new_name, sizeof(new_name));

	if(r == 0) {
		DWORD ecode = GetLastError();

		if(ecode == ERROR_INVALID_WINDOW_HANDLE) {
			ret = -EAGAIN;
			goto err_out_close_process;
		}

		if(ecode == 0) {
			wcsncpy(new_name, L"(empty)", _countof(new_name));
		} else {
			wcsncpy(new_name, L"(error)", _countof(new_name));
		}
	}

	if(hprocess == NULL) {
		DWORD ecode = GetLastError();
		swprintf(new_exe_path, _countof(new_exe_path), L"Error %lu opening process", ecode);
	}

	bufsz = _countof(new_name);
	b = QueryFullProcessImageName(hprocess, 0, new_exe_path, &bufsz);

	if(b == false) {
		ret = -EAGAIN;
		goto err_out_close_process;
	}

	ret = 0;

	fi->changed = true;
	fi->hwindow = hwindow;
	fi->pid = new_pid;
	wcsncpy(fi->name, new_name, _countof(fi->name));
	wcsncpy(fi->exe_path, new_exe_path, _countof(fi->exe_path));

err_out_close_process:
	CloseHandle(hprocess);

	return ret;
}

void get_now_time_string(wchar_t *sout, size_t sz)
{
	time_t t;
	struct tm *tmp;

	t = time(NULL);
	tmp = localtime(&t);
	assert(tmp);

	wcsftime(sout, sz, L"%H:%M:%S", tmp);
}
