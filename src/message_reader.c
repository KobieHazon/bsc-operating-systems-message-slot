#include "message_slot.h"

#include <fcntl.h>      
#include <unistd.h>     
#include <sys/ioctl.h>  
#include <stdio.h>
#include <stdlib.h>

#define ERR_VAL 1
#define STDOUT 1

void handle_error(int fd) {
	perror("Error: ");
	close(fd);
	exit(ERR_VAL);
}

int main(int argc, char *argv[]) {
	int dest_file, ret_val, cnt;
	char buffer[BUF_LEN + 1];
	buffer[BUF_LEN] = '\0';
	if (argc != 3) {
		printf("Error: message_sender accepts 2 arguments.\n");
		exit(ERR_VAL);
	}
	if ((dest_file = open(argv[1], O_RDONLY)) < 0)
		handle_error(dest_file);
	if ((ret_val = ioctl(dest_file, MSG_SLOT_CHANNEL, atoi(argv[2]))) < 0)
		handle_error(dest_file);
	if ((cnt = read(dest_file, &buffer, BUF_LEN)) < 0)
		handle_error(dest_file);
	if ((ret_val = close(dest_file)) < 0)
		handle_error(dest_file);
	if ((ret_val = write(STDOUT_FILENO, buffer, cnt)) < 0)
		handle_error(-1);
	exit(0);
}

