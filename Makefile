check:
	python3 scripts/check_repository.py

kernel-build:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/src modules

clean:
	rm -rf build
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/src clean
