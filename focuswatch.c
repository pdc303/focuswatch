#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <shlwapi.h>
#include "focuswatch_util.h"

#define CHECK_DELAY_MS 100

struct fw_config {
	// delay before monitoring focus, in seconds
	unsigned int predelay;
	bool exit_after_focus_change;
};

static void handle_commandline_args(int argc, char **argv, struct fw_config *cfg)
{
	int i = 0;

	cfg->predelay = 0;
	cfg->exit_after_focus_change = false;

	while(i < argc) {
		const char *arg = argv[i++];

		if(!strcmp(arg, "-w")) {
			const char *arg2;
			int n;

			arg2 = argv[i++];
			n = sscanf(arg2, "%d", &cfg->predelay);
			if(n != 1) {
				fprintf(stderr, "Error: Invalid number: %s\n", arg2);
				exit(1);
			}
		} else if(!strcmp(arg, "-x")) {
			cfg->exit_after_focus_change = true;
		}
	}
}

#define TRYLOOP_MAX_WAIT_MS 10000
bool get_current_focus_info_tryloop(struct focus_info *fi)
{
	int err = -EAGAIN;
	unsigned int waited = 0;

	while(err != 0) {
		err = get_current_focus_info(fi);

		if(err != 0) {
			if(waited >= (TRYLOOP_MAX_WAIT_MS * 1000)) {
				return false;
			}

			usleep(100000);
		}
	}

	return true;
}

int main(int argc, char **argv)
{
	struct fw_config cfg;
	struct focus_info fi;
	unsigned int counter = 0;

	handle_commandline_args(argc, argv, &cfg);

	init_focus_info(&fi);

	usleep(cfg.predelay * 1000000);

	printf("%-10s%-8s%-30s%-80s\n", "time", "PID", "EXE", "Name");

	while(1) {
		bool b;

		b = get_current_focus_info_tryloop(&fi);

		if(!b) {
			fprintf(stderr, "Error: Timeout getting focus info\n");
			return 1;
		}

		if(fi.changed) {
			char timestr[32];
			time_t t;
			struct tm *tmp;
			wchar_t exe_path[PATH_MAX];

			t = time(NULL);
			tmp = localtime(&t);
			assert(tmp);

			strftime(timestr, sizeof(timestr), "%H:%M:%S", tmp);

			wcsncpy(exe_path, fi.exe_path, _countof(exe_path));

			PathStripPathW(exe_path);

			wprintf(L"%-10s%-8lu%-30S%-80S\n", timestr, fi.pid, exe_path, fi.name);

			if(cfg.exit_after_focus_change && (counter++ == 1)) {
				break;
			}
		}
		usleep(CHECK_DELAY_MS * 1000);
	}

	return 0;
}
