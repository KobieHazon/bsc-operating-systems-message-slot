#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "message_slot.h"

#define REQUIRE(x) do { if (!(x)) { perror(#x); exit(1); } } while (0)
int main(void) {
    int fd = open("/dev/slot0", O_RDWR), other = open("/dev/slot1", O_RDWR);
    char buffer[128], message[128];
    REQUIRE(fd >= 0 && other >= 0);
    errno = 0; REQUIRE(read(fd, buffer, sizeof buffer) == -1 && errno == EINVAL);
    errno = 0; REQUIRE(ioctl(fd, MSG_SLOT_CHANNEL, 0) == -1 && errno == EINVAL);
    REQUIRE(ioctl(fd, MSG_SLOT_CHANNEL, 1) == 0);
    errno = 0; REQUIRE(read(fd, buffer, sizeof buffer) == -1 && errno == EWOULDBLOCK);
    REQUIRE(write(fd, "hello", 5) == 5);
    REQUIRE(read(fd, buffer, sizeof buffer) == 5 && memcmp(buffer, "hello", 5) == 0);
    errno = 0; REQUIRE(read(fd, buffer, 2) == -1 && errno == ENOSPC);
    errno = 0; REQUIRE(write(fd, "", 0) == -1 && errno == EMSGSIZE);
    REQUIRE(ioctl(fd, MSG_SLOT_CHANNEL, 2) == 0);
    REQUIRE(write(fd, "second", 6) == 6);
    REQUIRE(ioctl(fd, MSG_SLOT_CHANNEL, 1) == 0);
    REQUIRE(read(fd, buffer, sizeof buffer) == 5 && memcmp(buffer, "hello", 5) == 0);
    REQUIRE(ioctl(other, MSG_SLOT_CHANNEL, 1) == 0);
    errno = 0; REQUIRE(read(other, buffer, sizeof buffer) == -1 && errno == EWOULDBLOCK);
    memset(message, 'x', sizeof message);
    REQUIRE(write(other, message, sizeof message) == sizeof message);
    REQUIRE(read(other, buffer, sizeof buffer) == sizeof buffer);
    REQUIRE(memcmp(buffer, message, sizeof buffer) == 0);
    REQUIRE(close(fd) == 0 && close(other) == 0);
    puts("Device tests passed: channel selection, read/write, channel/minor isolation, boundary lengths, and errors");
    return 0;
}
