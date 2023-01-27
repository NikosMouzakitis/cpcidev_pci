/*
 *
 *
 *
 *
 *     Userspace application to be used in conkunction with the 
 *     kernel module driver qemu_custom_device_driver.ko in order
 *     to access the cpcidev_pci driver.
 *
 *     @author: Mouzakitis Nikolaos (2023)
 *
 *
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "chardev.h"

#define BUF_SIZE 1024

//global to printf from read/write functions.
char buf[BUF_SIZE];

//read from the device file.
int read_dev(void *ptr, int no_bytes, int no_items, FILE * fp, int addr)
{
	int rv;

	//read from device file.	
        fseek(fp, addr, 0);		
        rv = fread(ptr, no_bytes, no_items, fp);	

	printf("return value read(bytes):  %d\n ",rv);
	for (int i = 0; i <4; i++)	
		printf("%x ",buf[i]);

	printf("\n");
}

int main()
{
        FILE *fp;
	int rv;
	int op1, op2, opcode, result;

        void *ptr;
	int fd  = open("/dev/cpcidev_pci", O_RDWR);

	printf("userspce program run.\n");
	memset(buf, 0x0, BUF_SIZE);
	ptr = buf;
	if(fd == -1)
	{
		printf("error opening device /open()\n");
		return -1;
	}

	printf("device opened success.\n");
	printf("Perform addition 5+3\n");

	op1 = 5;
	op2 = 3;
	opcode = 1;
	printf("set op1 %x\n",op1);
	ioctl(fd, 8, op1);
	printf("set op2 %x\n",op2);
	ioctl(fd, 9, op2);
	printf("set opcode %x\n",opcode);
	ioctl(fd, 10, opcode);

	printf("ioctl to get op1\n");
	ioctl(fd, 4, &op1); 	
	printf("retrieved op1: %x\n", op1);

	printf("ioctl to get op2\n");
	ioctl(fd, 5, &op2); 	
	printf("retrieved op1: %x\n", op2);

	printf("ioctl to get opcode\n");
	ioctl(fd, 6, &opcode); 	
	printf("retrieved opcode: %x\n", opcode);
	
	//perform the addition.
	//read result.
	
	ioctl(fd, 7, &result);
	printf("retrived result: %x\n",result);

	printf("Perform 5-3\n");
	opcode = 2;
	ioctl(fd, 10, opcode);

	ioctl(fd, 7, &result);
	printf("retrived result: %x\n",result);
	
	printf("Perform 5*3\n");
	opcode = 3;
	ioctl(fd, 10, opcode);
	ioctl(fd, 7, &result);
	printf("retrived result: %x\n",result);
	
	
	close(fd);

	return 0;

}
