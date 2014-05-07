#ifndef _VISUALIZER_H
#define _VISUALIZER_H

#include <linux/ioctl.h>

/* ioctls and their arguments */
#define VISUALIZER_WRITE_DIGIT _IOW(VISUALIZER_MAGIC, 1, u16 *)
#define VISUALIZER_READ_DIGIT  _IOWR(VISUALIZER_MAGIC, 2, u16 *)

#endif
