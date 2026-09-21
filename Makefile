CC ?= cc
CFLAGS ?= -std=gnu11 -Wall -Wextra
KDIR ?= /lib/modules/$(shell uname -r)/build

.PHONY: all build run test test-vm check userspace kernel-build clean
all: build

build:
	uv run --no-project python scripts/run_vm.py --build-only

run:
	uv run --no-project python scripts/run_vm.py

test test-vm check:
	uv run --no-project python scripts/run_vm.py --test

userspace:
	@test "$$(uname -s)" = Linux || { echo "Native tools require Linux; on macOS use make run."; exit 1; }
	mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) -Isrc src/message_sender.c -o build/message-sender
	$(CC) $(CPPFLAGS) $(CFLAGS) -Isrc src/message_reader.c -o build/message-reader

kernel-build:
	@test "$$(uname -s)" = Linux && test -d "$(KDIR)" || { echo "A disposable Linux VM and matching kernel headers are required; on macOS use make run."; exit 1; }
	mkdir -p build/module
	cp src/message_slot.c src/message_slot.h build/module/
	cp src/Makefile.kernel build/module/Makefile
	$(MAKE) -C "$(KDIR)" M="$(CURDIR)/build/module" modules

clean:
	rm -rf build
