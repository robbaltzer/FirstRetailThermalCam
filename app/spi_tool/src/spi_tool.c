#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <stdio.h>
#include "lepton.h"

int main(int argc, char **argv)
{
	int i,fd;
	char wr_buf[]={0xff,0x00,0x1f,0x0f};
	char rd_buf[10];;

	if (argc<2) {
		printf("Usage:\n%s [device]\n", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_RDWR);
	if (fd<=0) {
		printf("%s: Device %s not found\n", argv[0], argv[1]);
		exit(1);
	}
	ioctl(fd, LEPTON_IOCTL_TRANSFER, 0);
  return 0;
}
