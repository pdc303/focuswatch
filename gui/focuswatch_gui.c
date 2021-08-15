#include <stdbool.h>
#include <assert.h>
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include "focuswatch_gui.h"
#include "focuswatch_gui_thread.h"
#include "focuswatch_util.h"
#include "focuswatch_debug.h"

static struct focuswatch_gui_state g_state = { 0 };

/* update the global delay_ms value when a different value is
 * selected in the combo box. the worker thread reads this value live.
 */
static void handle_combo_box_selection_change(HWND hcombobox_rate)
{
	int selidx;

	selidx = ComboBox_GetCurSel(hcombobox_rate);
	g_state.delay_ms = check_rates[selidx].delay_ms;
}

// add an item to the listview, using the info from the given focus_info
static void listview_add_item_from_focus_info(HWND hlistview, struct focus_info *fi)
{
	int cnt;
	LVITEM item = { 0 };
	wchar_t timestr[32];

	cnt = ListView_GetItemCount(hlistview);

	get_now_time_string(timestr, sizeof(timestr));

	item.mask = LVIF_TEXT;
	item.iItem = cnt;
	item.pszText = timestr;

	ListView_InsertItem(hlistview, &item);

	item.pszText = fi->name;
	item.iItem = cnt;
	item.iSubItem = 1;
	ListView_SetItem(hlistview, &item);

	item.pszText = fi->exe_path;
	item.iItem = cnt;
	item.iSubItem = 2;
	ListView_SetItem(hlistview, &item);
}

static void handle_focus_change_notification(struct focus_info *fi)
{
	listview_add_item_from_focus_info(g_state.hlistview, fi);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) {
		case WM_COMMAND:
			if(HIWORD(wParam) == CBN_SELCHANGE) {
				handle_combo_box_selection_change(g_state.hcombobox_rate);
			} else if(HIWORD(wParam) == BN_CLICKED) {
				ListView_DeleteAllItems(g_state.hlistview);
			}
			break;
        case WM_CLOSE:
			focuswatch_gui_thread_finish(g_state.thread);
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
		case MSG_ID_THREAD_TO_MAIN:
			if(wParam == WPARAM_FOCUS_CHANGED) {
				// received a focus change notification from the worker thread
				handle_focus_change_notification((struct focus_info *) lParam);
			}
			break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

// set the given GUI element to the standard system message font
static void element_set_font(HWND hwnd)
{
	NONCLIENTMETRICS metrics;
	HFONT font;
	BOOL B;

	metrics.cbSize = sizeof(NONCLIENTMETRICS);
	B = SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
	assert(B);

	font = CreateFontIndirect(&metrics.lfMessageFont);
	SendMessage(hwnd, WM_SETFONT, (WPARAM) font, MAKELPARAM(TRUE, 0));
}

// convenience func for adding a column to a listview
static void listview_add_column(HWND hlistview, const wchar_t *name, int width)
{
	HWND header;
	int cnt;
	LVCOLUMN col;

	header = ListView_GetHeader(hlistview);
	cnt = Header_GetItemCount(header);

	col.mask = LVCF_TEXT | LVCF_WIDTH;
	col.pszText = (wchar_t *) name;
	col.cx = width;

	ListView_InsertColumn(hlistview, cnt, &col);
}

static bool make_main_window(HINSTANCE hinstance, int ncmdshow)
{
	WNDCLASSEX wc;
	INITCOMMONCONTROLSEX cex = { 0 };
	BOOL B;
	ATOM a;

	cex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	cex.dwICC = ICC_WIN95_CLASSES;
	B = InitCommonControlsEx(&cex);
	assert(B);

	wc.cbSize = sizeof(wc);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_classname;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	a = RegisterClassEx(&wc);

	if(!a) {
		MessageBox(NULL, L"Window class registration failed", L"Error",
										MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	g_state.hwindow = CreateWindowEx(
									WS_EX_CLIENTEDGE,
									g_classname,
									L"FocusWatch",
									WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
									CW_USEDEFAULT, CW_USEDEFAULT,
									900, 800,
									NULL, NULL, hinstance, NULL);

	if(!g_state.hwindow) {
		MessageBox(NULL, L"Window creation failed", L"Error",
									MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	element_set_font(g_state.hwindow);

	g_state.hstatic_rate = CreateWindowEx(
									0,
									WC_STATIC,
									L"Check rate",
									WS_VISIBLE | WS_CHILD,
									10, 25, 60, 40,
									g_state.hwindow, NULL,
									hinstance, NULL);
	element_set_font(g_state.hstatic_rate);

	g_state.hcombobox_rate = CreateWindowEx(
									0,
									WC_COMBOBOX, L"",
									CBS_DROPDOWN | CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
									75, 20, 150, 40,
									g_state.hwindow, (HMENU) GUI_ELEMENT_ID_COMBO_BOX_RATE,
									hinstance, NULL);
	element_set_font(g_state.hcombobox_rate);

	for(int i = 0; i < _countof(check_rates); i++) {
		ComboBox_AddString(g_state.hcombobox_rate, check_rates[i].description);
	}
	ComboBox_SetCurSel(g_state.hcombobox_rate, 2);
	handle_combo_box_selection_change(g_state.hcombobox_rate);

	g_state.hclear_button = CreateWindowEx(
									0,
									WC_BUTTON,
									L"Clear",
									WS_CHILD | WS_VISIBLE,
									790, 10, 80, 40,
									g_state.hwindow, NULL,
									hinstance, NULL);
	element_set_font(g_state.hclear_button);

	g_state.hlistview = CreateWindowEx(
									0,
									WC_LISTVIEW,
									L"",
									WS_VISIBLE | WS_BORDER | WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
									10, 60, 860, 600,
									g_state.hwindow, NULL,
									hinstance, NULL);
	element_set_font(g_state.hlistview);

	listview_add_column(g_state.hlistview, L"Time", 80);
	listview_add_column(g_state.hlistview, L"Name", 440);
	listview_add_column(g_state.hlistview, L"EXE", 340);

	ShowWindow(g_state.hwindow, ncmdshow);
	UpdateWindow(g_state.hwindow);

	return true;
}

static bool do_message_loop(HINSTANCE hInstance)
{
	BOOL B;
	MSG msg;

	while(true) {
		B = GetMessage(&msg, NULL, 0, 0);

		if((B == -1) || (B == 0)) {
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
								LPSTR lpCmdLine, int nCmdShow)
{
	bool b;

	focuswatch_debug_init();

	b = make_main_window(hInstance, nCmdShow);

	if(!b) {
		return 0;
	}

	g_state.thread = focuswatch_gui_thread_create_worker_thread(g_state.hwindow, &g_state.delay_ms);

	b = do_message_loop(hInstance);

	focuswatch_debug_deinit();

	return !(b == true);
}
