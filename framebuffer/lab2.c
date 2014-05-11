/* Draws equalizer bands and dials and handles keyboard presses 
 */

#include "fbputchar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 128
#define INPUT_SIZE 2 * 128 + 1
#define ROW_1 45
#define ROW_2 46
#define COL_NUM(col) 50*(col-1)+30 

int top_line_pos = 2;
unsigned char CUR_CURSOR_STATE[2]={420, 30}; //{row, col}

// By Mark Aligbe (ma2799) and Sabina Smajlaj (ss3912)

/*
 * References:
 *
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 * http://www.thegeekstuff.com/2011/12/c-socket-programming/
 * 
 */



int main()
{
  int err, col, row, i;


  if ((err = fbopen()) != 0) {
    fprintf(stderr, "Error: Could not open framebuffer: %d\n", err);
    exit(1);
  }
  
  // Clear the screen before starting
  fbclearlines(0, 47);
  
 /* init: Draw 12 bands at distinct frequencies as well as 
  *the dials in the middle of the bands 
  */
  for (i = 1 ; i < 13 ; i++) {
    for (row = 470; row > 370; row--){
	    fbputchar('I', row, COL_NUM(i));
    }
    updatedial(420, COL_NUM(i)); 
  }
  //Initialize current cursor state for first column
  

  
  
  
  // The position of the cursor
  int keyRow = 420, keyCol = 30;
  
  // Display the cursor
  fbputchar(95, keyRow, keyCol);
  return 0; 
} //end main




