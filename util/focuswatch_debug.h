#ifndef __FOCUSWATCH_DEBUG_H__
#define __FOCUSWATCH_DEBUG_H__

#ifdef DEBUG
void focuswatch_debug_init(void);
void focuswatch_debug_deinit(void);
void focuswatch_debug_log(const wchar_t *fmt, ...);
#else
#define focuswatch_debug_init()
#define focuswatch_debug_deinit()
#define focuswatch_debug_log(...);
#endif

#endif // __FOCUSWATCH_DEBUG_H__
