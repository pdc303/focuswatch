CFLAGS := -Wall -Werror --std=c99 -D UNICODE=1

ifeq ($(DEBUG),1)
CFLAGS += -O0 -ggdb3
else
CFLAGS += -O2 -g
endif

.PHONY: default
default: dist/focuswatch.exe

dist/focuswatch.exe: dist focuswatch.c focuswatch_util.c
	gcc $(CFLAGS) $(filter %.c,$^) -o $@ -lShlwapi

dist:
	mkdir dist

dist/focuswatch.zip: dist/focuswatch.exe
	zip -j dist/focuswatch.zip dist/focuswatch.exe

.PHONY: package
package: dist/focuswatch.zip

clean:
	rm -f dist/focuswatch.exe
	rm -rf dist
