.PHONY: check kernel-build clean

check: test-vm

kernel-build:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/src modules

clean:
	rm -rf build
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/src clean

.PHONY: test-vm
test-vm:
	uv run --no-project python scripts/test_vm.py
