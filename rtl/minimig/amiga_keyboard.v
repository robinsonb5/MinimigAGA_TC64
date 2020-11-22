module amiga_keyboard
(
  input        clk,               // clock
  input        clk7_en,
  input        clk7n_en,
  input        reset,             // reset
  output       kbdrst,            // keyboard reset out
  input        kbddat_i,          // ps2 keyboard data
  input        kbdclk_i,          // ps2 keyboard clock
  output       kbddat_o,          // ps2 keyboard data
  output       kbdclk_o,          // ps2 keyboard clock
  input        keyboard_disabled, // disable keystrokes
  input        kbd_mouse_strobe,
  input        kms_level,
  input  [1:0] kbd_mouse_type,
  input  [7:0] kbd_mouse_data,
  output [7:0] osd_ctrl,          // osd control
  output       _lmb,
  output       _rmb,
  output [5:0] _joy2,
  output       aflock,            // auto fire lock
  output       freeze,            // Action Replay freeze key
  input        _f_led,            // audio filter LED
  input        disk_led,          // floppy disk activity LED
  output [5:0] mou_emu,
  output [5:0] joy_emu,
  input        hrtmon_en,

  output reg   key_strobe,        // parallel data out
  output reg [7:0] key_data,
  input        keyack,

  output reg   kbclk,             // serial data out
  output reg   kbdata
);

wire        keystrobe;
wire  [7:0] keydat;
reg   [7:0] osd_ctrl_reg;
assign osd_ctrl = osd_ctrl_reg;

// serial transmitter
always @(posedge clk) begin
	reg [10:0] txcnt;
	reg  [7:0] txsr;

	if (reset) begin
		kbclk <= 1;
		kbdata <= 1;
	end else if (clk7_en) begin
		// initialize the transmitter when key_strobe detected
		if (key_strobe) begin
			txcnt <= { 3'h7, 8'hff };
			txsr <= key_data;
		end

		// first put the data bit on kbdata
		if (txcnt[7:0] == 8'hff) begin
			kbdata <= txsr[7];
			txsr <= { txsr[6:0], 1'b0 };
		end
		// then a clock pulse follows
		if (txcnt[7:0] == 8'hc0) kbclk <= 0;
		if (txcnt[7:0] == 8'h40) kbclk <= 1;

		if (|txcnt) txcnt <= txcnt - 1'd1;
	end
end

`ifdef MINIMIG_PS2_KEYBOARD

wire freeze_out;
wire keystrobe_ps2;
wire [7:0] osd_ctrl_ps2;
wire osd_ctrl_strobe_ps2;

ciaa_ps2keyboard  kbd1
(
  .clk(clk),
  .clk7_en(clk7_en),
  .reset(reset),
  .ps2kdat_i(kbddat_i),
  .ps2kclk_i(kbdclk_i),
  .ps2kdat_o(kbddat_o),
  .ps2kclk_o(kbdclk_o),
  .leda(~_f_led),     // keyboard joystick LED - num lock
  .ledb(disk_led),    // disk activity LED - scroll lock
  .aflock(aflock),
  .kbdrst(kbdrst),
  .keydat(keydat[7:0]),
  .keystrobe(keystrobe_ps2),
  .keyack(keyack),
  .osd_ctrl(osd_ctrl_ps2),
  .osd_strobe(osd_ctrl_strobe_ps2),
  ._lmb(_lmb),
  ._rmb(_rmb),
  ._joy2(_joy2),
  .freeze(freeze_out),
  .mou_emu(mou_emu),
  .joy_emu(joy_emu)
);

assign freeze = hrtmon_en && freeze_out;

`ifdef MINIMIG_EXTRA_KEYBOARD

reg keystrobe_reg;
assign keystrobe = keystrobe_reg | keystrobe_ps2;

// !!! Amiga receives keycode ONE STEP ROTATED TO THE RIGHT AND INVERTED !!!
always @(posedge clk) begin

	if(osd_ctrl_strobe_ps2)
		osd_ctrl_reg<=osd_ctrl_ps2;
	else if (kbd_mouse_strobe)
		osd_ctrl_reg<=kbd_mouse_data;

	if (reset) begin
		key_data <= 8'h00;
		key_strobe <= 0;
	end else if (clk7_en) begin
		keystrobe_reg<=1'b0;
		if (keystrobe_ps2 & ~keyboard_disabled) begin
			key_data <= ~{keydat[6:0],keydat[7]};
			key_strobe <= 1;
		end else if (kbd_mouse_strobe & ~keyboard_disabled) begin
			keystrobe_reg<=1'b1;
			key_data <= ~{kbd_mouse_data[6:0],kbd_mouse_data[7]};
			key_strobe <= 1;
		end else begin
			key_strobe <= 0;
		end
	end
end

`else

assign keystrobe=keystrobe_ps2;

// sdr register
// !!! Amiga receives keycode ONE STEP ROTATED TO THE RIGHT AND INVERTED !!!
always @(posedge clk) begin
	osd_ctrl_reg<=osd_ctrl_ps2;
	if (reset) begin
		key_data <= 8'h00;
		key_strobe <= 0;
	end else if (clk7_en) begin
		if (keystrobe_ps2 & ~keyboard_disabled) begin
			key_strobe <= 1;
			key_data <= ~{keydat[6:0],keydat[7]};
		end else begin
			key_strobe <= 0;
		end
end
`endif
  
`else
//MiST kbd

assign kbdrst = 1'b0;
assign _lmb = 1'b1;
assign _rmb = 1'b1;
assign _joy2 = 6'b11_1111;
assign joy_emu = 6'b11_1111;
assign mou_emu = 6'b11_1111;
reg freeze_reg=0;
assign freeze = freeze_reg;
assign aflock = 1'b0;

reg keystrobe_reg;
assign keystrobe = keystrobe_reg && ((kbd_mouse_type == 2) || (kbd_mouse_type == 3));


reg kms_levelD;
always @(posedge clk) begin
	if (clk7n_en) begin
		keystrobe_reg <= 0;
		kms_levelD <= kms_level;
		if (kms_level ^ kms_levelD)	keystrobe_reg <= 1;
	end
end

// !!! Amiga receives keycode ONE STEP ROTATED TO THE RIGHT AND INVERTED !!!
always @(posedge clk) begin
	if (reset) begin
		key_strobe <= 0;
		key_data <= 0;
		osd_ctrl_reg <= 0;
	end else if (clk7_en) begin
		key_strobe <= 0;
		if (keystrobe && (kbd_mouse_type == 2) && ~keyboard_disabled) begin
			key_strobe <= 1;
			key_data <= ~{kbd_mouse_data[6:0],kbd_mouse_data[7]};
			if (hrtmon_en && (kbd_mouse_data == 8'h5f))
				freeze_reg <= 1;
			else
				freeze_reg <= 0;
		end
		if(keystrobe && ((kbd_mouse_type == 2) || (kbd_mouse_type == 3)))
			osd_ctrl_reg <= kbd_mouse_data;
	end
end

`endif

endmodule
