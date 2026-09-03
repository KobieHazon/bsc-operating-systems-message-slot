#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define MAJOR_NUM 240

#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int)

#define DEVICE_RANGE_NAME "message_slot"
#define BUF_LEN 128
#define SLOTS_NUM 256
#define DEVICE_FILE_NAME "message_slot_dev"

#define SUCCESS 0
#define FAILURE -1

#define INT_TO_VOID(num) (void *)(long)num
#define VOID_TO_INT(ptr) (int)(long)ptr

#endif 
