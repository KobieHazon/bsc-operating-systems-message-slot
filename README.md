# BSc Operating Systems - Message Slot

- Course: Operating Systems.
- My implementation is kept separately from supplied exercise files.

## Contents

Linux kernel-module coursework implementing a message-slot character device plus user-space sender and reader programs.

## Files

Exercise/framework material:

- `assignment/hw3.pdf`

Implementation material:

- `src/message_slot.h`
- `src/message_slot.c`
- `src/message_sender.c`
- `src/message_reader.c`
- `src/Makefile.kernel`

## Tech Stack

- C.
- POSIX APIs where applicable.
- Linux character-device and module APIs.
- `ioctl` channel selection and red-black-tree channel storage.

## Run the device tests

Install Docker, QEMU (`qemu-system-aarch64`), and `uv`, then run:

```sh
make test-vm
```

Docker builds the module and user programs against a matching Debian Linux kernel. QEMU boots that kernel in a disposable guest without networking, host disks, or shared host folders. Inside the guest, the tests load the module, create device nodes, exercise read/write and ioctl behavior, verify channel and device isolation, run both command-line tools, and unload/reload the module. Nothing is loaded into the Mac or Docker Desktop host kernel.

The module defaults to major number 240. Its `major_num=0` parameter requests a free major from Linux, avoiding conflicts with existing drivers. The ioctl command stays unchanged.

The suite covers sequential device operations; it is not a concurrent-access stress test.
