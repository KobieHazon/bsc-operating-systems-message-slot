#include "message_slot.h"

#include <fcntl.h>      
#include <unistd.h>     
#include <sys/ioctl.h>  
#include <stdio.h>
#include <stdlib.h>

#define ERR_VAL 1

void handle_error(int fd) {
	perror("Error: ");
	close(fd);
	exit(ERR_VAL);
}

int main(int argc, char *argv[]) {
	int dest_file, ret_val, cnt = 0;
	char* tmp;
	if (argc != 4) {
		printf("Error: message_sender accepts 3 arguments.\n");
		exit(ERR_VAL);
	}
	tmp = argv[3];
	//printf("GOT HERE\n");
	if ((dest_file = open(argv[1], O_WRONLY)) < 0)
		handle_error(dest_file);
	//printf("GOT HERE\n");
	if ((ret_val = ioctl(dest_file, MSG_SLOT_CHANNEL, atoi(argv[2]))) < 0)
		handle_error(dest_file);
	//printf("GOT HERE\n");
	for (cnt = 0; tmp[0] != '\0'; cnt++, tmp++);
	//printf("GOT HERE\n");
	if ((ret_val = write(dest_file, argv[3], cnt)) < 0)
		handle_error(dest_file);
	//printf("GOT HERE\n");
	if ((ret_val = close(dest_file)) < 0)
		handle_error(dest_file);
	//printf("GOT HERE\n");
	exit(0);
}
