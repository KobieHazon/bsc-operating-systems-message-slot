# BSc Operating Systems - Message Slot

A historical archive of my CS BSc coursework.

## Contents

Linux kernel-module coursework implementing a message-slot character device plus user-space sender and reader programs.

## Provenance

- Era: CS BSc.
- Last recovered work: 2019-2020 archive copy.
- Supplied exercise material is identified separately below.
- My implementation is kept separately from supplied exercise files.
- Submitted ZIP wrappers and Apple resource forks were omitted.

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

## Validate

```bash
make check
```

## Notes

A purpose-built Debian arm64 container compiled both user-space tools with strict warnings and built a nonempty AArch64 `message_slot.ko` against Linux 6.1.0-52 headers. Loading the module and exercising its device path was not attempted because Docker Desktop uses a different LinuxKit kernel.
