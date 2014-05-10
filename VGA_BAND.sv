/*
 * memory-mapped peripheral for the VGA BAND Display
 *
 */

module VGA_BAND(input logic      clk,
                input logic          reset,
                input logic [15:0]    writedata,
                input logic          write,
                input                chipselect,
                input logic          address,

                output logic [7:0] VGA_R, VGA_G, VGA_B,
                output logic       VGA_CLK, VGA_HS, VGA_VS, VGA_BLANK_n,
                output logic       VGA_SYNC_n);

    logic [4:0] dial31,
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
                dial20000; 

    VGA_BAND_Display band_display(.clk50(clk), .*);

    always_ff @(posedge clk)
        if (reset) begin
            dial31 <= 10'd412;
            dial72 <= 10'd412;
            dial150 <= 10'd412;
            dial250 <= 10'd412;
            dial440 <= 10'd412; 
            dial630 <= 10'd412;
            dial1000 <= 10'd412;
            dial2500 <= 10'd412;
            dial5000 <= 10'd412;
            dial8000 <= 10'd412;
            dial14000 <= 10'd412;
            dial20000 <= 10'd412;
             
        end else if (chipselect && write)
            case (address)
                dial31 <= writedata;
                dial72 <= writedata;
                dial150 <= writedata;
                dial250 <= writedata;
                dial440 <= writedata; 
                dial630 <= writedata;
                dial1000 <= writedata;
                dial2500 <= writedata;
                dial5000 <= writedata;
                dial8000 <= writedata;
                dial14000 <= writedata;
                dial20000 <= writedata;
            endcase
            
endmodule
