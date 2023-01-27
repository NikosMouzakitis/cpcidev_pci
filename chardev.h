#ifndef CHARDEV_H
#define CHARDEV_H

#include <linux/ioctl.h>

/* The major device number. We can't rely on dynamic 
 * registration any more, because ioctls need to know 
 * it. */
#define MAJOR_NUM 251

/* Set the message of the device driver */
#define IOCTL_SET_MSG _IOR(MAJOR_NUM, 0, char *)
#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, int)
#define IOCTL_GET_NTH_BYTE _IOWR(MAJOR_NUM, 2, int)

#define IOCTL_GET_OP1 _IOR(MAJOR_NUM, 4, int *)
#define IOCTL_GET_OP2 _IOR(MAJOR_NUM, 5, int *)
#define IOCTL_GET_OPCODE _IOR(MAJOR_NUM, 6, int *)
#define IOCTL_GET_RESULT _IOR(MAJOR_NUM, 7, int *)

#define IOCTL_SET_OP1 _IOW(MAJOR_NUM, 8, int)
#define IOCTL_SET_OP2 _IOW(MAJOR_NUM, 9, int )
#define IOCTL_SET_OPCODE _IOW(MAJOR_NUM, 10, int )


#define DEVICE_FILE_NAME "cpcidev_pci"

#endif

