#!/bin/busybox sh
export PATH=/bin:/sbin
trap 'poweroff -f' EXIT
mount -t proc proc /proc || exit 1
mount -t sysfs sysfs /sys || exit 1
mount -t devtmpfs devtmpfs /dev || exit 1
insmod /message_slot.ko major_num=0 || exit 1
major=$(/bin/busybox awk '$2 == "message_slot" { print $1 }' /proc/devices)
test -n "$major" || exit 1
mknod /dev/slot0 c "$major" 0 || exit 1
mknod /dev/slot1 c "$major" 1 || exit 1
echo
echo 'Message Slot Linux VM ready. The module is loaded inside this guest only.'
echo 'Try:'
echo '  /message-sender /dev/slot0 23 "hello from the VM"'
echo '  /message-reader /dev/slot0 23; echo'
echo 'Type exit or press Ctrl-D to shut down. Guest state is temporary.'
echo
setsid cttyhack sh -i
rmmod message_slot
