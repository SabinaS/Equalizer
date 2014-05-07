#ifndef _FFT_DRIVER_H
#define _FFT_DRIVER_H

#include <linux/ioctl.h>


/* ioctls and their arguments */
#define FFT_DRIVER_WRITE_DIGIT _IOW(FFT_DRIVER_MAGIC, 1, u16 *)
#define FFT_DRIVER_READ_DIGIT  _IOWR(FFT_DRIVER_MAGIC, 2, u16 *)

#endif
