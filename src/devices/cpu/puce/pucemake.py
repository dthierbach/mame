#!/usr/bin/env python
# license:BSD-3-Clause
# copyright-holders:Dirk Thierbach

import sys

fields = {
    "r": "u8 r = BIT(opcode,12,4);",
    "s": "u8 s = BIT(opcode,8,4);",
    "t": "u8 t = BIT(opcode,0,8);",
    "u": "u8 u = BIT(opcode,4,4);",
    "v": "u8 v = BIT(opcode,0,4);",
    "w": "u8 w = BIT(opcode,8,8);",
    "j": "u16 j = (pc & 0xe000) | (opcode & 0x1fff);",
    "k": "u16 b = (pc & 0xff00) | t;",
    "d": "u8 d = BIT(r,1,3);",
    "e": "u8 e = BIT(r,0);",
    "f": "u8 d = BIT(u,1,3);",
    "g": "u8 e = BIT(u,0);"
}

outer = {
    0: None,
    1: ["sai",   "SAI %04x",         "j",   "jump %04x", "j"],
    2: ["amd",   "AMD A%02x,C%02x",  "st",  "[%02x] := A%d", "ts"],
    3: ["mad",   "MAD A%02x,C%02x",  "st",  "A%d := [%02x]", "st"],
    4: ["sade",  "SADE %04x",        "k",   "br EOCF,%04x", "k"],
    5: ["crtb",  "CRTB B%02x,C%02x", "st",  "B%d := 0x%02x", "st"],
    6: ["sadx",  "SAD%d D%x,%04x",   "edk", "br D%d=%d,%04x", "dek"],
    7: ["crta",  "CRTA A%02x,C%02x", "st",  "A%d := 0x%02x", "st"]
}

    0x__: {0x

inner = {
    0x82: {'*': ['amim',  'AMIM M%02x,A%02x',  'uv',  '[M%d--] := A%d',           'uv']},	# no DI
    0x83: {0xF: ['tadi',  'TADI A%02x',        'u',   'DI := A%d',                'u']},	# DI set
    0x85: {0xF: ['ica',   'ICA A%02x',         'u',   'A%d++',                    'u']},	# no DI
    0x86: {'*': ['add',   'ADD A%02x,B%02x',   'uv',  'A%d + B%d + DI0',          'uv']},	# DI0,1,2 = CZH
    0x87: {'*': ['orb',   'ORB A%02x,B%02x',   'uv',  'B%d := A%d or B%d',        'vuv']},	# DI1 = zero
    0x88: {'*': ['amip',  'AMIP M%02x,A%02x',  'uv',  '[M%d++] := A%d',           'uv']},	# no DI
    0x89: {'*': ['bmi',   'BMI M%02x,B%02x',   'uv',  '[M%d] := B%d',             'uv']},	# no DI
    0x8A: {'*': ['bmim',  'BMIM M%02x,B%02x',  'uv',  '[M%d--] := B%d',           'uv']},	# no DI
    0x8B: {0xF: ['rota',  'ROTA A%02x',        'u',   'A%d.M <-> A%d.P',          'uu']},	# no DI
    0x8C: {'*': ['bmip',  'BMIP M%02x,B%02x',  'uv',  '[M%d++] := B%d',           'uv']},	# no DI
    0x8D: {0x8: ['emi',   'EMI M%02x',         'u',   '[M%d] <- data.A',          'u']},	# no DI
    0x8E: {0xF: ['vra',   'VRA A%02x',         'u',   'A%d==0',                   'u']},	# DI1 = zero
    0x90: {0x0: ['mei',   'MEI M%02x',         'u',   'data.A <- [M%d]',          'u']},	# no DI
    0x91: {'*': ['mai',   'MAI M%02x,A%02x',   'uv',  'A%d := [M%d]',             'vu']},	# no DI
    0x92: {'*': ['main',  'MAIM M%02x,A%02x',  'uv',  'A%d := [M%d--]',           'vu']},	# no DI
    0x93: {0xF: ['tbdi',  'TBDI B%02x',        'u',   'DI := B%d',                'u']},	# DI set
    0x94: {0x0: ['meip',  'MEIP M%02x',        'u',   'data.A <- [M%d++]',        'u']},	# no DI
    0x95: {0xF: ['icb',   'ICB B%02x',         'u',   'B%d++',                    'u']},	# no DI
    0x96: {'*': ['adda',  'ADDA A%02x,B%02x',  'uv',  'A%d := A%d + B%d + DI0',   'uuv']},	# DI0,1,2 = CZH
    0x97: {'*': ['and',   'AND A%02x,B%02x',   'uv',  'A%d and B%d',              'uv']},	# DI1 = zero
    0x98: {'*': ['maip',  'MAIP M%02x,A%02x',  'uv',  'A%d := [M%d++]',           'vu']}, 	# no DI
    0x99: {'*': ['mbi',   'MBI M%02x,B%02x',   'uv',  'B%d := [M%d]',             'vu']}, 	# no DI
    0x9A: {'*': ['mbim',  'MBIM M%02x,B%02x',  'uv',  'B%d := [M%d--]',           'vu']},	# no DI
    0x9B: {0xF: ['rotb',  'ROTB B%02x',        'u',   'B%d.M <-> B%d.P',          'uu']},	# no DI
    0x9C: {'*': ['mbip',  'MBIP M%02x,B%02x',  'uv',  'B%d := [M%d++]',           'vu']},	# no DI
    0x9D: {0x0: ['meim',  'MEIM M%02x',        'u',   'data.A <- [M%d--]',        'u']},	# no DI
    0x9E: {0xF: ['vrb',   'VRB B%02x',         'u',   'B%d==0',                   'u']},	# DI1 = zero
    0xA0: {'*': ['icd',   'ICD%d D%x,L%02x',   'gfv', 'L%d++ if D%x={%d}',        'vgf']),	# no DI
    0xA1: {0x8: ['emim',  'EMIM M%02x',        'u',   '[M%d--] <- data.A',        'u']},	# no DI
    0xA2: {0x8: ['emip',  'EMIP M%02x',        'u',   '[M%d++] <- data.A',        'u']},	# no DI
    0xA3: {0xF: ['sdia',  'SDIA A%02x',        'u',   'A%d <-> DI',               'u']},	# no DI
    0xA5: {0xF: ['icl',   'ICL L%02x',         'u',   'L%d++',                    'u']},	# no DI
    0xA6: {'*': ['addb',  'ADDB A%02x,B%02x',  'uv',  'B%d := A%d + B%d + DI0',   'vuv']},	# DI0,1,2 = CZH
    0xA7: {'*': ['anda',  'ANDA A%02x,B%02x',  'uv',  'A%d := A%d and B%d',       'uuv']},	# DI1 = zero
    0xA8: {'*': ['ami',   'AMI M%02x,A%02x',   'uv',  '[M%d] := A%d',             'uv']},	# no DI
    0xA9: {0x8: ['edb',   'EDB B%02x',         'u',   'B%d <- data.A',            'u']},	# no DI
    0xAA: {0x0: ['entl',  'ENTL L%02x',        'u',   'A%d <- name, B%d <- type', 'uu']},	# no DI
    0xAB: {0xF: ['azam',  'AZAM A%02x',        'u',   'A%d.M := 0',               'u']},	# no DI
    0xAD: {0xF: ['edc',   'EDC L%02x',         'u',   'L%d.MMM--, ECOF if zero',  'u']},	# no DI
    0xAE: {0xF: ['dca',   'DCA A%02x',         'u',   'A%d--',                    'u']},	# DI1 = zero
    0xB1: {0x4: ['ese',   'ESE M%02x',         'u',   'sel <- [M%d]',             'u']},	# no DI
    0xB2: {0xF: ['etib',  'ETIB B%02x',        'u',   'B%d <- type',              'u']},	# no DI
    0xB3: {0xF: ['sdib',  'SDIB B%02x',        'u',   'B%d <-> DI',               'u']},	# DI set
    0xB4: {0x2: ['eco',   'ECO M%02x',         'u',   'cmd <- [M%d]',             'u']},	# no DI
    0xB6: {'*': ['sot',   'SOT A%02x,B%02x',   'uv',  'A%d - B%d + DI0',          'uv']},	# DI0,1,2 = CZH
    0xB7: {'*': ['andb',  'ANDB',  'A', 'B', '{argy} := {argx} and {argy}']}
#   DI1 = zero
    0xB8: {0x
# opcode[0xB8]={'8': ['EDA',   'A', '',  '{argx} <- data.B']}
#  no DI
    0xB9: {0x
# opcode[0xB9]={'0': ['ENUA',  'A', '',  '{argx} <- name']}
#   no DI
    0xBA: {0x
# opcode[0xBA]={'*': ['SAB',   'A', 'B', '{argx} <-> {argy} ']}
#   no DI
    0xBB: {0x
# opcode[0xBB]={'F': ['AZAP',  'A', '',  '{argx}.P := 0']}
#   no DI
    0xBC: {0x
# opcode[0xBC]={'*': ['SLL',   'L', 'L', '{argx} <-> {argy}']}
#   no DI
    0xBD: {0x0: ["comx", "COM%d",           "u",  "  C%d",      "u"]},		#  no DI
    0xBD: {0x
# opcode[0xBD]={'0': ['COMx',  'C', '',  '  {argx}']}
#  no DI
    0xBE: {0x
# opcode[0xBE]={'F': ['DCB',   'B', '',  '{argx}--']}
#   DI1 = zero
    0xC3: {0x
# opcode[0xC3]={'0': ['SHDA',  'A', '',  '{argx}, DI0 := 0 >> {argx}'],
#   DI0
#               '1': ['SLDA',  'A', '',  '{argx}, DI0 := DI0 >> {argx}']}
#   DI0
    0xC4: {0x
# opcode[0xC4]={'0': ['SHSA',  'A', '',  'DI0, {argx} := {argx} << 0'],
#   DI0
#               '1': ['SLSA',  'A', '',  'DI0, {argx} := {argx} << DI0']}
#   DI0
    0xC5: {0x
# opcode[0xC5]={'F': ['TDIA',  'A', '',  '{argx} := DI']}
#   no DI
    0xC6: {0x
# opcode[0xC6]={'*': ['SOTA',  'A', 'B', '{argx} := {argx}-{argy}+DI0']}
#   DI0,1,2 = CZH
    0xC7: {0x
# opcode[0xC7]={'*': ['ORE',   'A', 'B', '{argx} xor {argy}']}
#   DI1 = zero
    0xC8: {0x
# opcode[0xC8]={'*': ['REDI',  'D', 'D', 'reset DI {argd}']}
#   reset bits with mask=1 to 0
    0xC9: {0x
# opcode[0xC9]={'*': ['SEDI',  'D', 'D', 'set DI {argd}']}
#   set bits with mask=1 to 1
    0xCA: {0x
# opcode[0xCA]={'0': ['TCCA',  'A', '',  '{argx} <- con']}
#   no DI
    0xCB: {0x
# opcode[0xCB]={'F': ['AZBM',  'B', '',  '{argx}.M := 0']}
#   no DI
    0xD1: {0x
# opcode[0xD1]={'*': ['MLI',   'M', 'L', '{argy} := [{argx}]']}
#   no DI
    0xD3: {0x
# opcode[0xD3]={'0': ['SHDB',  'B', '',  '{argx}, DI0 := 0 >> {argx}'],
#   DI0
#               '1': ['SLDB',  'B', '',  '{argx}, DI0 := DI0 >> {argx}']}
#   DI0
    0xD4: {0x
# opcode[0xD4]={'0': ['SHSB',  'B', '',  'DI0, {argx} := {argx} << 0'],
#   DI0
#               '1': ['SLSB',  'B', '',  'DI0, {argx} := {argx} << DI0']}
#   DI0
    0xD5: {0x
# opcode[0xD5]={'F': ['TDIB',  'B', '',  '{argx} := DI']}
#   no DI
    0xD6: {0x
# opcode[0xD6]={'*': ['SOTB',  'A', 'B', '{argy} := {argx}-{argy}+DI0']}
#   DI0,1,2 = CZH
    0xD7: {0x
# opcode[0xD7]={'*': ['OREA',  'A', 'B', '{argx} := {argx} xor {argy}']}
#   DI1 = zero
    0xD8: {0x
# opcode[0xD8]={'*': ['TAB',   'A', 'B', '{argy} := {argx}']}
#   no DI
    0xD9: {0x
# opcode[0xD9]={'*': ['TABP',  'A', 'B', '{argy}.P := {argx}.P']}
#   no DI
    0xDA: {0x
# opcode[0xDA]={'1': ['TDMA',  'A', '',  '{argx} <- con.M']}
#   no DI
    0xDB: {0x
# opcode[0xDB]={'F': ['AZBP',  'B', '',  '{argx}.P := 0']}
#   no DI
    0xDD: {0x
# opcode[0xDD]={'*': ['MLIM',  'M', 'L', '{argy} := [{argx}--]']}
#   no DI
    0xDE: {0x
# opcode[0xDE]={'*': ['MLIP',  'M', 'L', '{argy} := [{argx}++]']}
#   no DI
    0xE0: {0x
# opcode[0xE0]={'8': ['ESI',   'M', '',  '[{argx}] <- data/type']}
#   no DI
    0xE1: {0x
# opcode[0xE1]={'*': ['LMI',   'M', 'L', '[{argx}] := {argy}']}
#   no DI
    0xE2: {0x
# opcode[0xE2]={'*': ['LPMIP', 'M', 'L', '[{argx}++] := ++{argy}']}
#   no DI
    0xE5: {0x
# opcode[0xE5]={'F': ['DCL',   'L', '',  '{argx}--']}
#   DI1 = zero
    0xE6: {0x
# opcode[0xE6]={'*': ['OR',    'A', 'B', '{argx} or {argy}']}
#   DI1 = zero
    0xE7: {0x
# opcode[0xE7]={'*': ['OREB',  'A', 'B', '{argy} := {argx} xor {argy}']}
#   DI1 = zero
    0xE8: {0x
# opcode[0xE8]={'*': ['TABM',  'A', 'B', '{argy}.M := {argx}.M']}
#   no DI
    0xE9: {"*": ["tba",  "TBA A%02x,B%02x", "uv", "B%d := A%d", "vu"]},
    0xE9: {0x
# opcode[0xE9]={'*': ['TBA',   'A', 'B', '{argx} := {argy}']}	
#   no DI
    0xEA: {0x
# opcode[0xEA]={'2': ['TDPA',  'A', '',  '{argx} <- con.P']}
#   no DI
    0xEB: {0x
# opcode[0xEB]={'8': ['ESIP',  'M', '',  '[{argx}++] <- data/type']}
#   no DI
    0xEC: {0x
# opcode[0xEC]={'8': ['ESIM',  'M', '',  '[{argx}--] <- data/type']}
#   no DI
    0xED: {0x
# opcode[0xED]={'*': ['LMIM',  'M', 'L', '[{argx}--] := {argy}']}
#   no DI
    0xEE: {0x
# opcode[0xEE]={'*': ['LMIP',  'M', 'L', '[{argx}++] := {argy}']}
#   no DI
    0xF1: {0x
# opcode[0xF1]={'0': ['SEI',   'M', '',  'data.W <- [{argx}]']}
#   no DI
    0xF5: {0x
# opcode[0xF5]={'F': ['VRL',   'L', '',  '{argx}==0']}
#   DI1 = zero
    0xF6: {0x
# opcode[0xF6]={'*': ['ORA',   'A', 'B', '{argx} := {argx} or {argy}']}
#   DI1 = zero
    0xF7: {0x
# opcode[0xF7]={'0': ['SEIP',  'M', '',  'data.W <- [{argx}++]']}
#   no DI
    0x__: {0x
# opcode[0xF8]={'*': ['TBAP',  'A', 'B', '{argx}.P := {argy}.P']}
#   no DI
    0x__: {0x
# opcode[0xF9]={'*': ['TBAM',  'A', 'B', '{argx}.M := {argy}.M']}
#   no DI
    0x__: {0x
# opcode[0xFA]={'*': ['TABC',  'A', 'B', ' con <- {argx}, {argy}']}
#   no DI
    0x__: {0x
# opcode[0xFB]={'8': ['DEA',   'L', '',  'data.B <- B{argx}, A{argx} <- data.A'
#   no DI
    0x__: {0x
# opcode[0xFC]={'0': ['DAE',   'L', '',  'data.W <- {argx}'],  
#   no DI
#               '2': ['CAE',   'L', '',  'cmd.W <- {argx}']}
#  no DI
# 
    0xFD: {0x
# opcode[0xFD]={'0': ['SEIM',  'M', '',  'data.W <- [{argx}--]']}
#   no DI
}

def out(fout, s):
    fout.write(s)

def gen_decl_header(fout, name, args):
    argdecl = ",".join([f"u8 {var}" for var in args])
    out(fout, f"void op_{name}({argdecl});\n")

def gen_decl_method(fout, name, args):
    argdecl = ",".join([f"u8 {var}" for var in args])
    out(fout, f"inline void puce_device::{name}({argdecl}) {{\n  op_illegal(NULL);\n}}\n\n")

def gen_decl(fout, fun):
    for _, desc in sorted(outer.items()):
        if desc is not None:
            fun(fout, desc[0], desc[2])
    for _, mix in sorted(inner.items()):
        for _,desc in sorted(mix.items()):
            fun(fout, desc[0], desc[2])

def genf(fout, fs, done):
    for f in fs:
        if f not in done:
            out(fout, fields[f] + "\n")
            done = done + f
    return done;

def gen_mode(fout, mode, l):
    if mode:
        out(fout, f"op_{l[0]}({",".join(l[2])});")
    else:
        out(fout, f'dis = string_format("{l[1]}", {",".join(l[2])});')
        out(fout, f'com = string_format("{l[3]}", {",".join(l[4])});')

def gen_illegal(fout, mode):
    if mode:
        out(fout, "default: op_illegal(opcode);\n")
    else:
        out(fout, 'default: dis = "???"; com = "";')

def gen_all(fout, mode):
    if mode:
        out(fout, "inline void puce_device::decode(u16 pc, u16 opcode) {\n")
    else:
        out(fout, "inline void puce_disassembler::decode(std::ostream &stream, u16 pc, u16 opcode) {\n")
        out(fout, "std::string dis; std::string com;\n")
    done1 = genf(fout, "rst", "")
    out(fout, "switch(r) {\n")
    for i,desc in sorted(outer.items()):
        out(fout, f"case {i}: ")
        if desc is None:
            continue
        out(fout, "{\n");
        done2 = genf(fout, desc[2], done1)
        gen_mode(fout, mode, desc)
        out(fout, " break; }\n")
    out(fout, "default:\n")
    done2 = genf(fout, "uvw", done1)
    out(fout, "switch(w) {\n")
    for i,mix in sorted(inner.items()):
        out(fout, f"case 0x{i:02x}: ")
        if "*" in mix:
            desc = mix["*"]
            out(fout, "{\n");
            done3 = genf(fout, desc[2], done2)
            gen_mode(fout, mode, desc)
            out(fout, " break; }\n")
        else:
            out(fout, "switch(v) {\n");
            for j,desc in sorted(mix.items()):
                out(fout, f"case 0x{j:1x}: ")
                out(fout, "{\n");
                gen_mode(fout, mode, desc)
                out(fout, " break; }\n")
            gen_illegal(fout, mode)
            out(fout, "break; }\n")
    gen_illegal(fout, mode)
    out(fout, "}\n")
    out(fout, "}\n")
    if not mode:
	out(fout, 'util::stream_format(stream, "%-15s ; %s", dis, com);')
    out(fout, "}\n")


fout = sys.stdout

gen_decl(fout, gen_decl_header)
out(fout, "\n----------------------\n\n")
gen_decl(fout, gen_decl_method)
out(fout, "\n----------------------\n\n")
gen_all(fout, True)
out(fout, "\n----------------------\n\n")
gen_all(fout, False)


