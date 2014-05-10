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
  
  /* Draw 12 bands at distinct frequencies as well as the dials in the middle of the bands */
  for (i = 1 ; i < 13 ; i++) {
    for (row = 470; row > 370; row--){
	    fbputchar('I', row, COL_NUM(i));
    }
    updatedial(420, COL_NUM(i)-1)
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
      //// Escape
      if (packet.keycode[0] == 0x29) { /* ESC pressed? */
        int retToZ = 0;
        do {
            libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 0);
			retToZ = packet.modifiers == 0 && packet.keycode[0] == 0;
	    } while (!retToZ);
	    break;
      }
      //// Enter
      if (packet.keycode[0] == 0x28) {
        int retToZ = 0;
        do {
            libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 0);
			retToZ = packet.modifiers == 0 && packet.keycode[0] == 0;
	    } while (!retToZ);
        send_input(input, sockfd);
        strcpy(echo, "<Me> ");
        strcat(echo, input);
        fbputpacket(echo, &top_line_pos);
        memset(input, 0, INPUT_SIZE);
        keyRow = MAX_SCREEN_Y + 1;
        keyCol = 0;
        fbclearlines(MAX_SCREEN_Y + 1, MAX_SCREEN_Y + 3);
        continue;
      }
      //// Left
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
      //// Right
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
      //// Backspace (2a) / Delete (4c)
      if (packet.keycode[0] == 0x2a || packet.keycode[0] == 0x4c) {
        int retToZ = 0;
	    int pos = (keyRow - (MAX_SCREEN_Y + 1)) * 128 + keyCol;
	    if (packet.keycode[0] == 0x2a) {
    	    if (!(keyRow == MAX_SCREEN_Y + 1 && keyCol == 0)) {
	            pos--;
        	    if (keyCol == 0) {
        	        keyRow--;
        	        keyCol = 127;
        	    } else
        	        keyCol--;
    	    } else {
                do {
                    libusb_interrupt_transfer(keyboard, endpoint_address,
        			      (unsigned char *) &packet, sizeof(packet),
        			      &transferred, 100);
        			retToZ = packet.modifiers == 0 && packet.keycode[0] == 0;
        	    } while (!retToZ);
        	    continue;
    	    }
	    }
        do {
            libusb_interrupt_transfer(keyboard, endpoint_address,
			      (unsigned char *) &packet, sizeof(packet),
			      &transferred, 100);
			retToZ = packet.modifiers == 0 && packet.keycode[0] == 0;
	    } while (!retToZ);
	    printf("pos: %d\n", pos);
	    clear_pos(pos, input);
        print_input(input, NULL, NULL);
        continue;
      }
      
      // Parse the key being pressed
      int offset; 
      int dec = 0;
      int retToZ = 0;
      do {
      if( packet.keycode[0] >= 0x04 && packet.keycode[0] <= 0x1D){ /* shift letters */
    	offset = packet.keycode[0] - 0x04;
    	if( packet.modifiers == 0x02 || packet.modifiers == 0x20){
    		offset  = offset - 0x20; 
    	}
    
    	//printf("offset: %x %d\n", packet.keycode[0], offset);
    	dec =97 + offset; 
    	
    	
      }
      else if( packet.keycode[0] >= 0x1E && packet.keycode[0]<= 0x22){ /* shift 1, 2, 3, 4, 5*/
    	offset = packet.keycode[0] - 0x1E;
    	if( packet.modifiers == 0x02 || packet.modifiers == 0x20){
    		offset = offset - 0x10; 
    	}
    	
    	//printf("offset: %d", offset);
    	dec = 49 + offset;
    	
    	if ((packet.modifiers == 0x02 || packet.modifiers == 0x20) && packet.keycode[0] == 0x1f)
    	    dec = 64;
      }
      else if( packet.keycode[0] >= 0x24 && packet.keycode[0] <=0x26){ /* 7, 8, 9*/
    	offset = packet.keycode[0] - 0x24;
    	if( packet.modifiers == 0x02 || packet.modifiers == 0x20){
    		offset = offset - 0x11; 
    	}
    	
    	//printf("offset: %d", offset);
    	dec = 55 + offset; 
      }
      else if( packet.keycode[0] == 0x23){ /* 6*/
    	dec = 54; 
    	if( packet.modifiers == 0x02 || packet.modifiers == 0x20){
    		dec = 94;  
    	}
    	
    	
      }
      else if( packet.keycode[0] == 0x2c) // space
        dec=32;
      else if( packet.keycode[0] == 0x27){ /* shift 0 */
    	dec = 48; 
    	if( packet.modifiers == 0x02 || packet.modifiers == 0x20){
    		dec = 41; 
    	}
      }
      else if( packet.keycode[0] >= 0x2F && packet.keycode[0] <= 0x31){ /* [ ] \  */
    	offset = packet.keycode[0] - 0x2F; 
    	if( packet.modifiers == 0x02 || packet.modifiers == 0x20){
    		offset = offset + 0x20; 
    	}
    	dec = 91 + offset; 
    	if (packet.keycode[0] == 0x31)
    	    dec--;
    	else if (packet.keycode[0] == 0x30)
    	    dec++;
      }
      else if( packet.keycode[0]  == 0x37 || packet.keycode[0] == 0x38){ /* . / */
    	offset = packet.keycode[0] - 0x37; 
    	if( packet.modifiers == 0x02 || packet.modifiers == 0x20){
    		offset = offset + 16; 
    	}
    	dec = 46 + offset; 
      }
      
      else if( packet.keycode[0] == 0x36){ /* < */
    	dec = 44; 
    	if( packet.modifiers == 0x02 || packet.modifiers == 0x20){
    		dec = 60; 
    	}
      }
      
      else if( packet.keycode[0] == 0x34){ /* " */
    	dec = 39; 
    	if( packet.modifiers == 0x02 || packet.modifiers == 0x20){
    		dec = 34; 
    	}
      }
      else if( packet.keycode[0] == 0x33){ /* ; */
    	dec = 59; 
    	if( packet.modifiers == 0x02 || packet.modifiers == 0x20){
    		dec = 58; 
    	}
      }
      else if (packet.keycode[0] == 0x35) {
            dec = 96;
            if (packet.modifiers == 0x02 || packet.modifiers == 0x20)
                dec = 126;
        }

        else if (packet.keycode[0] == 0x2d) {
            dec = 45;
            if (packet.modifiers == 0x02 || packet.modifiers == 0x20)
                dec = 95;
        }

        else if (packet.keycode[0] == 0x2e) {
            dec = 61;
            if (packet.modifiers == 0x02 || packet.modifiers == 0x20)
                dec = 43;
        }
        libusb_interrupt_transfer(keyboard, endpoint_address,
		      (unsigned char *) &packet, sizeof(packet),
		      &transferred, 100);
		retToZ = packet.keycode[0] == 0;
      } while (!retToZ);
      char cdec = (char)(dec);
      
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

  /* Terminate the network thread */
  pthread_cancel(network_thread);

  /* Wait for the network thread to finish */
  pthread_join(network_thread, NULL);

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

void send_input(char *input, int sockfd)
{
    int len = strlen(input);
    write(sockfd, input, len);
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

void *network_thread_f(void *ignored)
{
  char recvBuf[BUFFER_SIZE] = {0};
  int n;
  /* Receive data */
  while ( (n = read(sockfd, &recvBuf, BUFFER_SIZE - 1)) > 0 ) {
    recvBuf[n] = '\0';
    printf("%s", recvBuf);
    fbputpacket(recvBuf, &top_line_pos); //wrapping here for screen
    memset(recvBuf, 0, BUFFER_SIZE);
  }

  return NULL;
}

