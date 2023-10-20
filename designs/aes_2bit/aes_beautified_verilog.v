//
//   Copyright 2018 Hannes Gross, SGS Digital Trust Services GmbH
//   Copyright 2018 Ko Stoffelen, Digital Security Group, Radboud University
//   Copyright 2018 Lauren De Meyer, Imec - COSIC, KU Leuven
//   Copyright 2023 Vedad Hadžić, Graz University of Technology
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//


module aes_top (
	pt_in,
	key_in,
	m0,
	m1,
	ct_out
);
	input [127:0] pt_in;
	input [127:0] key_in;
	input m0;
	input m1;
	output [127:0] ct_out;
	wire [127:0] round_key0;
	wire [127:0] round_key1;
	wire [127:0] round_key2;
	wire [127:0] round_key3;
	wire [127:0] round_key4;
	wire [127:0] round_key5;
	wire [127:0] round_key6;
	wire [127:0] round_key7;
	wire [127:0] round_key8;
	wire [127:0] round_key9;
	wire [127:0] state_in0;
	wire [127:0] state_in1;
	wire [127:0] state_in2;
	wire [127:0] state_in3;
	wire [127:0] state_in4;
	wire [127:0] state_in5;
	wire [127:0] state_in6;
	wire [127:0] state_in7;
	wire [127:0] state_in8;
	wire [127:0] state_in9;
	pre_round pre_round_inst(
		.state_in(pt_in),
		.key_in(key_in),
		.m0(m0),
		.m1(m1),
		.key_out(round_key0),
		.state_out(state_in0)
	);
	round round1(
		.state_in(state_in0),
		.key_in(round_key0),
		.m0(m0),
		.m1(m1),
		.rcon_in(8'h02),
		.key_out(round_key1),
		.state_out(state_in1)
	);
	round round2(
		.state_in(state_in1),
		.key_in(round_key1),
		.m0(m0),
		.m1(m1),
		.rcon_in(8'h04),
		.key_out(round_key2),
		.state_out(state_in2)
	);
	round round3(
		.state_in(state_in2),
		.key_in(round_key2),
		.m0(m0),
		.m1(m1),
		.rcon_in(8'h08),
		.key_out(round_key3),
		.state_out(state_in3)
	);
	round round4(
		.state_in(state_in3),
		.key_in(round_key3),
		.m0(m0),
		.m1(m1),
		.rcon_in(8'h10),
		.key_out(round_key4),
		.state_out(state_in4)
	);
	round round5(
		.state_in(state_in4),
		.key_in(round_key4),
		.m0(m0),
		.m1(m1),
		.rcon_in(8'h20),
		.key_out(round_key5),
		.state_out(state_in5)
	);
	round round6(
		.state_in(state_in5),
		.key_in(round_key5),
		.m0(m0),
		.m1(m1),
		.rcon_in(8'h40),
		.key_out(round_key6),
		.state_out(state_in6)
	);
	round round7(
		.state_in(state_in6),
		.key_in(round_key6),
		.m0(m0),
		.m1(m1),
		.rcon_in(8'h80),
		.key_out(round_key7),
		.state_out(state_in7)
	);
	round round8(
		.state_in(state_in7),
		.key_in(round_key7),
		.m0(m0),
		.m1(m1),
		.rcon_in(8'h1b),
		.key_out(round_key8),
		.state_out(state_in8)
	);
	round round9(
		.state_in(state_in8),
		.key_in(round_key8),
		.m0(m0),
		.m1(m1),
		.rcon_in(8'h36),
		.key_out(round_key9),
		.state_out(state_in9)
	);
	final_round round10(
		.state_in(state_in9),
		.key_in(round_key9),
		.m0(m0),
		.m1(m1),
		.state_out(ct_out)
	);
endmodule
module pre_round (
	state_in,
	key_in,
	m0,
	m1,
	state_out,
	key_out
);
	input [127:0] state_in;
	input [127:0] key_in;
	input m0;
	input m1;
	output [127:0] state_out;
	output [127:0] key_out;
	addroundkey_trans addroundkey_trans0(
		.state_in(state_in),
		.key_in(key_in),
		.m0(m0),
		.m1(m1),
		.state_out(state_out)
	);
	key_schedule key_schedule_1(
		.key_in(key_in),
		.rcon_in(8'h01),
		.m0(m0),
		.m1(m1),
		.key_out(key_out)
	);
endmodule
module round (
	state_in,
	key_in,
	m0,
	m1,
	rcon_in,
	state_out,
	key_out
);
	input [127:0] state_in;
	input [127:0] key_in;
	input m0;
	input m1;
	input [7:0] rcon_in;
	output [127:0] state_out;
	output [127:0] key_out;
	wire [127:0] sb;
	wire [127:0] sr;
	wire [127:0] mc;
	subbytes_wrapper subbytes_ins(
		.state_in(state_in),
		.m0(m0),
		.m1(m1),
		.state_out(sb)
	);
	shiftrows shiftrows_ins(
		.state_in(sb),
		.state_out(sr)
	);
	genvar i;
	generate
		for (i = 0; i < 4; i = i + 1) begin : gen_code_label
			mixcolumns_wrapper mixcolumns_rowi(
				.a0(sr[(15 - i) * 8+:8]),
				.a1(sr[(15 - (4 + i)) * 8+:8]),
				.a2(sr[(15 - (8 + i)) * 8+:8]),
				.a3(sr[(15 - (12 + i)) * 8+:8]),
				.m0(m0),
				.m1(m1),
				.b0(mc[(15 - i) * 8+:8]),
				.b1(mc[(15 - (4 + i)) * 8+:8]),
				.b2(mc[(15 - (8 + i)) * 8+:8]),
				.b3(mc[(15 - (12 + i)) * 8+:8])
			);
		end
	endgenerate
	addroundkey_trans addroundkey_ins(
		.state_in(mc),
		.key_in(key_in),
		.m0(m0),
		.m1(m1),
		.state_out(state_out)
	);
	key_schedule key_schedule_ins(
		.key_in(key_in),
		.rcon_in(rcon_in),
		.m0(m0),
		.m1(m1),
		.key_out(key_out)
	);
endmodule
module final_round (
	state_in,
	key_in,
	m0,
	m1,
	state_out
);
	input [127:0] state_in;
	input [127:0] key_in;
	input m0;
	input m1;
	output [127:0] state_out;
	wire [127:0] sb;
	wire [127:0] sr;
	subbytes_wrapper subbytes_ins(
		.state_in(state_in),
		.m0(m0),
		.m1(m1),
		.state_out(sb)
	);
	shiftrows shiftrows_ins(
		.state_in(sb),
		.state_out(sr)
	);
	addroundkey_trans addroundkey_ins(
		.state_in(sr),
		.key_in(key_in),
		.m0(m0),
		.m1(m1),
		.state_out(state_out)
	);
endmodule
module shiftrows (
	state_in,
	state_out
);
	input [127:0] state_in;
	output [127:0] state_out;
	assign state_out[120+:8] = state_in[120+:8];
	assign state_out[112+:8] = state_in[112+:8];
	assign state_out[104+:8] = state_in[104+:8];
	assign state_out[96+:8] = state_in[96+:8];
	assign state_out[88+:8] = state_in[80+:8];
	assign state_out[80+:8] = state_in[72+:8];
	assign state_out[72+:8] = state_in[64+:8];
	assign state_out[64+:8] = state_in[88+:8];
	assign state_out[56+:8] = state_in[40+:8];
	assign state_out[48+:8] = state_in[32+:8];
	assign state_out[40+:8] = state_in[56+:8];
	assign state_out[32+:8] = state_in[48+:8];
	assign state_out[24+:8] = state_in[0+:8];
	assign state_out[16+:8] = state_in[24+:8];
	assign state_out[8+:8] = state_in[16+:8];
	assign state_out[0+:8] = state_in[8+:8];
endmodule
module key_schedule (
	key_in,
	rcon_in,
	m0,
	m1,
	key_out
);
	input [127:0] key_in;
	input [7:0] rcon_in;
	input m0;
	input m1;
	output [127:0] key_out;
	wire [31:0] transformed_word;
	wire [31:0] tr_rcon;
	wire [31:0] col0;
	wire [31:0] col1;
	wire [31:0] col2;
	wire [31:0] col3;
	subword subword_ins(
		.key_in({key_in[96+:8], key_in[0+:8], key_in[32+:8], key_in[64+:8]}),
		.m0(m0),
		.m1(m1),
		.key_out(transformed_word)
	);
	assign tr_rcon[24+:8] = transformed_word[24+:8] ^ rcon_in;
	assign tr_rcon[16+:8] = transformed_word[16+:8];
	assign tr_rcon[8+:8] = transformed_word[8+:8];
	assign tr_rcon[0+:8] = transformed_word[0+:8];
	addword addrow_inst0(
		.a_in(tr_rcon),
		.b_in({key_in[24+:8], key_in[56+:8], key_in[88+:8], key_in[120+:8]}),
		.m0(m0),
		.m1(m1),
		.word_out(col0)
	);
	addword addrow_inst1(
		.a_in(col0),
		.b_in({key_in[16+:8], key_in[48+:8], key_in[80+:8], key_in[112+:8]}),
		.m0(m0),
		.m1(m1),
		.word_out(col1)
	);
	addword addrow_inst2(
		.a_in(col1),
		.b_in({key_in[8+:8], key_in[40+:8], key_in[72+:8], key_in[104+:8]}),
		.m0(m0),
		.m1(m1),
		.word_out(col2)
	);
	addword addrow_inst3(
		.a_in(col2),
		.b_in({key_in[0+:8], key_in[32+:8], key_in[64+:8], key_in[96+:8]}),
		.m0(m0),
		.m1(m1),
		.word_out(col3)
	);
	assign key_out[120+:8] = col0[24+:8];
	assign key_out[112+:8] = col1[24+:8];
	assign key_out[104+:8] = col2[24+:8];
	assign key_out[96+:8] = col3[24+:8];
	assign key_out[88+:8] = col0[16+:8];
	assign key_out[80+:8] = col1[16+:8];
	assign key_out[72+:8] = col2[16+:8];
	assign key_out[64+:8] = col3[16+:8];
	assign key_out[56+:8] = col0[8+:8];
	assign key_out[48+:8] = col1[8+:8];
	assign key_out[40+:8] = col2[8+:8];
	assign key_out[32+:8] = col3[8+:8];
	assign key_out[24+:8] = col0[0+:8];
	assign key_out[16+:8] = col1[0+:8];
	assign key_out[8+:8] = col2[0+:8];
	assign key_out[0+:8] = col3[0+:8];
endmodule
module addword (
	a_in,
	b_in,
	m0,
	m1,
	word_out
);
	input [31:0] a_in;
	input [31:0] b_in;
	input m0;
	input m1;
	output [31:0] word_out;
	genvar i;
	generate
		for (i = 0; i < 4; i = i + 1) begin : gen_code_label
			addroundkey_wrapper addroundkey_wrapper_inst(
				.s(a_in[(3 - i) * 8+:8]),
				.k(b_in[(3 - i) * 8+:8]),
				.m0(m0),
				.m1(m1),
				.o(word_out[(3 - i) * 8+:8])
			);
		end
	endgenerate
endmodule
module subword (
	key_in,
	m0,
	m1,
	key_out
);
	input [31:0] key_in;
	input m0;
	input m1;
	output [31:0] key_out;
	genvar i;
	generate
		for (i = 0; i < 4; i = i + 1) begin : gen_code_label
			aes_sbox aes_sbox_inst(
				.i0(key_in[(3 - i) * 8]),
				.i1(key_in[((3 - i) * 8) + 1]),
				.i2(key_in[((3 - i) * 8) + 2]),
				.i3(key_in[((3 - i) * 8) + 3]),
				.i4(key_in[((3 - i) * 8) + 4]),
				.i5(key_in[((3 - i) * 8) + 5]),
				.i6(key_in[((3 - i) * 8) + 6]),
				.i7(key_in[((3 - i) * 8) + 7]),
				.MASK1(m0),
				.MASK2(m1),
				.o0(key_out[(3 - i) * 8]),
				.o1(key_out[((3 - i) * 8) + 1]),
				.o2(key_out[((3 - i) * 8) + 2]),
				.o3(key_out[((3 - i) * 8) + 3]),
				.o4(key_out[((3 - i) * 8) + 4]),
				.o5(key_out[((3 - i) * 8) + 5]),
				.o6(key_out[((3 - i) * 8) + 6]),
				.o7(key_out[((3 - i) * 8) + 7])
			);
		end
	endgenerate
endmodule
module mixcolumns_wrapper (
	a0,
	a1,
	a2,
	a3,
	m0,
	m1,
	b0,
	b1,
	b2,
	b3
);
	input [7:0] a0;
	input [7:0] a1;
	input [7:0] a2;
	input [7:0] a3;
	input m0;
	input m1;
	output [7:0] b0;
	output [7:0] b1;
	output [7:0] b2;
	output [7:0] b3;
	mixcolumns mixcolumns_ins(
		.MASK1_0(m0),
		.MASK1_1(m0),
		.MASK2_0(m1),
		.MASK2_1(m1),
		.a0x0_0(a0[0]),
		.a0x1_0(a0[1]),
		.a0x2_0(a0[2]),
		.a0x3_0(a0[3]),
		.a0x4_0(a0[4]),
		.a0x5_0(a0[5]),
		.a0x6_0(a0[6]),
		.a0x7_0(a0[7]),
		.a1x0_0(a1[0]),
		.a1x1_0(a1[1]),
		.a1x2_0(a1[2]),
		.a1x3_0(a1[3]),
		.a1x4_0(a1[4]),
		.a1x5_0(a1[5]),
		.a1x6_0(a1[6]),
		.a1x7_0(a1[7]),
		.a2x0_0(a2[0]),
		.a2x1_0(a2[1]),
		.a2x2_0(a2[2]),
		.a2x3_0(a2[3]),
		.a2x4_0(a2[4]),
		.a2x5_0(a2[5]),
		.a2x6_0(a2[6]),
		.a2x7_0(a2[7]),
		.a3x0_0(a3[0]),
		.a3x1_0(a3[1]),
		.a3x2_0(a3[2]),
		.a3x3_0(a3[3]),
		.a3x4_0(a3[4]),
		.a3x5_0(a3[5]),
		.a3x6_0(a3[6]),
		.a3x7_0(a3[7]),
		.b0x0_0(b0[0]),
		.b0x1_0(b0[1]),
		.b0x2_0(b0[2]),
		.b0x3_0(b0[3]),
		.b0x4_0(b0[4]),
		.b0x5_0(b0[5]),
		.b0x6_0(b0[6]),
		.b0x7_0(b0[7]),
		.b1x0_0(b1[0]),
		.b1x1_0(b1[1]),
		.b1x2_0(b1[2]),
		.b1x3_0(b1[3]),
		.b1x4_0(b1[4]),
		.b1x5_0(b1[5]),
		.b1x6_0(b1[6]),
		.b1x7_0(b1[7]),
		.b2x0_0(b2[0]),
		.b2x1_0(b2[1]),
		.b2x2_0(b2[2]),
		.b2x3_0(b2[3]),
		.b2x4_0(b2[4]),
		.b2x5_0(b2[5]),
		.b2x6_0(b2[6]),
		.b2x7_0(b2[7]),
		.b3x0_0(b3[0]),
		.b3x1_0(b3[1]),
		.b3x2_0(b3[2]),
		.b3x3_0(b3[3]),
		.b3x4_0(b3[4]),
		.b3x5_0(b3[5]),
		.b3x6_0(b3[6]),
		.b3x7_0(b3[7])
	);
endmodule
module mixcolumns (
	a0x0_0,
	a0x0_1,
	a1x0_0,
	a1x0_1,
	a2x0_0,
	a2x0_1,
	a3x0_0,
	a3x0_1,
	a0x1_0,
	a0x1_1,
	a1x1_0,
	a1x1_1,
	a2x1_0,
	a2x1_1,
	a3x1_0,
	a3x1_1,
	a0x2_0,
	a0x2_1,
	a1x2_0,
	a1x2_1,
	a2x2_0,
	a2x2_1,
	a3x2_0,
	a3x2_1,
	a0x3_0,
	a0x3_1,
	a1x3_0,
	a1x3_1,
	a2x3_0,
	a2x3_1,
	a3x3_0,
	a3x3_1,
	a0x4_0,
	a0x4_1,
	a1x4_0,
	a1x4_1,
	a2x4_0,
	a2x4_1,
	a3x4_0,
	a3x4_1,
	a0x5_0,
	a0x5_1,
	a1x5_0,
	a1x5_1,
	a2x5_0,
	a2x5_1,
	a3x5_0,
	a3x5_1,
	a0x6_0,
	a0x6_1,
	a1x6_0,
	a1x6_1,
	a2x6_0,
	a2x6_1,
	a3x6_0,
	a3x6_1,
	a0x7_0,
	a0x7_1,
	a1x7_0,
	a1x7_1,
	a2x7_0,
	a2x7_1,
	a3x7_0,
	a3x7_1,
	MASK1_0,
	MASK1_1,
	MASK2_0,
	MASK2_1,
	b0x0_0,
	b0x0_1,
	b1x0_0,
	b1x0_1,
	b2x0_0,
	b2x0_1,
	b3x0_0,
	b3x0_1,
	b0x1_0,
	b0x1_1,
	b1x1_0,
	b1x1_1,
	b2x1_0,
	b2x1_1,
	b3x1_0,
	b3x1_1,
	b0x2_0,
	b0x2_1,
	b1x2_0,
	b1x2_1,
	b2x2_0,
	b2x2_1,
	b3x2_0,
	b3x2_1,
	b0x3_0,
	b0x3_1,
	b1x3_0,
	b1x3_1,
	b2x3_0,
	b2x3_1,
	b3x3_0,
	b3x3_1,
	b0x4_0,
	b0x4_1,
	b1x4_0,
	b1x4_1,
	b2x4_0,
	b2x4_1,
	b3x4_0,
	b3x4_1,
	b0x5_0,
	b0x5_1,
	b1x5_0,
	b1x5_1,
	b2x5_0,
	b2x5_1,
	b3x5_0,
	b3x5_1,
	b0x6_0,
	b0x6_1,
	b1x6_0,
	b1x6_1,
	b2x6_0,
	b2x6_1,
	b3x6_0,
	b3x6_1,
	b0x7_0,
	b0x7_1,
	b1x7_0,
	b1x7_1,
	b2x7_0,
	b2x7_1,
	b3x7_0,
	b3x7_1
);
	input a0x0_0;
	input a0x0_1;
	input a1x0_0;
	input a1x0_1;
	input a2x0_0;
	input a2x0_1;
	input a3x0_0;
	input a3x0_1;
	input a0x1_0;
	input a0x1_1;
	input a1x1_0;
	input a1x1_1;
	input a2x1_0;
	input a2x1_1;
	input a3x1_0;
	input a3x1_1;
	input a0x2_0;
	input a0x2_1;
	input a1x2_0;
	input a1x2_1;
	input a2x2_0;
	input a2x2_1;
	input a3x2_0;
	input a3x2_1;
	input a0x3_0;
	input a0x3_1;
	input a1x3_0;
	input a1x3_1;
	input a2x3_0;
	input a2x3_1;
	input a3x3_0;
	input a3x3_1;
	input a0x4_0;
	input a0x4_1;
	input a1x4_0;
	input a1x4_1;
	input a2x4_0;
	input a2x4_1;
	input a3x4_0;
	input a3x4_1;
	input a0x5_0;
	input a0x5_1;
	input a1x5_0;
	input a1x5_1;
	input a2x5_0;
	input a2x5_1;
	input a3x5_0;
	input a3x5_1;
	input a0x6_0;
	input a0x6_1;
	input a1x6_0;
	input a1x6_1;
	input a2x6_0;
	input a2x6_1;
	input a3x6_0;
	input a3x6_1;
	input a0x7_0;
	input a0x7_1;
	input a1x7_0;
	input a1x7_1;
	input a2x7_0;
	input a2x7_1;
	input a3x7_0;
	input a3x7_1;
	input MASK1_0;
	input MASK1_1;
	input MASK2_0;
	input MASK2_1;
	output b0x0_0;
	output b0x0_1;
	output b1x0_0;
	output b1x0_1;
	output b2x0_0;
	output b2x0_1;
	output b3x0_0;
	output b3x0_1;
	output b0x1_0;
	output b0x1_1;
	output b1x1_0;
	output b1x1_1;
	output b2x1_0;
	output b2x1_1;
	output b3x1_0;
	output b3x1_1;
	output b0x2_0;
	output b0x2_1;
	output b1x2_0;
	output b1x2_1;
	output b2x2_0;
	output b2x2_1;
	output b3x2_0;
	output b3x2_1;
	output b0x3_0;
	output b0x3_1;
	output b1x3_0;
	output b1x3_1;
	output b2x3_0;
	output b2x3_1;
	output b3x3_0;
	output b3x3_1;
	output b0x4_0;
	output b0x4_1;
	output b1x4_0;
	output b1x4_1;
	output b2x4_0;
	output b2x4_1;
	output b3x4_0;
	output b3x4_1;
	output b0x5_0;
	output b0x5_1;
	output b1x5_0;
	output b1x5_1;
	output b2x5_0;
	output b2x5_1;
	output b3x5_0;
	output b3x5_1;
	output b0x6_0;
	output b0x6_1;
	output b1x6_0;
	output b1x6_1;
	output b2x6_0;
	output b2x6_1;
	output b3x6_0;
	output b3x6_1;
	output b0x7_0;
	output b0x7_1;
	output b1x7_0;
	output b1x7_1;
	output b2x7_0;
	output b2x7_1;
	output b3x7_0;
	output b3x7_1;
	assign ha0x0_0 = a0x0_0 ^ MASK1_0;
	assign ha0x0_1 = MASK1_1 ^ MASK2_1;
	assign a0_mul_2x0_0 = a0x7_0;
	assign a0_mul_2x0_1 = MASK2_1;
	assign a0_mul_2x1_0 = ha0x0_0 ^ a0x7_0;
	assign a0_mul_2x1_1 = MASK1_1;
	assign a0_mul_2x2_0 = a0x1_0;
	assign a0_mul_2x2_1 = MASK1_1 ^ MASK2_1;
	assign a0_mul_2x3_0 = a0x2_0 ^ a0x7_0;
	assign a0_mul_2x3_1 = MASK1_1;
	assign a0_mul_2x4_0 = a0x3_0 ^ a0x7_0;
	assign a0_mul_2x4_1 = MASK1_1 ^ MASK2_1;
	assign a0_mul_2x5_0 = a0x4_0;
	assign a0_mul_2x5_1 = MASK1_1;
	assign a0_mul_2x6_0 = a0x5_0;
	assign a0_mul_2x6_1 = MASK2_1;
	assign a0_mul_2x7_0 = a0x6_0;
	assign a0_mul_2x7_1 = MASK1_1;
	assign ha1x0_0 = a1x0_0 ^ MASK1_0;
	assign ha1x0_1 = MASK1_1 ^ MASK2_1;
	assign a1_mul_2x0_0 = a1x7_0;
	assign a1_mul_2x0_1 = MASK2_1;
	assign a1_mul_2x1_0 = ha1x0_0 ^ a1x7_0;
	assign a1_mul_2x1_1 = MASK1_1;
	assign a1_mul_2x2_0 = a1x1_0;
	assign a1_mul_2x2_1 = MASK1_1 ^ MASK2_1;
	assign a1_mul_2x3_0 = a1x2_0 ^ a1x7_0;
	assign a1_mul_2x3_1 = MASK1_1;
	assign a1_mul_2x4_0 = a1x3_0 ^ a1x7_0;
	assign a1_mul_2x4_1 = MASK1_1 ^ MASK2_1;
	assign a1_mul_2x5_0 = a1x4_0;
	assign a1_mul_2x5_1 = MASK1_1;
	assign a1_mul_2x6_0 = a1x5_0;
	assign a1_mul_2x6_1 = MASK2_1;
	assign a1_mul_2x7_0 = a1x6_0;
	assign a1_mul_2x7_1 = MASK1_1;
	assign ha2x0_0 = a2x0_0 ^ MASK1_0;
	assign ha2x0_1 = MASK1_1 ^ MASK2_1;
	assign a2_mul_2x0_0 = a2x7_0;
	assign a2_mul_2x0_1 = MASK2_1;
	assign a2_mul_2x1_0 = ha2x0_0 ^ a2x7_0;
	assign a2_mul_2x1_1 = MASK1_1;
	assign a2_mul_2x2_0 = a2x1_0;
	assign a2_mul_2x2_1 = MASK1_1 ^ MASK2_1;
	assign a2_mul_2x3_0 = a2x2_0 ^ a2x7_0;
	assign a2_mul_2x3_1 = MASK1_1;
	assign a2_mul_2x4_0 = a2x3_0 ^ a2x7_0;
	assign a2_mul_2x4_1 = MASK1_1 ^ MASK2_1;
	assign a2_mul_2x5_0 = a2x4_0;
	assign a2_mul_2x5_1 = MASK1_1;
	assign a2_mul_2x6_0 = a2x5_0;
	assign a2_mul_2x6_1 = MASK2_1;
	assign a2_mul_2x7_0 = a2x6_0;
	assign a2_mul_2x7_1 = MASK1_1;
	assign ha3x0_0 = a3x0_0 ^ MASK1_0;
	assign ha3x0_1 = MASK1_1 ^ MASK2_1;
	assign a3_mul_2x0_0 = a3x7_0;
	assign a3_mul_2x0_1 = MASK2_1;
	assign a3_mul_2x1_0 = ha3x0_0 ^ a3x7_0;
	assign a3_mul_2x1_1 = MASK1_1;
	assign a3_mul_2x2_0 = a3x1_0;
	assign a3_mul_2x2_1 = MASK1_1 ^ MASK2_1;
	assign a3_mul_2x3_0 = a3x2_0 ^ a3x7_0;
	assign a3_mul_2x3_1 = MASK1_1;
	assign a3_mul_2x4_0 = a3x3_0 ^ a3x7_0;
	assign a3_mul_2x4_1 = MASK1_1 ^ MASK2_1;
	assign a3_mul_2x5_0 = a3x4_0;
	assign a3_mul_2x5_1 = MASK1_1;
	assign a3_mul_2x6_0 = a3x5_0;
	assign a3_mul_2x6_1 = MASK2_1;
	assign a3_mul_2x7_0 = a3x6_0;
	assign a3_mul_2x7_1 = MASK1_1;
	assign ha0_mul_2x0_0 = a0_mul_2x0_0 ^ MASK1_0;
	assign ha0_mul_2x0_1 = MASK1_1 ^ MASK2_1;
	assign ha0_mul_2x2_0 = a0_mul_2x2_0 ^ MASK1_0;
	assign ha0_mul_2x2_1 = MASK2_1;
	assign ha0_mul_2x3_0 = a0_mul_2x3_0 ^ MASK2_0;
	assign ha0_mul_2x3_1 = MASK1_1 ^ MASK2_1;
	assign a0_mul_3x0_0 = ha0_mul_2x0_0 ^ a0x0_0;
	assign a0_mul_3x0_1 = MASK1_1;
	assign a0_mul_3x1_0 = a0_mul_2x1_0 ^ a0x1_0;
	assign a0_mul_3x1_1 = MASK2_1;
	assign a0_mul_3x2_0 = ha0_mul_2x2_0 ^ a0x2_0;
	assign a0_mul_3x2_1 = MASK1_1;
	assign a0_mul_3x3_0 = ha0_mul_2x3_0 ^ a0x3_0;
	assign a0_mul_3x3_1 = MASK2_1;
	assign a0_mul_3x4_0 = a0_mul_2x4_0 ^ a0x4_0;
	assign a0_mul_3x4_1 = MASK2_1;
	assign a0_mul_3x5_0 = a0_mul_2x5_0 ^ a0x5_0;
	assign a0_mul_3x5_1 = MASK1_1 ^ MASK2_1;
	assign a0_mul_3x6_0 = a0_mul_2x6_0 ^ a0x6_0;
	assign a0_mul_3x6_1 = MASK1_1 ^ MASK2_1;
	assign a0_mul_3x7_0 = a0_mul_2x7_0 ^ a0x7_0;
	assign a0_mul_3x7_1 = MASK1_1 ^ MASK2_1;
	assign ha1_mul_2x0_0 = a1_mul_2x0_0 ^ MASK1_0;
	assign ha1_mul_2x0_1 = MASK1_1 ^ MASK2_1;
	assign ha1_mul_2x2_0 = a1_mul_2x2_0 ^ MASK1_0;
	assign ha1_mul_2x2_1 = MASK2_1;
	assign ha1_mul_2x3_0 = a1_mul_2x3_0 ^ MASK2_0;
	assign ha1_mul_2x3_1 = MASK1_1 ^ MASK2_1;
	assign a1_mul_3x0_0 = ha1_mul_2x0_0 ^ a1x0_0;
	assign a1_mul_3x0_1 = MASK1_1;
	assign a1_mul_3x1_0 = a1_mul_2x1_0 ^ a1x1_0;
	assign a1_mul_3x1_1 = MASK2_1;
	assign a1_mul_3x2_0 = ha1_mul_2x2_0 ^ a1x2_0;
	assign a1_mul_3x2_1 = MASK1_1;
	assign a1_mul_3x3_0 = ha1_mul_2x3_0 ^ a1x3_0;
	assign a1_mul_3x3_1 = MASK2_1;
	assign a1_mul_3x4_0 = a1_mul_2x4_0 ^ a1x4_0;
	assign a1_mul_3x4_1 = MASK2_1;
	assign a1_mul_3x5_0 = a1_mul_2x5_0 ^ a1x5_0;
	assign a1_mul_3x5_1 = MASK1_1 ^ MASK2_1;
	assign a1_mul_3x6_0 = a1_mul_2x6_0 ^ a1x6_0;
	assign a1_mul_3x6_1 = MASK1_1 ^ MASK2_1;
	assign a1_mul_3x7_0 = a1_mul_2x7_0 ^ a1x7_0;
	assign a1_mul_3x7_1 = MASK1_1 ^ MASK2_1;
	assign ha2_mul_2x0_0 = a2_mul_2x0_0 ^ MASK1_0;
	assign ha2_mul_2x0_1 = MASK1_1 ^ MASK2_1;
	assign ha2_mul_2x2_0 = a2_mul_2x2_0 ^ MASK1_0;
	assign ha2_mul_2x2_1 = MASK2_1;
	assign ha2_mul_2x3_0 = a2_mul_2x3_0 ^ MASK2_0;
	assign ha2_mul_2x3_1 = MASK1_1 ^ MASK2_1;
	assign a2_mul_3x0_0 = ha2_mul_2x0_0 ^ a2x0_0;
	assign a2_mul_3x0_1 = MASK1_1;
	assign a2_mul_3x1_0 = a2_mul_2x1_0 ^ a2x1_0;
	assign a2_mul_3x1_1 = MASK2_1;
	assign a2_mul_3x2_0 = ha2_mul_2x2_0 ^ a2x2_0;
	assign a2_mul_3x2_1 = MASK1_1;
	assign a2_mul_3x3_0 = ha2_mul_2x3_0 ^ a2x3_0;
	assign a2_mul_3x3_1 = MASK2_1;
	assign a2_mul_3x4_0 = a2_mul_2x4_0 ^ a2x4_0;
	assign a2_mul_3x4_1 = MASK2_1;
	assign a2_mul_3x5_0 = a2_mul_2x5_0 ^ a2x5_0;
	assign a2_mul_3x5_1 = MASK1_1 ^ MASK2_1;
	assign a2_mul_3x6_0 = a2_mul_2x6_0 ^ a2x6_0;
	assign a2_mul_3x6_1 = MASK1_1 ^ MASK2_1;
	assign a2_mul_3x7_0 = a2_mul_2x7_0 ^ a2x7_0;
	assign a2_mul_3x7_1 = MASK1_1 ^ MASK2_1;
	assign ha3_mul_2x0_0 = a3_mul_2x0_0 ^ MASK1_0;
	assign ha3_mul_2x0_1 = MASK1_1 ^ MASK2_1;
	assign ha3_mul_2x2_0 = a3_mul_2x2_0 ^ MASK1_0;
	assign ha3_mul_2x2_1 = MASK2_1;
	assign ha3_mul_2x3_0 = a3_mul_2x3_0 ^ MASK2_0;
	assign ha3_mul_2x3_1 = MASK1_1 ^ MASK2_1;
	assign a3_mul_3x0_0 = ha3_mul_2x0_0 ^ a3x0_0;
	assign a3_mul_3x0_1 = MASK1_1;
	assign a3_mul_3x1_0 = a3_mul_2x1_0 ^ a3x1_0;
	assign a3_mul_3x1_1 = MASK2_1;
	assign a3_mul_3x2_0 = ha3_mul_2x2_0 ^ a3x2_0;
	assign a3_mul_3x2_1 = MASK1_1;
	assign a3_mul_3x3_0 = ha3_mul_2x3_0 ^ a3x3_0;
	assign a3_mul_3x3_1 = MASK2_1;
	assign a3_mul_3x4_0 = a3_mul_2x4_0 ^ a3x4_0;
	assign a3_mul_3x4_1 = MASK2_1;
	assign a3_mul_3x5_0 = a3_mul_2x5_0 ^ a3x5_0;
	assign a3_mul_3x5_1 = MASK1_1 ^ MASK2_1;
	assign a3_mul_3x6_0 = a3_mul_2x6_0 ^ a3x6_0;
	assign a3_mul_3x6_1 = MASK1_1 ^ MASK2_1;
	assign a3_mul_3x7_0 = a3_mul_2x7_0 ^ a3x7_0;
	assign a3_mul_3x7_1 = MASK1_1 ^ MASK2_1;
	assign Ab0x0_0 = a0_mul_2x0_0 ^ a1_mul_3x0_0;
	assign Ab0x0_1 = MASK1_1 ^ MASK2_1;
	assign Ab0x1_0 = a0_mul_2x1_0 ^ a1_mul_3x1_0;
	assign Ab0x1_1 = MASK1_1 ^ MASK2_1;
	assign Ab0x2_0 = a0_mul_2x2_0 ^ a1_mul_3x2_0;
	assign Ab0x2_1 = MASK2_1;
	assign Ab0x3_0 = a0_mul_2x3_0 ^ a1_mul_3x3_0;
	assign Ab0x3_1 = MASK1_1 ^ MASK2_1;
	assign Ab0x4_0 = a0_mul_2x4_0 ^ a1_mul_3x4_0;
	assign Ab0x4_1 = MASK1_1;
	assign Ab0x5_0 = a0_mul_2x5_0 ^ a1_mul_3x5_0;
	assign Ab0x5_1 = MASK2_1;
	assign Ab0x6_0 = a0_mul_2x6_0 ^ a1_mul_3x6_0;
	assign Ab0x6_1 = MASK1_1;
	assign Ab0x7_0 = a0_mul_2x7_0 ^ a1_mul_3x7_0;
	assign Ab0x7_1 = MASK2_1;
	assign ha3x1_0 = a3x1_0 ^ MASK1_0;
	assign ha3x1_1 = MASK2_1;
	assign ha3x2_0 = a3x2_0 ^ MASK1_0;
	assign ha3x2_1 = MASK2_1;
	assign ha3x3_0 = a3x3_0 ^ MASK2_0;
	assign ha3x3_1 = MASK1_1 ^ MASK2_1;
	assign ha3x4_0 = a3x4_0 ^ MASK2_0;
	assign ha3x4_1 = MASK1_1 ^ MASK2_1;
	assign ha3x5_0 = a3x5_0 ^ MASK1_0;
	assign ha3x5_1 = MASK1_1 ^ MASK2_1;
	assign ha3x6_0 = a3x6_0 ^ MASK2_0;
	assign ha3x6_1 = MASK1_1 ^ MASK2_1;
	assign ha3x7_0 = a3x7_0 ^ MASK1_0;
	assign ha3x7_1 = MASK1_1 ^ MASK2_1;
	assign Bb0x0_0 = ha2x0_0 ^ a3x0_0;
	assign Bb0x0_1 = MASK1_1;
	assign Bb0x1_0 = a2x1_0 ^ ha3x1_0;
	assign Bb0x1_1 = MASK1_1;
	assign Bb0x2_0 = a2x2_0 ^ ha3x2_0;
	assign Bb0x2_1 = MASK1_1;
	assign Bb0x3_0 = a2x3_0 ^ ha3x3_0;
	assign Bb0x3_1 = MASK2_1;
	assign Bb0x4_0 = a2x4_0 ^ ha3x4_0;
	assign Bb0x4_1 = MASK2_1;
	assign Bb0x5_0 = a2x5_0 ^ ha3x5_0;
	assign Bb0x5_1 = MASK1_1;
	assign Bb0x6_0 = a2x6_0 ^ ha3x6_0;
	assign Bb0x6_1 = MASK2_1;
	assign Bb0x7_0 = a2x7_0 ^ ha3x7_0;
	assign Bb0x7_1 = MASK1_1;
	assign b0x0_0 = Ab0x0_0 ^ Bb0x0_0;
	assign b0x0_1 = MASK2_1;
	assign hb0x1_0 = Ab0x1_0 ^ Bb0x1_0;
	assign hb0x1_1 = MASK2_1;
	assign b0x1_0 = hb0x1_0 ^ MASK1_0;
	assign b0x1_1 = MASK1_1 ^ MASK2_1;
	assign hb0x2_0 = Ab0x2_0 ^ Bb0x2_0;
	assign hb0x2_1 = MASK1_1 ^ MASK2_1;
	assign b0x2_0 = hb0x2_0;
	assign b0x2_1 = MASK1_1 ^ MASK2_1;
	assign b0x3_0 = Ab0x3_0 ^ Bb0x3_0;
	assign b0x3_1 = MASK1_1;
	assign hb0x4_0 = Ab0x4_0 ^ Bb0x4_0;
	assign hb0x4_1 = MASK1_1 ^ MASK2_1;
	assign b0x4_0 = hb0x4_0 ^ MASK2_0;
	assign b0x4_1 = MASK1_1;
	assign hb0x5_0 = Ab0x5_0 ^ Bb0x5_0;
	assign hb0x5_1 = MASK1_1 ^ MASK2_1;
	assign b0x5_0 = hb0x5_0 ^ MASK1_0;
	assign b0x5_1 = MASK2_1;
	assign hb0x6_0 = Ab0x6_0 ^ Bb0x6_0;
	assign hb0x6_1 = MASK1_1 ^ MASK2_1;
	assign b0x6_0 = hb0x6_0 ^ MASK2_0;
	assign b0x6_1 = MASK1_1;
	assign hb0x7_0 = Ab0x7_0 ^ Bb0x7_0;
	assign hb0x7_1 = MASK1_1 ^ MASK2_1;
	assign b0x7_0 = hb0x7_0 ^ MASK1_0;
	assign b0x7_1 = MASK2_1;
	assign Ab1x0_0 = ha0x0_0 ^ a1_mul_2x0_0;
	assign Ab1x0_1 = MASK1_1;
	assign Ab1x1_0 = a0x1_0 ^ a1_mul_2x1_0;
	assign Ab1x1_1 = MASK2_1;
	assign Ab1x2_0 = a0x2_0 ^ ha1_mul_2x2_0;
	assign Ab1x2_1 = MASK1_1;
	assign Ab1x3_0 = a0x3_0 ^ ha1_mul_2x3_0;
	assign Ab1x3_1 = MASK2_1;
	assign Ab1x4_0 = a0x4_0 ^ a1_mul_2x4_0;
	assign Ab1x4_1 = MASK2_1;
	assign Ab1x5_0 = a0x5_0 ^ a1_mul_2x5_0;
	assign Ab1x5_1 = MASK1_1 ^ MASK2_1;
	assign Ab1x6_0 = a0x6_0 ^ a1_mul_2x6_0;
	assign Ab1x6_1 = MASK1_1 ^ MASK2_1;
	assign Ab1x7_0 = a0x7_0 ^ a1_mul_2x7_0;
	assign Ab1x7_1 = MASK1_1 ^ MASK2_1;
	assign Bb1x0_0 = a2_mul_3x0_0 ^ a3x0_0;
	assign Bb1x0_1 = MASK1_1 ^ MASK2_1;
	assign Bb1x1_0 = a2_mul_3x1_0 ^ a3x1_0;
	assign Bb1x1_1 = MASK1_1;
	assign Bb1x2_0 = a2_mul_3x2_0 ^ a3x2_0;
	assign Bb1x2_1 = MASK2_1;
	assign Bb1x3_0 = a2_mul_3x3_0 ^ a3x3_0;
	assign Bb1x3_1 = MASK1_1 ^ MASK2_1;
	assign Bb1x4_0 = a2_mul_3x4_0 ^ a3x4_0;
	assign Bb1x4_1 = MASK1_1 ^ MASK2_1;
	assign Bb1x5_0 = a2_mul_3x5_0 ^ a3x5_0;
	assign Bb1x5_1 = MASK1_1;
	assign Bb1x6_0 = a2_mul_3x6_0 ^ a3x6_0;
	assign Bb1x6_1 = MASK2_1;
	assign Bb1x7_0 = a2_mul_3x7_0 ^ a3x7_0;
	assign Bb1x7_1 = MASK1_1;
	assign b1x0_0 = Ab1x0_0 ^ Bb1x0_0;
	assign b1x0_1 = MASK2_1;
	assign b1x1_0 = Ab1x1_0 ^ Bb1x1_0;
	assign b1x1_1 = MASK1_1 ^ MASK2_1;
	assign b1x2_0 = Ab1x2_0 ^ Bb1x2_0;
	assign b1x2_1 = MASK1_1 ^ MASK2_1;
	assign b1x3_0 = Ab1x3_0 ^ Bb1x3_0;
	assign b1x3_1 = MASK1_1;
	assign b1x4_0 = Ab1x4_0 ^ Bb1x4_0;
	assign b1x4_1 = MASK1_1;
	assign b1x5_0 = Ab1x5_0 ^ Bb1x5_0;
	assign b1x5_1 = MASK2_1;
	assign b1x6_0 = Ab1x6_0 ^ Bb1x6_0;
	assign b1x6_1 = MASK1_1;
	assign b1x7_0 = Ab1x7_0 ^ Bb1x7_0;
	assign b1x7_1 = MASK2_1;
	assign ha0x1_0 = a0x1_0 ^ MASK1_0;
	assign ha0x1_1 = MASK2_1;
	assign ha0x2_0 = a0x2_0 ^ MASK1_0;
	assign ha0x2_1 = MASK2_1;
	assign ha0x3_0 = a0x3_0 ^ MASK2_0;
	assign ha0x3_1 = MASK1_1 ^ MASK2_1;
	assign ha0x4_0 = a0x4_0 ^ MASK2_0;
	assign ha0x4_1 = MASK1_1 ^ MASK2_1;
	assign ha0x5_0 = a0x5_0 ^ MASK1_0;
	assign ha0x5_1 = MASK1_1 ^ MASK2_1;
	assign ha0x6_0 = a0x6_0 ^ MASK2_0;
	assign ha0x6_1 = MASK1_1 ^ MASK2_1;
	assign ha0x7_0 = a0x7_0 ^ MASK1_0;
	assign ha0x7_1 = MASK1_1 ^ MASK2_1;
	assign Ab2x0_0 = ha0x0_0 ^ a1x0_0;
	assign Ab2x0_1 = MASK1_1;
	assign Ab2x1_0 = ha0x1_0 ^ a1x1_0;
	assign Ab2x1_1 = MASK1_1;
	assign Ab2x2_0 = ha0x2_0 ^ a1x2_0;
	assign Ab2x2_1 = MASK1_1;
	assign Ab2x3_0 = ha0x3_0 ^ a1x3_0;
	assign Ab2x3_1 = MASK2_1;
	assign Ab2x4_0 = ha0x4_0 ^ a1x4_0;
	assign Ab2x4_1 = MASK2_1;
	assign Ab2x5_0 = ha0x5_0 ^ a1x5_0;
	assign Ab2x5_1 = MASK1_1;
	assign Ab2x6_0 = ha0x6_0 ^ a1x6_0;
	assign Ab2x6_1 = MASK2_1;
	assign Ab2x7_0 = ha0x7_0 ^ a1x7_0;
	assign Ab2x7_1 = MASK1_1;
	assign Bb2x0_0 = a2_mul_2x0_0 ^ a3_mul_3x0_0;
	assign Bb2x0_1 = MASK1_1 ^ MASK2_1;
	assign Bb2x1_0 = a2_mul_2x1_0 ^ a3_mul_3x1_0;
	assign Bb2x1_1 = MASK1_1 ^ MASK2_1;
	assign Bb2x2_0 = a2_mul_2x2_0 ^ a3_mul_3x2_0;
	assign Bb2x2_1 = MASK2_1;
	assign Bb2x3_0 = a2_mul_2x3_0 ^ a3_mul_3x3_0;
	assign Bb2x3_1 = MASK1_1 ^ MASK2_1;
	assign Bb2x4_0 = a2_mul_2x4_0 ^ a3_mul_3x4_0;
	assign Bb2x4_1 = MASK1_1;
	assign Bb2x5_0 = a2_mul_2x5_0 ^ a3_mul_3x5_0;
	assign Bb2x5_1 = MASK2_1;
	assign Bb2x6_0 = a2_mul_2x6_0 ^ a3_mul_3x6_0;
	assign Bb2x6_1 = MASK1_1;
	assign Bb2x7_0 = a2_mul_2x7_0 ^ a3_mul_3x7_0;
	assign Bb2x7_1 = MASK2_1;
	assign b2x0_0 = Ab2x0_0 ^ Bb2x0_0;
	assign b2x0_1 = MASK2_1;
	assign hb2x1_0 = Ab2x1_0 ^ Bb2x1_0;
	assign hb2x1_1 = MASK2_1;
	assign b2x1_0 = hb2x1_0 ^ MASK1_0;
	assign b2x1_1 = MASK1_1 ^ MASK2_1;
	assign b2x2_0 = Ab2x2_0 ^ Bb2x2_0;
	assign b2x2_1 = MASK1_1 ^ MASK2_1;
	assign b2x3_0 = Ab2x3_0 ^ Bb2x3_0;
	assign b2x3_1 = MASK1_1;
	assign hb2x4_0 = Ab2x4_0 ^ Bb2x4_0;
	assign hb2x4_1 = MASK1_1 ^ MASK2_1;
	assign b2x4_0 = hb2x4_0 ^ MASK2_0;
	assign b2x4_1 = MASK1_1;
	assign hb2x5_0 = Ab2x5_0 ^ Bb2x5_0;
	assign hb2x5_1 = MASK1_1 ^ MASK2_1;
	assign b2x5_0 = hb2x5_0 ^ MASK1_0;
	assign b2x5_1 = MASK2_1;
	assign hb2x6_0 = Ab2x6_0 ^ Bb2x6_0;
	assign hb2x6_1 = MASK1_1 ^ MASK2_1;
	assign b2x6_0 = hb2x6_0 ^ MASK2_0;
	assign b2x6_1 = MASK1_1;
	assign hb2x7_0 = Ab2x7_0 ^ Bb2x7_0;
	assign hb2x7_1 = MASK1_1 ^ MASK2_1;
	assign b2x7_0 = hb2x7_0 ^ MASK1_0;
	assign b2x7_1 = MASK2_1;
	assign Ab3x0_0 = a0_mul_3x0_0 ^ a1x0_0;
	assign Ab3x0_1 = MASK1_1 ^ MASK2_1;
	assign Ab3x1_0 = a0_mul_3x1_0 ^ a1x1_0;
	assign Ab3x1_1 = MASK1_1;
	assign Ab3x2_0 = a0_mul_3x2_0 ^ a1x2_0;
	assign Ab3x2_1 = MASK2_1;
	assign Ab3x3_0 = a0_mul_3x3_0 ^ a1x3_0;
	assign Ab3x3_1 = MASK1_1 ^ MASK2_1;
	assign Ab3x4_0 = a0_mul_3x4_0 ^ a1x4_0;
	assign Ab3x4_1 = MASK1_1 ^ MASK2_1;
	assign Ab3x5_0 = a0_mul_3x5_0 ^ a1x5_0;
	assign Ab3x5_1 = MASK1_1;
	assign Ab3x6_0 = a0_mul_3x6_0 ^ a1x6_0;
	assign Ab3x6_1 = MASK2_1;
	assign Ab3x7_0 = a0_mul_3x7_0 ^ a1x7_0;
	assign Ab3x7_1 = MASK1_1;
	assign Bb3x0_0 = ha2x0_0 ^ a3_mul_2x0_0;
	assign Bb3x0_1 = MASK1_1;
	assign Bb3x1_0 = a2x1_0 ^ a3_mul_2x1_0;
	assign Bb3x1_1 = MASK2_1;
	assign Bb3x2_0 = a2x2_0 ^ ha3_mul_2x2_0;
	assign Bb3x2_1 = MASK1_1;
	assign Bb3x3_0 = a2x3_0 ^ ha3_mul_2x3_0;
	assign Bb3x3_1 = MASK2_1;
	assign Bb3x4_0 = a2x4_0 ^ a3_mul_2x4_0;
	assign Bb3x4_1 = MASK2_1;
	assign Bb3x5_0 = a2x5_0 ^ a3_mul_2x5_0;
	assign Bb3x5_1 = MASK1_1 ^ MASK2_1;
	assign Bb3x6_0 = a2x6_0 ^ a3_mul_2x6_0;
	assign Bb3x6_1 = MASK1_1 ^ MASK2_1;
	assign Bb3x7_0 = a2x7_0 ^ a3_mul_2x7_0;
	assign Bb3x7_1 = MASK1_1 ^ MASK2_1;
	assign b3x0_0 = Ab3x0_0 ^ Bb3x0_0;
	assign b3x0_1 = MASK2_1;
	assign b3x1_0 = Ab3x1_0 ^ Bb3x1_0;
	assign b3x1_1 = MASK1_1 ^ MASK2_1;
	assign b3x2_0 = Ab3x2_0 ^ Bb3x2_0;
	assign b3x2_1 = MASK1_1 ^ MASK2_1;
	assign b3x3_0 = Ab3x3_0 ^ Bb3x3_0;
	assign b3x3_1 = MASK1_1;
	assign b3x4_0 = Ab3x4_0 ^ Bb3x4_0;
	assign b3x4_1 = MASK1_1;
	assign b3x5_0 = Ab3x5_0 ^ Bb3x5_0;
	assign b3x5_1 = MASK2_1;
	assign b3x6_0 = Ab3x6_0 ^ Bb3x6_0;
	assign b3x6_1 = MASK1_1;
	assign b3x7_0 = Ab3x7_0 ^ Bb3x7_0;
	assign b3x7_1 = MASK2_1;
endmodule
module addroundkey_trans (
	state_in,
	key_in,
	m0,
	m1,
	state_out
);
	input [127:0] state_in;
	input [127:0] key_in;
	input m0;
	input m1;
	output [127:0] state_out;
	genvar i;
	generate
		for (i = 0; i < 16; i = i + 1) begin : gen_code_label
			addroundkey_wrapper addroundkey_wrapper_inst(
				.s(state_in[(15 - i) * 8+:8]),
				.k(key_in[(15 - i) * 8+:8]),
				.m0(m0),
				.m1(m1),
				.o(state_out[(15 - i) * 8+:8])
			);
		end
	endgenerate
endmodule
module addroundkey_wrapper (
	s,
	k,
	m0,
	m1,
	o
);
	input [7:0] s;
	input [7:0] k;
	input m0;
	input m1;
	output [7:0] o;
	addroundkey addroundkey_ins(
		.MASK1(m0),
		.MASK2(m1),
		.i0(s[0]),
		.i1(s[1]),
		.i2(s[2]),
		.i3(s[3]),
		.i4(s[4]),
		.i5(s[5]),
		.i6(s[6]),
		.i7(s[7]),
		.k0(k[0]),
		.k1(k[1]),
		.k2(k[2]),
		.k3(k[3]),
		.k4(k[4]),
		.k5(k[5]),
		.k6(k[6]),
		.k7(k[7]),
		.o0(o[0]),
		.o1(o[1]),
		.o2(o[2]),
		.o3(o[3]),
		.o4(o[4]),
		.o5(o[5]),
		.o6(o[6]),
		.o7(o[7])
	);
endmodule
module addroundkey (
	MASK1,
	MASK2,
	i0,
	i1,
	i2,
	i3,
	i4,
	i5,
	i6,
	i7,
	k0,
	k1,
	k2,
	k3,
	k4,
	k5,
	k6,
	k7,
	o0,
	o1,
	o2,
	o3,
	o4,
	o5,
	o6,
	o7
);
	input MASK1;
	input MASK2;
	input i0;
	input i1;
	input i2;
	input i3;
	input i4;
	input i5;
	input i6;
	input i7;
	input k0;
	input k1;
	input k2;
	input k3;
	input k4;
	input k5;
	input k6;
	input k7;
	output o0;
	output o1;
	output o2;
	output o3;
	output o4;
	output o5;
	output o6;
	output o7;
	assign MASK12 = MASK1 ^ MASK2;
	assign mk0 = k0 ^ MASK1;
	assign mk1 = k1 ^ MASK1;
	assign mk2 = k2 ^ MASK1;
	assign mk3 = k3 ^ MASK2;
	assign mk4 = k4 ^ MASK2;
	assign mk5 = k5 ^ MASK1;
	assign mk6 = k6 ^ MASK2;
	assign mk7 = k7 ^ MASK1;
	assign mo0 = i0 ^ mk0;
	assign mo1 = i1 ^ mk1;
	assign mo2 = i2 ^ mk2;
	assign mo3 = i3 ^ mk3;
	assign mo4 = i4 ^ mk4;
	assign mo5 = i5 ^ mk5;
	assign mo6 = i6 ^ mk6;
	assign mo7 = i7 ^ mk7;
	assign o0 = mo0 ^ MASK12;
	assign o1 = mo1 ^ MASK2;
	assign o2 = mo2 ^ MASK2;
	assign o3 = mo3 ^ MASK12;
	assign o4 = mo4 ^ MASK12;
	assign o5 = mo5 ^ MASK12;
	assign o6 = mo6 ^ MASK12;
	assign o7 = mo7 ^ MASK12;
endmodule
module subbytes_wrapper (
	state_in,
	m0,
	m1,
	state_out
);
	input [127:0] state_in;
	input m0;
	input m1;
	output [127:0] state_out;
	genvar i;
	generate
		for (i = 0; i < 16; i = i + 1) begin : gen_code_label
			aes_sbox aes_sbox_inst(
				.i0(state_in[(15 - i) * 8]),
				.i1(state_in[((15 - i) * 8) + 1]),
				.i2(state_in[((15 - i) * 8) + 2]),
				.i3(state_in[((15 - i) * 8) + 3]),
				.i4(state_in[((15 - i) * 8) + 4]),
				.i5(state_in[((15 - i) * 8) + 5]),
				.i6(state_in[((15 - i) * 8) + 6]),
				.i7(state_in[((15 - i) * 8) + 7]),
				.MASK1(m0),
				.MASK2(m1),
				.o0(state_out[(15 - i) * 8]),
				.o1(state_out[((15 - i) * 8) + 1]),
				.o2(state_out[((15 - i) * 8) + 2]),
				.o3(state_out[((15 - i) * 8) + 3]),
				.o4(state_out[((15 - i) * 8) + 4]),
				.o5(state_out[((15 - i) * 8) + 5]),
				.o6(state_out[((15 - i) * 8) + 6]),
				.o7(state_out[((15 - i) * 8) + 7])
			);
		end
	endgenerate
endmodule
module aes_sbox (
	i0,
	i1,
	i2,
	i3,
	i4,
	i5,
	i6,
	i7,
	MASK1,
	MASK2,
	o0,
	o1,
	o2,
	o3,
	o4,
	o5,
	o6,
	o7
);
	input i0;
	input i1;
	input i2;
	input i3;
	input i4;
	input i5;
	input i6;
	input i7;
	input MASK1;
	input MASK2;
	output o0;
	output o1;
	output o2;
	output o3;
	output o4;
	output o5;
	output o6;
	output o7;
	assign T1_0 = i7 ^ i4;
	assign T1_1 = MASK1 ^ MASK2;
	assign T2_0 = i7 ^ i2;
	assign T2_1 = MASK1;
	assign T3_0 = i7 ^ i1;
	assign T3_1 = MASK1;
	assign T4_0 = i4 ^ i2;
	assign T4_1 = MASK2;
	assign T5_0 = i3 ^ i1;
	assign T5_1 = MASK2;
	assign T6_0 = T1_0 ^ T5_0;
	assign T6_1 = MASK1;
	assign T7_0 = i6 ^ i5;
	assign T7_1 = MASK1 ^ MASK2;
	assign T8_0 = i0 ^ T6_0;
	assign T8_1 = MASK1 ^ MASK2;
	assign ht8_0 = T8_0 ^ MASK1;
	assign ht8_1 = MASK2;
	assign T9_0 = i0 ^ T7_0;
	assign T9_1 = MASK1;
	assign ht9_0 = T9_0 ^ MASK2;
	assign ht9_1 = MASK1 ^ MASK2;
	assign T10_0 = T6_0 ^ T7_0;
	assign T10_1 = MASK2;
	assign T11_0 = i6 ^ i2;
	assign T11_1 = MASK2;
	assign ht11_0 = T11_0 ^ MASK1;
	assign ht11_1 = MASK1 ^ MASK2;
	assign T12_0 = i5 ^ i2;
	assign T12_1 = MASK1;
	assign T13_0 = T3_0 ^ T4_0;
	assign T13_1 = MASK1 ^ MASK2;
	assign ht13_0 = T13_0 ^ MASK1;
	assign ht13_1 = MASK2;
	assign T14_0 = T6_0 ^ T11_0;
	assign T14_1 = MASK1 ^ MASK2;
	assign T15_0 = T5_0 ^ ht11_0;
	assign T15_1 = MASK1;
	assign T16_0 = T5_0 ^ T12_0;
	assign T16_1 = MASK1 ^ MASK2;
	assign T17_0 = T9_0 ^ T16_0;
	assign T17_1 = MASK2;
	assign ht17_0 = T17_0 ^ MASK1;
	assign ht17_1 = MASK1 ^ MASK2;
	assign T18_0 = i4 ^ i0;
	assign T18_1 = MASK1 ^ MASK2;
	assign ht18_0 = T18_0 ^ MASK1;
	assign ht18_1 = MASK2;
	assign T19_0 = T7_0 ^ ht18_0;
	assign T19_1 = MASK1;
	assign ht19_0 = T19_0 ^ MASK2;
	assign ht19_1 = MASK1 ^ MASK2;
	assign T20_0 = T1_0 ^ T19_0;
	assign T20_1 = MASK2;
	assign T21_0 = i1 ^ i0;
	assign T21_1 = MASK1;
	assign T22_0 = T7_0 ^ T21_0;
	assign T22_1 = MASK2;
	assign T23_0 = T2_0 ^ T22_0;
	assign T23_1 = MASK1 ^ MASK2;
	assign T24_0 = T2_0 ^ T10_0;
	assign T24_1 = MASK1 ^ MASK2;
	assign ht24_0 = T24_0 ^ MASK1;
	assign ht24_1 = MASK2;
	assign T25_0 = T20_0 ^ ht17_0;
	assign T25_1 = MASK1;
	assign T26_0 = T3_0 ^ T16_0;
	assign T26_1 = MASK2;
	assign T27_0 = T1_0 ^ T12_0;
	assign T27_1 = MASK2;
	assign ht27_0 = T27_0 ^ MASK1;
	assign ht27_1 = MASK1 ^ MASK2;
	assign D_0 = i0;
	assign D_1 = MASK2;
	assign M1_0 = ((T6_0 & T13_0) ^ ((T6_0 & T13_1) ^ T13_1)) ^ (((T6_1 & T13_0) ^ ((T6_1 & T13_1) ^ T13_1)) ^ T6_1);
	assign M1_1 = T6_1;
	assign M2_0 = ((T23_0 & ht8_0) ^ ((T23_0 & ht8_1) ^ ht8_1)) ^ (((T23_1 & ht8_0) ^ ((T23_1 & ht8_1) ^ ht8_1)) ^ T23_1);
	assign M2_1 = T23_1;
	assign M3_0 = T14_0 ^ M1_0;
	assign M3_1 = MASK2;
	assign M4_0 = ((D_0 & T19_0) ^ ((D_0 & T19_1) ^ T19_1)) ^ (((D_1 & T19_0) ^ ((D_1 & T19_1) ^ T19_1)) ^ D_1);
	assign M4_1 = D_1;
	assign M5_0 = M4_0 ^ M1_0;
	assign M5_1 = MASK1 ^ MASK2;
	assign M6_0 = ((T16_0 & T3_0) ^ ((T16_0 & T3_1) ^ T3_1)) ^ (((T16_1 & T3_0) ^ ((T16_1 & T3_1) ^ T3_1)) ^ T16_1);
	assign M6_1 = T16_1;
	assign M7_0 = ((T22_0 & T9_0) ^ ((T22_0 & T9_1) ^ T9_1)) ^ (((T22_1 & T9_0) ^ ((T22_1 & T9_1) ^ T9_1)) ^ T22_1);
	assign M7_1 = T22_1;
	assign M8_0 = T26_0 ^ M6_0;
	assign M8_1 = MASK1;
	assign M9_0 = ((T20_0 & ht17_0) ^ ((T20_0 & ht17_1) ^ ht17_1)) ^ (((T20_1 & ht17_0) ^ ((T20_1 & ht17_1) ^ ht17_1)) ^ T20_1);
	assign M9_1 = T20_1;
	assign M10_0 = M9_0 ^ M6_0;
	assign M10_1 = MASK1;
	assign M11_0 = ((T15_0 & T1_0) ^ ((T15_0 & T1_1) ^ T1_1)) ^ (((T15_1 & T1_0) ^ ((T15_1 & T1_1) ^ T1_1)) ^ T15_1);
	assign M11_1 = T15_1;
	assign M12_0 = ((ht27_0 & T4_0) ^ ((ht27_0 & T4_1) ^ T4_1)) ^ (((ht27_1 & T4_0) ^ ((ht27_1 & T4_1) ^ T4_1)) ^ ht27_1);
	assign M12_1 = ht27_1;
	assign M13_0 = M12_0 ^ M11_0;
	assign M13_1 = MASK2;
	assign M14_0 = ((T10_0 & T2_0) ^ ((T10_0 & T2_1) ^ T2_1)) ^ (((T10_1 & T2_0) ^ ((T10_1 & T2_1) ^ T2_1)) ^ T10_1);
	assign M14_1 = T10_1;
	assign M15_0 = M14_0 ^ M11_0;
	assign M15_1 = MASK1 ^ MASK2;
	assign M16_0 = M3_0 ^ M2_0;
	assign M16_1 = MASK1;
	assign M17_0 = M5_0 ^ ht24_0;
	assign M17_1 = MASK1;
	assign M18_0 = M8_0 ^ M7_0;
	assign M18_1 = MASK1 ^ MASK2;
	assign M19_0 = M10_0 ^ M15_0;
	assign M19_1 = MASK2;
	assign M20_0 = M16_0 ^ M13_0;
	assign M20_1 = MASK1 ^ MASK2;
	assign M21_0 = M17_0 ^ M15_0;
	assign M21_1 = MASK2;
	assign M22_0 = M18_0 ^ M13_0;
	assign M22_1 = MASK1;
	assign M23_0 = M19_0 ^ T25_0;
	assign M23_1 = MASK1 ^ MASK2;
	assign hm23_0 = M23_0 ^ MASK1;
	assign hm23_1 = MASK2;
	assign M24_0 = M22_0 ^ M23_0;
	assign M24_1 = MASK2;
	assign M25_0 = ((M22_0 & M20_0) ^ ((M22_0 & M20_1) ^ M20_1)) ^ (((M22_1 & M20_0) ^ ((M22_1 & M20_1) ^ M20_1)) ^ M22_1);
	assign M25_1 = M22_1;
	assign hm25_0 = M25_0 ^ MASK2;
	assign hm25_1 = MASK1 ^ MASK2;
	assign M26_0 = M21_0 ^ M25_0;
	assign M26_1 = MASK1 ^ MASK2;
	assign M27_0 = M20_0 ^ M21_0;
	assign M27_1 = MASK1;
	assign M28_0 = M23_0 ^ M25_0;
	assign M28_1 = MASK2;
	assign M29_0 = ((M27_0 & M28_0) ^ ((M27_0 & M28_1) ^ M28_1)) ^ (((M27_1 & M28_0) ^ ((M27_1 & M28_1) ^ M28_1)) ^ M27_1);
	assign M29_1 = M27_1;
	assign M30_0 = ((M24_0 & M26_0) ^ ((M24_0 & M26_1) ^ M26_1)) ^ (((M24_1 & M26_0) ^ ((M24_1 & M26_1) ^ M26_1)) ^ M24_1);
	assign M30_1 = M24_1;
	assign M31_0 = ((M20_0 & hm23_0) ^ ((M20_0 & hm23_1) ^ hm23_1)) ^ (((M20_1 & hm23_0) ^ ((M20_1 & hm23_1) ^ hm23_1)) ^ M20_1);
	assign M31_1 = M20_1;
	assign M32_0 = ((M31_0 & M27_0) ^ ((M31_0 & M27_1) ^ M27_1)) ^ (((M31_1 & M27_0) ^ ((M31_1 & M27_1) ^ M27_1)) ^ M31_1);
	assign M32_1 = M31_1;
	assign M33_0 = M27_0 ^ hm25_0;
	assign M33_1 = MASK2;
	assign M34_0 = ((M22_0 & M21_0) ^ ((M22_0 & M21_1) ^ M21_1)) ^ (((M22_1 & M21_0) ^ ((M22_1 & M21_1) ^ M21_1)) ^ M22_1);
	assign M34_1 = M22_1;
	assign M35_0 = ((M34_0 & M24_0) ^ ((M34_0 & M24_1) ^ M24_1)) ^ (((M34_1 & M24_0) ^ ((M34_1 & M24_1) ^ M24_1)) ^ M34_1);
	assign M35_1 = M34_1;
	assign M36_0 = M24_0 ^ M25_0;
	assign M36_1 = MASK1 ^ MASK2;
	assign M37_0 = M21_0 ^ M29_0;
	assign M37_1 = MASK1 ^ MASK2;
	assign M38_0 = M32_0 ^ M33_0;
	assign M38_1 = MASK1;
	assign M39_0 = M23_0 ^ M30_0;
	assign M39_1 = MASK1;
	assign M40_0 = M35_0 ^ M36_0;
	assign M40_1 = MASK2;
	assign M41_0 = M38_0 ^ M40_0;
	assign M41_1 = MASK1 ^ MASK2;
	assign M42_0 = M37_0 ^ M39_0;
	assign M42_1 = MASK2;
	assign M43_0 = M37_0 ^ M38_0;
	assign M43_1 = MASK2;
	assign M44_0 = M39_0 ^ M40_0;
	assign M44_1 = MASK1 ^ MASK2;
	assign M45_0 = M42_0 ^ M41_0;
	assign M45_1 = MASK1;
	assign M46_0 = ((M44_0 & T6_0) ^ ((M44_0 & T6_1) ^ T6_1)) ^ (((M44_1 & T6_0) ^ ((M44_1 & T6_1) ^ T6_1)) ^ M44_1);
	assign M46_1 = M44_1;
	assign M47_0 = ((M40_0 & T8_0) ^ ((M40_0 & T8_1) ^ T8_1)) ^ (((M40_1 & T8_0) ^ ((M40_1 & T8_1) ^ T8_1)) ^ M40_1);
	assign M47_1 = M40_1;
	assign M48_0 = ((M39_0 & D_0) ^ ((M39_0 & D_1) ^ D_1)) ^ (((M39_1 & D_0) ^ ((M39_1 & D_1) ^ D_1)) ^ M39_1);
	assign M48_1 = M39_1;
	assign M49_0 = ((M43_0 & T16_0) ^ ((M43_0 & T16_1) ^ T16_1)) ^ (((M43_1 & T16_0) ^ ((M43_1 & T16_1) ^ T16_1)) ^ M43_1);
	assign M49_1 = M43_1;
	assign M50_0 = ((ht9_0 & M38_0) ^ ((ht9_0 & M38_1) ^ M38_1)) ^ (((ht9_1 & M38_0) ^ ((ht9_1 & M38_1) ^ M38_1)) ^ ht9_1);
	assign M50_1 = ht9_1;
	assign M51_0 = ((M37_0 & T17_0) ^ ((M37_0 & T17_1) ^ T17_1)) ^ (((M37_1 & T17_0) ^ ((M37_1 & T17_1) ^ T17_1)) ^ M37_1);
	assign M51_1 = M37_1;
	assign M52_0 = ((T15_0 & M42_0) ^ ((T15_0 & M42_1) ^ M42_1)) ^ (((T15_1 & M42_0) ^ ((T15_1 & M42_1) ^ M42_1)) ^ T15_1);
	assign M52_1 = T15_1;
	assign M53_0 = ((T27_0 & M45_0) ^ ((T27_0 & M45_1) ^ M45_1)) ^ (((T27_1 & M45_0) ^ ((T27_1 & M45_1) ^ M45_1)) ^ T27_1);
	assign M53_1 = T27_1;
	assign M54_0 = ((T10_0 & M41_0) ^ ((T10_0 & M41_1) ^ M41_1)) ^ (((T10_1 & M41_0) ^ ((T10_1 & M41_1) ^ M41_1)) ^ T10_1);
	assign M54_1 = T10_1;
	assign M55_0 = ((M44_0 & ht13_0) ^ ((M44_0 & ht13_1) ^ ht13_1)) ^ (((M44_1 & ht13_0) ^ ((M44_1 & ht13_1) ^ ht13_1)) ^ M44_1);
	assign M55_1 = M44_1;
	assign M56_0 = ((M40_0 & T23_0) ^ ((M40_0 & T23_1) ^ T23_1)) ^ (((M40_1 & T23_0) ^ ((M40_1 & T23_1) ^ T23_1)) ^ M40_1);
	assign M56_1 = M40_1;
	assign M57_0 = ((ht19_0 & M39_0) ^ ((ht19_0 & M39_1) ^ M39_1)) ^ (((ht19_1 & M39_0) ^ ((ht19_1 & M39_1) ^ M39_1)) ^ ht19_1);
	assign M57_1 = ht19_1;
	assign M58_0 = ((T3_0 & M43_0) ^ ((T3_0 & M43_1) ^ M43_1)) ^ (((T3_1 & M43_0) ^ ((T3_1 & M43_1) ^ M43_1)) ^ T3_1);
	assign M58_1 = T3_1;
	assign M59_0 = ((M38_0 & T22_0) ^ ((M38_0 & T22_1) ^ T22_1)) ^ (((M38_1 & T22_0) ^ ((M38_1 & T22_1) ^ T22_1)) ^ M38_1);
	assign M59_1 = M38_1;
	assign M60_0 = ((M37_0 & T20_0) ^ ((M37_0 & T20_1) ^ T20_1)) ^ (((M37_1 & T20_0) ^ ((M37_1 & T20_1) ^ T20_1)) ^ M37_1);
	assign M60_1 = M37_1;
	assign M61_0 = ((T1_0 & M42_0) ^ ((T1_0 & M42_1) ^ M42_1)) ^ (((T1_1 & M42_0) ^ ((T1_1 & M42_1) ^ M42_1)) ^ T1_1);
	assign M61_1 = T1_1;
	assign M62_0 = ((T4_0 & M45_0) ^ ((T4_0 & M45_1) ^ M45_1)) ^ (((T4_1 & M45_0) ^ ((T4_1 & M45_1) ^ M45_1)) ^ T4_1);
	assign M62_1 = T4_1;
	assign M63_0 = ((T2_0 & M41_0) ^ ((T2_0 & M41_1) ^ M41_1)) ^ (((T2_1 & M41_0) ^ ((T2_1 & M41_1) ^ M41_1)) ^ T2_1);
	assign M63_1 = T2_1;
	assign L0_0 = M61_0 ^ M62_0;
	assign L0_1 = MASK1;
	assign L1_0 = M50_0 ^ M56_0;
	assign L1_1 = MASK1;
	assign ml1_0 = L1_0 ^ MASK2;
	assign ml1_1 = MASK1 ^ MASK2;
	assign L2_0 = M46_0 ^ M48_0;
	assign L2_1 = MASK2;
	assign L3_0 = M47_0 ^ M55_0;
	assign L3_1 = MASK1;
	assign L4_0 = M54_0 ^ M58_0;
	assign L4_1 = MASK1 ^ MASK2;
	assign L5_0 = M49_0 ^ M61_0;
	assign L5_1 = MASK1;
	assign L6_0 = M62_0 ^ L5_0;
	assign L6_1 = MASK1 ^ MASK2;
	assign L7_0 = M46_0 ^ L3_0;
	assign L7_1 = MASK2;
	assign L8_0 = M51_0 ^ M59_0;
	assign L8_1 = MASK2;
	assign L9_0 = M52_0 ^ M53_0;
	assign L9_1 = MASK1 ^ MASK2;
	assign L10_0 = M53_0 ^ L4_0;
	assign L10_1 = MASK1;
	assign L11_0 = M60_0 ^ L2_0;
	assign L11_1 = MASK1;
	assign L12_0 = M48_0 ^ M51_0;
	assign L12_1 = MASK2;
	assign L13_0 = M50_0 ^ L0_0;
	assign L13_1 = MASK2;
	assign L14_0 = M52_0 ^ M61_0;
	assign L14_1 = MASK2;
	assign L15_0 = M55_0 ^ L1_0;
	assign L15_1 = MASK2;
	assign L16_0 = M56_0 ^ L0_0;
	assign L16_1 = MASK1 ^ MASK2;
	assign L17_0 = M57_0 ^ L1_0;
	assign L17_1 = MASK2;
	assign L18_0 = M58_0 ^ L8_0;
	assign L18_1 = MASK1 ^ MASK2;
	assign L19_0 = M63_0 ^ L4_0;
	assign L19_1 = MASK2;
	assign L20_0 = L0_0 ^ ml1_0;
	assign L20_1 = MASK2;
	assign L21_0 = L1_0 ^ L7_0;
	assign L21_1 = MASK1 ^ MASK2;
	assign hl21_0 = L21_0 ^ MASK1;
	assign hl21_1 = MASK2;
	assign L22_0 = L3_0 ^ L12_0;
	assign L22_1 = MASK1 ^ MASK2;
	assign L23_0 = L18_0 ^ L2_0;
	assign L23_1 = MASK1;
	assign L24_0 = L15_0 ^ L9_0;
	assign L24_1 = MASK1;
	assign L25_0 = L6_0 ^ L10_0;
	assign L25_1 = MASK2;
	assign L26_0 = L7_0 ^ L9_0;
	assign L26_1 = MASK1;
	assign L27_0 = L8_0 ^ L10_0;
	assign L27_1 = MASK1 ^ MASK2;
	assign L28_0 = L11_0 ^ L14_0;
	assign L28_1 = MASK1 ^ MASK2;
	assign L29_0 = L11_0 ^ L17_0;
	assign L29_1 = MASK1 ^ MASK2;
	assign S0_0 = L6_0 ^ L24_0;
	assign S0_1 = MASK2;
	assign S1_0 = ~(L16_0 ^ L26_0);
	assign S1_1 = MASK2;
	assign S2_0 = ~(L19_0 ^ L28_0);
	assign S2_1 = MASK1;
	assign S3_0 = L6_0 ^ hl21_0;
	assign S3_1 = MASK1;
	assign S4_0 = L20_0 ^ L22_0;
	assign S4_1 = MASK1;
	assign S5_0 = L25_0 ^ L29_0;
	assign S5_1 = MASK1;
	assign S6_0 = ~(L13_0 ^ L27_0);
	assign S6_1 = MASK1;
	assign S7_0 = ~(L6_0 ^ L23_0);
	assign S7_1 = MASK2;
	assign m0m1_0 = MASK1 ^ MASK2;
	assign m0m1_1 = MASK1 ^ MASK2;
	assign o7 = S0_0;
	assign o6 = S1_0 ^ m0m1_0;
	assign o5 = S2_0 ^ m0m1_0;
	assign o4 = S3_0;
	assign o3 = S4_0;
	assign o2 = S5_0 ^ MASK2;
	assign o1 = S6_0 ^ MASK2;
	assign o0 = S7_0;
endmodule
