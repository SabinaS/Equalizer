#include "fbputchar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "usbkeyboard.h"
#include <pthread.h>

#define IPADDR(a,b,c,d) (htonl(((a)<<24)|((b)<<16)|((c)<<8)|(d)))
#define SERVER_HOST IPADDR(192,168,1,1)
#define SERVER_PORT htons(42000)

#define BUFFER_SIZE 128
#define INPUT_SIZE 2 * 128 + 1
#define ROW_1 45
#define ROW_2 46
#define COL_NUM(int col){ 50*(col-1)+30 }

// By Mark Aligbe (ma2799) and Sabina Smajlaj (ss3912)

/*
 * References:
 *
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 * http://www.thegeekstuff.com/2011/12/c-socket-programming/
 * 
 */


struct libusb_device_handle *keyboard;
uint8_t endpoint_address;

void print_input(char *, int *, int *);
void send_input(char *, int);
void clear_pos(int, char *);

int top_line_pos = 2;

int main()
{
  int err, col, row, i;

  struct usb_keyboard_packet packet;
  int transferred;

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
    updatedial(420, COL_NUM(i))
  }


  /* Open the keyboard */
  if ( (keyboard = openkeyboard(&endpoint_address)) == NULL ) {
    fprintf(stderr, "Did not find a keyboard\n");
    exit(1);
  }
    

  // The position of the cursor
  int keyRow = MAX_SCREEN_Y + 1, keyCol = 0;
  // The array holding the user input
  char input[INPUT_SIZE] = {0};
  char echo[INPUT_SIZE + 5] = {0};
  // The cursor is visible if this is 1, invisible if -1, and X otherwise
  int cursorState = 1;
  
  char keycode[128] = {0};
  // Display the cursor
  fbputchar(95, keyRow, keyCol);

  /* Look for and handle keypresses */
  for (;;) {
    // Blink the cursor
    if (libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 200) == LIBUSB_ERROR_TIMEOUT) {
        if (cursorState == 1) {
            fbputchar(input[(keyRow - (MAX_SCREEN_Y + 1)) * 128 + keyCol] == 0 ? 32 : input[(keyRow - (MAX_SCREEN_Y + 1)) * 128 + keyCol], keyRow, keyCol);
            cursorState = -1;
        } else if (cursorState == -1) {
            fbputchar(95, keyRow, keyCol);
            cursorState = 1;
        }
        continue;
    }
    //Put cursor at top initially 
    if (cursorState == 1) {
        fbputchar(input[(keyRow - (MAX_SCREEN_Y + 1)) * 128 + keyCol] == 0 ? 32 : input[(keyRow - (MAX_SCREEN_Y + 1)) * 128 + keyCol], keyRow, keyCol);
        cursorState = -1;
    } else if (cursorState == -1) {
        fbputchar(95, keyRow, keyCol);
        cursorState = 1;
    }
    
    // Parse the packet
    if (transferred == sizeof(packet)) {
      // Check for special keys first
      
      //// LeftArrow
      if (packet.keycode[0] == 0x50) {
        int retToZ = 0;
        do {
            libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 100);
			retToZ = packet.modifiers == 0 && packet.keycode[0] == 0;
	    } while (!retToZ);
	    if (keyRow == MAX_SCREEN_Y + 1 && keyCol == 0)
	        continue;
	    if (keyCol == 0) {
	        keyRow--;
	        keyCol = 127;
	    } else
	        keyCol--;
        print_input(input, NULL, NULL);
        continue;
      }
      //// RightArrow
      if (packet.keycode[0] == 0x4f) {
        int retToZ = 0;
        do {
            libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 100);
			retToZ = packet.modifiers == 0 && packet.keycode[0] == 0;
	    } while (!retToZ);
	    if (keyRow == MAX_SCREEN_Y + 2 && keyCol == 127)
	        continue;
	    if (keyCol == 127) {
	        keyRow++;
	        keyCol = 0;
	    } else
	        keyCol++;
        print_input(input, NULL, NULL);
        continue;
      }
      //// DownArrow
      if (packet.keycode[0] == 0x51) {
        int retToZ = 0;
        do {
            libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 100);
			retToZ = packet.modifiers == 0 && packet.keycode[0] == 0;
	    } while (!retToZ);
	    if (keyRow == MAX_SCREEN_Y + 1 && keyCol == 0)
	        continue;
	    if (keyCol == 0) {
	        keyRow--;
	        keyCol = 127;
	    } else
	        keyCol--;
        print_input(input, NULL, NULL);
        continue;
      }
      //// UpArrow
      if (packet.keycode[0] == 0x52) {
        int retToZ = 0;
        do {
            libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 100);
			retToZ = packet.modifiers == 0 && packet.keycode[0] == 0;
	    } while (!retToZ);
	    if (keyRow == MAX_SCREEN_Y + 1 && keyCol == 0)
	        continue;
	    if (keyCol == 0) {
	        keyRow--;
	        keyCol = 127;
	    } else
	        keyCol--;
        print_input(input, NULL, NULL);
        continue;
      }
     
      
      // Print the line
      sprintf(keycode, "%c", cdec);
      int inBeginSize = (keyRow - (MAX_SCREEN_Y + 1)) * 128 + keyCol + 1;
      int inEndSize = INPUT_SIZE - ((keyRow - (MAX_SCREEN_Y + 1)) * 128 + keyCol);
      char inBegin[inBeginSize];
      char inEnd[inEndSize];
      memset(inBegin, 0, inBeginSize);
      memset(inEnd, 0, inEndSize);
      printf("inBegin: %d, inEnd: %d\n", sizeof(inBegin), sizeof(inEnd));
      strncpy(inBegin, input, inBeginSize - 1);
      strcpy(inEnd, keycode);
      strncat(inEnd, input + (inBeginSize - 1), inEndSize - 1);
      memset(input, 0, INPUT_SIZE);
      strcpy(input, inBegin);
      strcat(input, inEnd);
      //strncat(input, keycode, INPUT_SIZE - strlen(input) - 1);
      printf("input len: %d\n", strlen(input));
      if (!(keyRow == MAX_SCREEN_Y + 2 && keyCol == 127)) {
        if (keyCol == 127) {
            keyRow++;
            keyCol = 0;
        } else
            keyCol++;
      }
      print_input(input, NULL, NULL);
    }
  }

  return 0;
}

void clear_pos(int pos, char *buf)
{
    int len = strlen(buf);
    printf("len as seen from clear_pos %d\n", len);
    if (pos >= len) return;
    int overwritten = 0;
    for (; pos < len; pos++) {
        buf[pos] = buf[pos + 1];
        overwritten = 1;
    }
    printf("buf: %s\n", buf);
    if (!overwritten)
        buf[pos] = 0;
}

void print_input(char *input, int *cRow, int *cCol)
{
    fbclearlines(MAX_SCREEN_Y + 1, MAX_SCREEN_Y + MAX_INPUT_HEIGHT + 1);
    int col = 0, row = 0;
    while (*input != 0) {
        if (col == 128 && row < 1) {
            col = 0;
            row++;
        }
        fbputchar(*input, row + MAX_SCREEN_Y + 1, col++);
        input++;
    }
    if (cRow != NULL)
        *cRow = MAX_SCREEN_Y + (row > 1 ? 1 : row) + 1;
    if (cCol != NULL)
        *cCol = .,mncol > 127 ? 127 : col;
}


