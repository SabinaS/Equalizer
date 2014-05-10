/*
 * should draw bars
 * modifying frequency ranges
 * 20 hz to 20 khz -- 20 bands from -12 db min and + 12 db max 
 * keyboard left and right to choose between bands and up and down to change db values 
 *
 */


module equalizer_values_display(
 input logic         clk50, reset,
 input logic [4:0]          dial31,
                            dial72,
                            dial150,
                            dial250,
                            dial440, 
                            dial630,
                            dial1000,
                            dial2500,
                            dial5000,
                            dial8000,
                            dial14000,
                            dial20000, 
 output logic [7:0] VGA_R, VGA_G, VGA_B,
 output logic         VGA_CLK, VGA_HS, VGA_VS, VGA_BLANK_n, VGA_SYNC_n);

/*
 * 640 X 480 VGA timing for a 50 MHz clock: one pixel every other cycle
 * 
 * HCOUNT 1599 0             1279       1599 0
 *             _______________              ________
 * ___________|    Video      |____________|  Video
 * 
 * 
 * |SYNC| BP |<-- HACTIVE -->|FP|SYNC| BP |<-- HACTIVE
 *       _______________________      _____________
 * |____|       VGA_HS          |____|
 */
   // Parameters for hcount
   parameter HACTIVE      = 11'd 1280,
             HFRONT_PORCH = 11'd 32,
             HSYNC        = 11'd 192,
             HBACK_PORCH  = 11'd 96,   
             HTOTAL       = HACTIVE + HFRONT_PORCH + HSYNC + HBACK_PORCH; // 1600
   
   // Parameters for vcount
   parameter VACTIVE      = 10'd 480,
             VFRONT_PORCH = 10'd 10,
             VSYNC        = 10'd 2,
             VBACK_PORCH  = 10'd 33,
             VTOTAL       = VACTIVE + VFRONT_PORCH + VSYNC + VBACK_PORCH; // 525

   logic [10:0]                 hcount; // Horizontal counter
                                             // Hcount[10:1] indicates pixel column (0-639)
   logic                  endOfLine;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          hcount <= 0;
     else if (endOfLine) hcount <= 0;
     else               hcount <= hcount + 11'd 1;

   assign endOfLine = hcount == HTOTAL - 1;

   // Vertical counter
   logic [9:0]                  vcount;
   logic                  endOfField;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          vcount <= 0;
     else if (endOfLine)
       if (endOfField)   vcount <= 0;
       else              vcount <= vcount + 10'd 1;

   assign endOfField = vcount == VTOTAL - 1;

   // Horizontal sync: from 0x520 to 0x5DF (0x57F)
   // 101 0010 0000 to 101 1101 1111
   assign VGA_HS = !( (hcount[10:8] == 3'b101) & !(hcount[7:5] == 3'b111));
   assign VGA_VS = !( vcount[9:1] == (VACTIVE + VFRONT_PORCH) / 2);

   assign VGA_SYNC_n = 1; // For adding sync to video signals; not used for VGA
   
   // Horizontal active: 0 to 1279     Vertical active: 0 to89
   // 101 0000 0000  1280           01 1110 0000  480
   // 110 0011 1111  1599           10 0000 1100  524
   assign VGA_BLANK_n = !( hcount[10] & (hcount[9] | hcount[8]) ) &
            !( vcount[9] | (vcount[8:5] == 4'b1111) );   

   /* VGA_CLK is 25 MHz
    *             __    __    __
    * clk50    __|  |__|  |__|
    *        
    *             _____       __
    * hcount[0]__|     |_____|
    */
   assign VGA_CLK = hcount[0]; // 25 MHz clock: pixel latched on rising edge
    reg[3:0] y_range; 
    
    logic [8:0]     band31,
                    band72,
                    band150,
                    band250,
                    band440, 
                    band630,
                    band1000,
                    band2500,
                    band5000,
                    band8000,
                    band14000,
                    band20000; 
               
    
   initial begin
                    y_range = 
                    band31 = 10'd370; 
                    band72 = 10'd370; 
                    band150 = 10'd370; 
                    band250 = 10'd370; 
                    band440 = 10'd370;  
                    band630 = 10'd370; 
                    band1000 = 10'd370; 
                    band2500 = 10'd370; 
                    band5000 = 10'd370; 
                    band8000 = 10'd370; 
                    band14000 = 10'd370; 
                    band20000 = 10'd370;  
                    
                    dial31 = 10'd412; 
                    dial72 = 10'd412; 
                    dial150 = 10'd412; 
                    dial250 = 10'd412; 
                    dial440 = 10'd412;  
                    dial630 = 10'd412; 
                    dial1000 = 10'd412; 
                    dial2500 = 10'd412; 
                    dial5000 = 10'd412; 
                    dial8000 = 10'd412; 
                    dial14000 = 10'd412; 
                    dial20000 = 10'd412;  
   
   
   
   always_comb begin
      {VGA_R, VGA_G, VGA_B} = {8'h0, 8'h0, 8'd255)}; // Blue
      
        
	if (vcount[8:0] > 10'd412) begin
		y2 = vcount[8:0] - yCent;
	end else if (vcount[8:0] < 10'd412) begin
		x2 = hcount[10:1] - xCent;
		y2 = vcount[8:0] - yCent;
	end
	if ((x2 * x2) + (y2 * y2) < r2)
		{VGA_R, VGA_G, VGA_B} = {8'd200, 8'h0, 8'h0};
   	end  
   
endmodule // VGA_BALL_Display
