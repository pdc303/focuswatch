CFLAGS := -Wall -Werror --std=c99 -D UNICODE=1 -D _WIN32_IE=0x0500 -D WINVER=0x500
LFLAGS :=
GUI_LFLAGS := -lcomctl32 -lUxTheme -lGdi32
COMMON_SRCS := util/focuswatch_util.c

ifeq ($(DEBUG),1)
CFLAGS += -O0 -ggdb3 -DDEBUG
GUI_LFLAGS += -mconsole
COMMON_SRCS += util/focuswatch_debug.c
else
CFLAGS += -O2 -g
GUI_LFLAGS += -mwindows -Wl,--subsystem,windows
LFLAGS += -s
endif

INCS := -Iutil

.PHONY: default
default: dist/focuswatch_cli.exe dist/focuswatch_gui.exe

dist/focuswatch_cli.exe: dist cli/focuswatch.c $(COMMON_SRCS)
	gcc $(CFLAGS) $(INCS) $(filter %.c,$^) -o $@ $(LFLAGS) -lShlwapi

build/resource.o: build gui/resource.rc
	windres $(filter %.rc,$^) -o $@

dist/focuswatch_gui.exe: dist gui/focuswatch_gui.c gui/focuswatch_gui_thread.c $(COMMON_SRCS) build/resource.o
	gcc $(CFLAGS) $(INCS) $(filter %.c,$^) $(filter %.o,$^) -o $@ $(LFLAGS) $(GUI_LFLAGS)

build:
	mkdir build

dist:
	mkdir dist

dist/focuswatch.zip: dist/focuswatch_cli.exe dist/focuswatch_gui.exe
	zip -j dist/focuswatch.zip dist/focuswatch_cli.exe dist/focuswatch_gui.exe

.PHONY: package
package: dist/focuswatch.zip

clean:
	rm -f build/resource.o
	rm -f dist/focuswatch_cli.exe
	rm -f dist/focuswatch_gui.exe
	rm -f dist/focuswatch.zip
	rm -rf build
	rm -rf dist
