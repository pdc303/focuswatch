#ifndef __FOCUSWATCH_UTIL_H__
#define __FOCUSWATCH_UTIL_H__

#include <limits.h>
#include <windows.h>
#include <stdbool.h>

struct focus_info {
	HWND hwindow;
	bool changed;
	DWORD pid;
	wchar_t name[PATH_MAX];
	wchar_t exe_path[PATH_MAX];
};

void init_focus_info(struct focus_info *fi);
/* returns:
 * 0: success
 * -EGAIN: Temporarily couldn't get window info. Try again.
 * -EFAULT: Other error.
 */
int get_current_focus_info(struct focus_info *fi);

#endif // __FOCUSWATCH_UTIL_H__
