#ifndef _EQUALIZER_H
#define _EQUALIZER_H

#include <linux/ioctl.h>


/* ioctls and their arguments */
#define EQUALIZER_WRITE_DIGIT _IOW(EQUALIZER_MAGIC, 1, equalizer_arg_t *)
#define EQUALIZER_READ_DIGIT  _IOWR(EQUALIZER_MAGIC, 2, equalizer_arg_t *)

#endif
