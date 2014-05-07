//takes in 8k samples from fft_driver, equalizes them, passes them back to fft_driver

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "fft_driver.h"

#define FFT_DRIVER_WRITE_DIGIT _IOW(FFT_DRIVER_MAGIC, 1, u16 *)
#define FFT_DRIVER_READ_DIGIT  _IOWR(FFT_DRIVER_MAGIC, 2, u16 *)

#define DRIVER_NAME "fft_driver"
#define SAMPLENUM 8192
#define SAMPLEBYTES SAMPLENUM*2

int fp; 

void read_samples()
{
    u16 *dataArray = kmalloc(SAMPLEBYTES, GFP_KERNEL); //allocating space for data array 
    
    if (ioctl(fd, FFT_DRIVER_READ_DIGIT, dataArray) == -1)
        printf("FFT_DRIVER_READ_DIGIT failed: %s\n",
            strerror(errno));
    else {
        if (status & FFT_DRIVER_READ_DIGIT)
            puts("FFT_DRIVER_READ_DIGIT is not set");
        else
            puts("FFT_DRIVER_READ_DIGIT is set");
    }
    
}

void write_samples(u16* dataArray)
{
    if (ioctl(fd, FFT_DRIVER_WRITE_DIGIT, dataArray) == -1)
        printf("FFT_DRIVER_WRITE_DIGIT failed: %s\n",
            strerror(errno));
    else {
        if (status & FFT_DRIVER_READ_DIGIT)
            puts("FFT_DRIVER_WRITE_DIGIT is not set");
        else
            puts("FFT_DRIVER_WRITE_DIGIT is set");
    }
}

int main()
{
    
    static const char *filename = "/dev/vga_ball"; //which file?
    u16 *dataArray = kmalloc(SAMPLEBYTES, GFP_KERNEL); //allocating space for data array
    
    if ((fp = open(filename, O_RDWR)) == -1) {
        fprintf(stderr, "could not open %s\n", filename);
        return -1;
    }
    
    read_samples(); //reading in 32k samples 
    
    //equalize samples 
    fft(dataArray); 
    //0 to SAMPLENUM; get values from hardware
    //STD_LOGIC_VECTOR(WIDTH_EXPONENT-1 DOWNTO 0)
    
    write_samples(dataArray); //writing the 32k samples 
    
    close(fd);
    printf("Equalizer Userspace program terminating\n");
    return 0;
}
