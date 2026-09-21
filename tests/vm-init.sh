#!/bin/busybox sh
export PATH=/bin:/sbin
finish() { echo VM_TEST_FAILED; poweroff -f; }
trap finish EXIT
mount -t proc proc /proc || exit 1
mount -t sysfs sysfs /sys || exit 1
mount -t devtmpfs devtmpfs /dev || exit 1
insmod /message_slot.ko major_num=0 || exit 1
major=$(/bin/busybox awk '$2 == "message_slot" { print $1 }' /proc/devices)
test -n "$major" || exit 1
mknod /dev/slot0 c "$major" 0 || exit 1
mknod /dev/slot1 c "$major" 1 || exit 1
/device-test || exit 1
/message-sender /dev/slot0 23 command-line-message || exit 1
test "$(/message-reader /dev/slot0 23)" = command-line-message || exit 1
rmmod message_slot || exit 1
insmod /message_slot.ko major_num="$major" || exit 1
/device-test || exit 1
rmmod message_slot || exit 1
echo VM_TEST_PASSED
trap - EXIT
poweroff -f
