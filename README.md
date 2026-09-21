# BSc Operating Systems - Message Slot

- Course: Operating Systems.

My Linux character-device module and its sender/reader programs. Each device has independent numbered channels. A write replaces the last message on a channel; reads return that message. Messages are 1–128 bytes and channels use nonzero identifiers.

## Requirements

For the isolated VM workflow on macOS or Linux:

- A running Docker installation with Linux ARM64 image support.
- QEMU providing `qemu-system-aarch64`.
- Python 3 managed by `uv`, and Make.

On macOS with Homebrew, `brew install qemu uv` supplies QEMU and uv. Start Docker Desktop separately. The first build downloads the Debian toolchain/kernel packages. Apple Silicon uses hardware acceleration; other hosts use QEMU emulation and can be slower.

## Run and try the device

From the repository root:

```sh
make run
```

This builds the module and tools, then opens a shell in a **disposable Linux VM**. Wait for `Message Slot Linux VM ready` and the guest's `#` prompt. The module is already loaded and `/dev/slot0` and `/dev/slot1` exist. Inside that guest, run:

```sh
/message-sender /dev/slot0 23 "hello from the VM"
/message-reader /dev/slot0 23; echo
```

Expected output:

```text
hello from the VM
```

Try a second channel and then read the first again:

```sh
/message-sender /dev/slot0 24 "another channel"
/message-reader /dev/slot0 23; echo
/message-reader /dev/slot0 24; echo
```

Type `exit` or press Ctrl-D to shut down the guest. Each launch is fresh: messages do not persist. The VM has no network, shared host folders or host disks; the module is never loaded into macOS or the Docker Desktop host kernel.

To build the guest kernel/initramfs without opening it, use `make build`; the files go under `build/vm/`. `make run` always builds a fresh guest using the current source. To skip rebuilding the Docker toolchain image on later runs:

```sh
uv run --no-project python scripts/run_vm.py --skip-build
```

## Tests

```sh
make test
```

The isolated guest runs device and command-line tests covering read/write, ioctl channel selection, message length/error cases, channel/device isolation, and module unload/reload. It then shuts down automatically. `make test-vm` and `make check` are aliases. This is not an exhaustive concurrent-access stress test.

## Native Linux build (disposable VM only)

Do not load coursework kernel modules on a normal workstation or a shared container host. Inside a separate disposable Linux VM with a C compiler, Make, and headers matching its running kernel:

```sh
make userspace
make kernel-build
sudo insmod build/module/message_slot.ko major_num=0
major=$(awk '$2 == "message_slot" {print $1}' /proc/devices)
sudo mknod /dev/slot0 c "$major" 0
sudo ./build/message-sender /dev/slot0 23 "hello"
sudo ./build/message-reader /dev/slot0 23
sudo rmmod message_slot
sudo rm /dev/slot0
```

`major_num=0` requests a free device number. To build against a specific installed kernel, pass `KDIR=/path/to/linux-headers` to `make kernel-build`; load the result only under that matching kernel. `make clean` removes local build output and works on macOS too.

## Files

- `src/message_slot.c` and `src/message_slot.h`: kernel module and device API.
- `src/message_sender.c` and `src/message_reader.c`: command-line tools.
- `assignment/hw3.pdf`: supplied assignment.
- `scripts/` and `docker/`: isolated guest build and interactive launcher.
- `tests/`: device tests and automated guest startup.
