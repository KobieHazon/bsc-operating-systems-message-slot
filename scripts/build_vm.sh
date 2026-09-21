#!/bin/sh
set -eu
kernel=6.1.0-53-arm64
mkdir -p /tmp/module /tmp/guest/bin /tmp/guest/sbin /tmp/guest/proc /tmp/guest/sys /tmp/guest/dev
cp /project/src/* /tmp/module/
cp /tmp/module/Makefile.kernel /tmp/module/Makefile
make -C /usr/src/linux-headers-$kernel M=/tmp/module modules
cp /tmp/module/message_slot.ko /tmp/guest/
gcc -static -Wall -Wextra -I/project/src /project/tests/device_test.c -o /tmp/guest/device-test
gcc -static /project/src/message_sender.c -o /tmp/guest/message-sender
gcc -static /project/src/message_reader.c -o /tmp/guest/message-reader
cp /bin/busybox /tmp/guest/bin/
for app in sh mount insmod rmmod mknod poweroff setsid cttyhack ls cat echo dmesg; do ln -s /bin/busybox /tmp/guest/bin/$app; done
case "${VM_INIT:-tests/vm-init.sh}" in
    tests/vm-init.sh|scripts/vm-init.sh) ;;
    *) echo "Unsupported guest init script" >&2; exit 1 ;;
esac
cp "/project/${VM_INIT:-tests/vm-init.sh}" /tmp/guest/init
chmod +x /tmp/guest/init
cp /boot/vmlinuz-$kernel /output/kernel
cd /tmp/guest
find . -print0 | cpio --null -o --format=newc | gzip -9 > /output/initramfs.gz
