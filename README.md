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
- `pthread` for the parallel-find assignment.
- Linux kernel-module APIs for the message-slot assignment.

## Validate

```bash
make check
```

## Notes

Full build requires a Linux kernel build tree; this macOS staging pass uses static source validation only.
