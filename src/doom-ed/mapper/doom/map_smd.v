
`include "../base/defs.v"

module map_smd(

	output [`BW_MAP_OUT-1:0]mapout,
	input [`BW_MAP_IN-1:0]mapin,
	input clk
);
	
	`include "../base/mapio.v"
	`include "../base/sys_cfg.v"
	`include "../base/pi_bus.v"

	
	assign mask_off = 1;
	assign dtack = 1;
	assign pi_din_map[7:0] = 8'hff;//pi_din_map can be readed via usb (64K area at 0x1830000 in pi address space)
//*************************************************************************************

	//mapper dout
	assign map_oe        = (rom_ce & !oe_as) | (regs_ce & !oe);
	
	assign map_do[15:0]  = 
	spot_ce & spot_addr[0] == 0 ? {mem_do[`ROM0][15:8], mem_do[`ROM0][15:8]} :
	spot_ce & spot_addr[0] == 1 ? {mem_do[`ROM0][7:0], mem_do[`ROM0][7:0]}   :
	colu_ce & colu_addr[0] == 0 ? {mem_do[`ROM0][15:8], mem_do[`ROM0][15:8]} :
	colu_ce & colu_addr[0] == 1 ? {mem_do[`ROM0][7:0], mem_do[`ROM0][7:0]}   :
	
	regs_ce & cpu_addr[7:0] == 8'h84 ? {eswap[7:0], eswap[15:8]} :
	regs_ce & cpu_addr[7:0] == 8'h86 ? {eswap[23:16], eswap[31:24]} :
	
	regs_ce & cpu_addr[7:0] == 8'h90 ? mul_res[47:32]  : 
	regs_ce & cpu_addr[7:0] == 8'h92 ? mul_res[31:16]  :
	regs_ce & cpu_addr[7:0] == 8'h94 ? mul_res[15:0]   :
	
	regs_ce & cpu_addr[7:0] == 8'h98 ? div_res[31:16]  : 
	regs_ce & cpu_addr[7:0] == 8'h9A ? div_res[15:0]   : 
	
	regs_ce & cpu_addr[7:0] == 8'hB8 ? shifl[31:16]  :
	regs_ce & cpu_addr[7:0] == 8'hBA ? shifl[15:0]   :
	
	regs_ce & cpu_addr[7:0] == 8'hBC ? shifr[31:16]  :
	regs_ce & cpu_addr[7:0] == 8'hBE ? shifr[15:0]   :
	
	fbuf_cycle	? fbuf_do[15:0] : //palette can be injected here
	mem_do[`ROM0][15:0];

	//memory
	assign mem_addr[`ROM0][22:0] = 
	spot_ce ? spot_addr[22:0] : 
	fbuf_ce ? fbuf_addr[22:0] : 
	colu_ce ? colu_addr[22:0] : 
	cpu_addr[22:0];//rom address
	
	assign mem_di[`ROM0][15:0]   = cpu_data[15:0];
	
	assign mem_oe   [`ROM0]		  = (rom_ce & !oe_as) | fbuf_cycle | (spot_ce & !oe_as) | (colu_ce & !oe_as);
	assign mem_we_lo[`ROM0]      = (rom_ce & !we_lo) | colu_we_lo;
	assign mem_we_hi[`ROM0]      = (rom_ce & !we_hi) | colu_we_hi;
	
	wire rom_ce = !ce_hi;
	
//*********************************	
	wire regs_ce = !tim;
	wire reg_we  = regs_ce & !we_lo;
	wire reg_we_sync = {reg_we_st, reg_we} == 2'b01;
	
	
	//bitmap transformation
	wire fbuf_ce = {cpu_addr[22:16], 16'd0} == 24'h70000;
	wire fbuf_oe = rom_ce & fbuf_ce & !oe;
	wire fbuf_oe_sync =  {fbuf_oe_st, fbuf_oe} == 2'b01;
	wire fbuf_cycle = fbuf_oe_ctr <= 8 | fbuf_oe;
	
	//planes render
	wire [22:0]spot_addr = spot + fbuf_addr_s;//[22:0];
	wire spot_ce  = regs_ce & {cpu_addr[7:2], 2'd0} == 8'hA0;
	wire spot_oe  = spot_ce & !oe;
	wire spot_inc = {spot_oe_st, spot_oe} == 2'b10;
	
	//walls render (columns)
	wire [22:0]colu_addr_r = fbuf_addr_s + ((xfrac >> 16) & 127);
	wire [22:0]colu_addr_w = yfrac;
	wire [22:0]colu_addr = !oe ? colu_addr_r : colu_addr_w;
	wire colu_ce  = regs_ce & cpu_addr[7:0]== 8'hB0;
	wire colu_we_hi = colu_ce & !we_lo & colu_addr[0] == 0;
	wire colu_we_lo = colu_ce & !we_lo & colu_addr[0] == 1;
	wire colu_we    = colu_we_hi | colu_we_lo;
	wire colu_oe    = colu_ce & !oe;
	wire colu_oe_sync = {colu_oe_st, colu_oe} == 2'b10;
	wire colu_we_sync = {colu_we_st, colu_we} == 2'b10;
	
	//bitmap transform
	wire [15:0]fbuf_addr_int = {cpu_addr[14:1], 2'b00};
	wire [2:0]tx = fbuf_addr_int[2:0];			//tile x
	wire [2:0]ty = fbuf_addr_int[5:3];			//tile y
	wire [5:0]co = fbuf_addr_int[15:6] % 32;	//column
	wire [4:0]ro = fbuf_addr_int[15:6] / 32;	//row
	
	reg [23:0]fbuf_addr_s;
	reg [22:0]fbuf_addr;
	reg [15:0]fbuf_do;
	reg [15:0]fbuf_do_buff;
	reg [3:0]fbuf_oe_ctr;
	
	reg fbuf_oe_st;
	reg reg_we_st;
	reg spot_oe_st;
	reg colu_oe_st;
	reg colu_we_st;
	
	//math
	reg signed [31:0]mul_a;
	reg signed [31:0]mul_b;
	reg signed [47:0]mul_res;
	reg mul_exec;
	
	reg signed [31:0]div_a;
	reg signed [31:0]div_b;
	reg signed [31:0]div_res;
	reg div_exec;
	
	reg [31:0]eswap;
	
	//render counters
	reg signed [31:0]xstep;//also src address for column
	reg signed [31:0]xfrac;//also dst address for column
	reg signed [31:0]ystep;
	reg signed [31:0]yfrac;
	
	reg signed [31:0]shifv;
	reg signed [4:0]shifx;
	
	wire signed [31:0]shifl = shifv << shifx;
	wire signed [31:0]shifr = shifv >> shifx;
	
	wire signed [47:0]div_a_xt = div_a[31:0] << 16;//{div_a[31:0], 16'h0000};
	
	wire signed [31:0]spot = (yfrac / 1024 & 63 * 64) + (xfrac / 65536 & 63);
	
	always @(negedge clk)
	begin
		
		reg_we_st  <= reg_we;
		fbuf_oe_st <= fbuf_oe;
		spot_oe_st <= spot_oe;
		colu_oe_st <= colu_oe;
		colu_we_st <= colu_we;
		
//*********************************	bitbap transform
		if(fbuf_oe_sync)
		begin
			fbuf_oe_ctr <= 0;
			fbuf_do <= fbuf_do_buff;
			fbuf_addr[22:0] <= fbuf_addr_s[22:0] + tx + 320 * ty + 8 * co + 2560 * ro;
		end
			else
		if(fbuf_oe_ctr != 15)
		begin
			fbuf_oe_ctr <= fbuf_oe_ctr + 1;
		end
		
		if(fbuf_oe_ctr == 4)
		begin
			fbuf_do_buff[15:8] <= {mem_do[`ROM0][11:8], mem_do[`ROM0][3:0]};
			fbuf_addr <= fbuf_addr + 2;
		end
		
		if(fbuf_oe_ctr == 8)
		begin
			fbuf_do_buff[7:0] <= {mem_do[`ROM0][11:8], mem_do[`ROM0][3:0]};
		end
//*********************************	column/rows counters
		if(spot_inc)
		begin
			xfrac <= xfrac + xstep;
			yfrac <= yfrac + ystep;
		end
		
		
		if(colu_oe_sync)
		begin
			xfrac <= xfrac + xstep;
		end
		
		if(colu_we_sync)
		begin
			yfrac <= yfrac + ystep;
		end
		
//*********************************	control registers
		
		case({reg_we_sync, cpu_addr[7:0]})
		
			9'h180:fbuf_addr_s[23:16] <= cpu_data[7:0];
			9'h182:fbuf_addr_s[15:0]  <= cpu_data[15:0];
			
			9'h184:eswap[31:16] <= cpu_data[15:0];
			9'h186:eswap[15:0]  <= cpu_data[15:0];
			
			9'h190:mul_a[31:16] <= cpu_data[15:0];
			9'h192:mul_a[15:0]  <= cpu_data[15:0];
			9'h194:mul_b[31:16] <= cpu_data[15:0];
			9'h196:mul_b[15:0]  <= cpu_data[15:0];

			
			9'h198:div_a[31:16] <= cpu_data[15:0];
			9'h19A:div_a[15:0]  <= cpu_data[15:0]; //div_a[15:0]  <= cpu_data[15:0];
			9'h19C:div_b[31:16] <= cpu_data[15:0];
			9'h19E:div_b[15:0]  <= cpu_data[15:0];
			
			
			9'h1A0:xstep[31:16] <= cpu_data[15:0];//read returns spot
			9'h1A2:xstep[15:0]  <= cpu_data[15:0];//read returns spot and inc
			9'h1A4:ystep[31:16] <= cpu_data[15:0];
			9'h1A6:ystep[15:0]  <= cpu_data[15:0];
			
			9'h1A8:xfrac[31:16] <= cpu_data[15:0];
			9'h1AA:xfrac[15:0]  <= cpu_data[15:0];
			9'h1AC:yfrac[31:16] <= cpu_data[15:0];
			9'h1AE:yfrac[15:0]  <= cpu_data[15:0];
			
			9'h1B6:shifx[4:0]   <= cpu_data[15:0];
			9'h1B8:shifv[31:16] <= cpu_data[15:0];
			9'h1BA:shifv[15:0]  <= cpu_data[15:0];
			
		endcase 
		
		
		clk_math <= !clk_math;

	end
	
	
	reg clk_math;
	
	always @(negedge clk_math)
	begin
		mul_res <= (mul_a * mul_b);
		if(div_b != 0)div_res <= (div_a_xt / div_b);
	end
	

endmodule

