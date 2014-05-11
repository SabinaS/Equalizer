/* Creates frequency spectrum visualizer for different audio frequencies
 */ 

#include "fbputchar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "usbkeyboard.h"
#include <pthread.h>

#define COL_NUM(col) 50*(col-1)+30 

//read in audio frequncies 
//register changes in frequencies
//send changes to fbputchar in form of squares?
