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
    "k": "u16 k = (pc & 0xff00) | t;",
    "d": "u8 d = BIT(r,1,3);",
    "e": "u8 e = BIT(r,0);",
    "f": "u8 f = BIT(u,1,3);",
    "g": "u8 g = BIT(u,0);"
}

outer = {
    0: None,
    1: ["sai",   "SAI %04x",         "j",   "jump %04x",      "j"  , 'no DI'],
    2: ["amd",   "AMD A%02x,C%02x",  "st",  "[%02x] := A%d",  "ts" , 'no DI'],
    3: ["mad",   "MAD A%02x,C%02x",  "st",  "A%d := [%02x]",  "st" , 'no DI'],
    4: ["sade",  "SADE %04x",        "k",   "br EOCF,%04x",   "k"  , 'no DI'],
    5: ["crtb",  "CRTB B%02x,C%02x", "st",  "B%d := 0x%02x",  "st" , 'no DI'],
    6: ["sadx",  "SAD%d D%x,%04x",   "edk", "br D%d=%d,%04x", "dek", 'no DI'],
    7: ["crta",  "CRTA A%02x,C%02x", "st",  "A%d := 0x%02x",  "st" , 'no DI']
}

inner = {
    0x82: {'*': ['amim',  'AMIM M%02x,A%02x',  'uv',  '[M%d--] := A%d',               'uv' , 'no DI        ']},
    0x83: {0xF: ['tadi',  'TADI A%02x',        'u',   'DI := A%d',                    'u'  , 'DI set       ']},
    0x85: {0xF: ['ica',   'ICA A%02x',         'u',   'A%d++',                        'u'  , 'no DI        ']},
    0x86: {'*': ['add',   'ADD A%02x,B%02x',   'uv',  'A%d + B%d + DI0',              'uv' , 'DI0,1,2 = CZH']},
    0x87: {'*': ['orb',   'ORB A%02x,B%02x',   'uv',  'B%d := A%d or B%d',            'vuv', 'DI1 = zero   ']},
    0x88: {'*': ['amip',  'AMIP M%02x,A%02x',  'uv',  '[M%d++] := A%d',               'uv' , 'no DI        ']},
    0x89: {'*': ['bmi',   'BMI M%02x,B%02x',   'uv',  '[M%d] := B%d',                 'uv' , 'no DI        ']},
    0x8A: {'*': ['bmim',  'BMIM M%02x,B%02x',  'uv',  '[M%d--] := B%d',               'uv' , 'no DI        ']},
    0x8B: {0xF: ['rota',  'ROTA A%02x',        'u',   'A%d.M <-> A%d.P',              'uu' , 'no DI        ']},
    0x8C: {'*': ['bmip',  'BMIP M%02x,B%02x',  'uv',  '[M%d++] := B%d',               'uv' , 'no DI        ']},
    0x8D: {0x8: ['emi',   'EMI M%02x',         'u',   '[M%d] <- data.A',              'u'  , 'no DI        ']},
    0x8E: {0xF: ['vra',   'VRA A%02x',         'u',   'A%d==0',                       'u'  , 'DI1 = zero   ']},
    0x90: {0x0: ['mei',   'MEI M%02x',         'u',   'data.A <- [M%d]',              'u'  , 'no DI        ']},
    0x91: {'*': ['mai',   'MAI M%02x,A%02x',   'uv',  'A%d := [M%d]',                 'vu' , 'no DI        ']},
    0x92: {'*': ['maim',  'MAIM M%02x,A%02x',  'uv',  'A%d := [M%d--]',               'vu' , 'no DI        ']},
    0x93: {0xF: ['tbdi',  'TBDI B%02x',        'u',   'DI := B%d',                    'u'  , 'DI set       ']},
    0x94: {0x0: ['meip',  'MEIP M%02x',        'u',   'data.A <- [M%d++]',            'u'  , 'no DI        ']},
    0x95: {0xF: ['icb',   'ICB B%02x',         'u',   'B%d++',                        'u'  , 'no DI        ']},
    0x96: {'*': ['adda',  'ADDA A%02x,B%02x',  'uv',  'A%d := A%d + B%d + DI0',       'uuv', 'DI0,1,2 = CZH']},
    0x97: {'*': ['and',   'AND A%02x,B%02x',   'uv',  'A%d and B%d',                  'uv' , 'DI1 = zero   ']},
    0x98: {'*': ['maip',  'MAIP M%02x,A%02x',  'uv',  'A%d := [M%d++]',               'vu' , 'no DI        ']},
    0x99: {'*': ['mbi',   'MBI M%02x,B%02x',   'uv',  'B%d := [M%d]',                 'vu' , 'no DI        ']},
    0x9A: {'*': ['mbim',  'MBIM M%02x,B%02x',  'uv',  'B%d := [M%d--]',               'vu' , 'no DI        ']},
    0x9B: {0xF: ['rotb',  'ROTB B%02x',        'u',   'B%d.M <-> B%d.P',              'uu' , 'no DI        ']},
    0x9C: {'*': ['mbip',  'MBIP M%02x,B%02x',  'uv',  'B%d := [M%d++]',               'vu' , 'no DI        ']},
    0x9D: {0x0: ['meim',  'MEIM M%02x',        'u',   'data.A <- [M%d--]',            'u'  , 'no DI        ']},
    0x9E: {0xF: ['vrb',   'VRB B%02x',         'u',   'B%d==0',                       'u'  , 'DI1 = zero   ']},
    0xA0: {'*': ['icd',   'ICD%d D%x,L%02x',   'gfv', 'L%d++ if D%x=%d',              'vfg', 'no DI        ']},
    0xA1: {0x8: ['emim',  'EMIM M%02x',        'u',   '[M%d--] <- data.A',            'u'  , 'no DI        ']},
    0xA2: {0x8: ['emip',  'EMIP M%02x',        'u',   '[M%d++] <- data.A',            'u'  , 'no DI        ']},
    0xA3: {0xF: ['sdia',  'SDIA A%02x',        'u',   'A%d <-> DI',                   'u'  , 'DI set       ']},
    0xA5: {0xF: ['icl',   'ICL L%02x',         'u',   'L%d++',                        'u'  , 'no DI        ']},
    0xA6: {'*': ['addb',  'ADDB A%02x,B%02x',  'uv',  'B%d := A%d + B%d + DI0',       'vuv', 'DI0,1,2 = CZH']},
    0xA7: {'*': ['anda',  'ANDA A%02x,B%02x',  'uv',  'A%d := A%d and B%d',           'uuv', 'DI1 = zero   ']},
    0xA8: {'*': ['ami',   'AMI M%02x,A%02x',   'uv',  '[M%d] := A%d',                 'uv' , 'no DI        ']},
    0xA9: {0x8: ['edb',   'EDB B%02x',         'u',   'B%d <- data.A',                'u'  , 'no DI        ']},
    0xAA: {0x0: ['entl',  'ENTL L%02x',        'u',   'A%d <- name, B%d <- type',     'uu' , 'no DI        ']},
    0xAB: {0xF: ['azam',  'AZAM A%02x',        'u',   'A%d.M := 0',                   'u'  , 'no DI        ']},
    0xAD: {0xF: ['edc',   'EDC L%02x',         'u',   'L%d.MMM--, ECOF if zero',      'u'  , 'no DI        ']},
    0xAE: {0xF: ['dca',   'DCA A%02x',         'u',   'A%d--',                        'u'  , 'DI1 = zero   ']},
    0xB1: {0x4: ['ese',   'ESE M%02x',         'u',   'sel <- [M%d]',                 'u'  , 'no DI        ']},
    0xB2: {0xF: ['etib',  'ETIB B%02x',        'u',   'B%d <- type',                  'u'  , 'no DI        ']},
    0xB3: {0xF: ['sdib',  'SDIB B%02x',        'u',   'B%d <-> DI',                   'u'  , 'DI set       ']},
    0xB4: {0x2: ['eco',   'ECO M%02x',         'u',   'cmd <- [M%d]',                 'u'  , 'no DI        ']},
    0xB6: {'*': ['sot',   'SOT A%02x,B%02x',   'uv',  'A%d - B%d + DI0',              'uv' , 'DI0,1,2 = CZH']},
    0xB7: {'*': ['andb',  'ANDB A%02x,B%02x',  'uv',  'B%d := A%d and B%d',           'vuv', 'DI1 = zero   ']},
    0xB8: {0x8: ['eda',   'EDA A%02x',         'u',   'A%d <- data.A',                'u'  , 'no DI        ']},
    0xB9: {0x0: ['enua',  'ENUA A%02x',        'u',   'A%d <- name',                  'u'  , 'no DI        ']},
    0xBA: {'*': ['sab',   'SAB A%02x,B%02x',   'uv',  'A%d <-> B%d',                  'uv' , 'no DI        ']},
    0xBB: {0xF: ['azap',  'AZAP A%02x',        'u',   'A%d.P := 0',                   'u'  , 'no DI        ']},
    0xBC: {'*': ['sll',   'SLL L%02x,L%02x',   'uv',  'L%d <-> L%d',                  'uv' , 'no DI        ']},
    0xBD: {0x0: ['comx',  'COM%d',             'u',   '  C%d',                        'u'  , 'no DI        ']},
    0xBE: {0xF: ['dcb',   'DCB B%02x',         'u',   'B%d--',                        'u'  , 'DI1 = zero   ']},
    0xC3: {0x0: ['shda',  'SHDA A%02x',        'u',   'A%d, DI0 := 0 >> A%d',         'uu' , 'DI0          '], 
           0x1: ['slda',  'SLDA A%02x',        'u',   'A%d, DI0 := DI0 >> A%d',       'uu' , 'DI0          ']},
    0xC4: {0x0: ['shsa',  'SHSA A%02x',        'u',   'DI0, A%d := A%d << 0',         'uu' , 'DI0          '], 
           0x1: ['slsa',  'SLSA A%02x',        'u',   'DI0, A%d := A%d << DI0',       'uu' , 'DI0          ']},
    0xC5: {0xF: ['tdia',  'TDIA A%02x',        'u',   'A%d := DI',                    'u'  , 'no DI        ']},
    0xC6: {'*': ['sota',  'SOTA A%02x,B%02x',  'uv',  'A%d := A%d - B%d + DI0',       'uuv', 'DI0,1,2 = CZH']},
    0xC7: {'*': ['ore',   'ORE A%02x,B%02x',   'uv',  'A%d xor B%d',                  'uv' , 'DI1 = zero   ']},
    0xC8: {'*': ['redi',  'REDI C%02x',        't',   'reset DI 0x%02x',              't'  , '...          ']},
    0xC9: {'*': ['sedi',  'SEDI C%02x',        't',   'set DI 0x%02x',                't'  , '...          ']},
    0xCA: {0x0: ['tcca',  'TCCA A%02x',        'u',   'A%d <- con',                   'u'  , 'no DI        ']},
    0xCB: {0xF: ['azbm',  'AZBM B%02x',        'u',   'B%d.M := 0',                   'u'  , 'no DI        ']},
    0xD1: {'*': ['mli',   'MLI M%02x,L%02x',   'uv',  'L%d := [M%d]',                 'vu' , 'no DI        ']},
    0xD3: {0x0: ['shdb',  'SHDB B%02x',        'u',   'B%d, DI0 := 0 >> B%d',         'uu' , 'DI0          '], 
           0x1: ['sldb',  'SLDB B%02x',        'u',   'B%d, DI0 := DI0 >> B%d',       'uu' , 'DI0          ']},
    0xD4: {0x0: ['shsb',  'SHSB B%02x',        'u',   'DI0, B%d := B%d << 0',         'uu' , 'DI0          '], 
           0x1: ['slsb',  'SLSB B%02x',        'u',   'DI0, B%d := B%d << DI0',       'uu' , 'DI0          ']},
    0xD5: {0xF: ['tdib',  'TDIB B%02x',        'u',   'B%d := DI',                    'u'  , 'no DI        ']},
    0xD6: {'*': ['sotb',  'SOTB A%02x,B%02x',  'uv',  'B%d := A%d - B%d + DI0',       'vuv', 'DI0,1,2 = CZH']},
    0xD7: {'*': ['orea',  'OREA A%02x,B%02x',  'uv',  'A%d := A%d xor B%d',           'uuv', 'DI1 = zero   ']},
    0xD8: {'*': ['tab',   'TAB A%02x,B%02x',   'uv',  'B%d := A%d',                   'vu' , 'no DI        ']},
    0xD9: {'*': ['tabp',  'TABP A%02x,B%02x',  'uv',  'B%d.P := A%d.P',               'vu' , 'no DI        ']},
    0xDA: {0x1: ['tdma',  'TDMA A%02x',        'u',   'A%d <- con.M',                 'u'  , 'no DI        ']},
    0xDB: {0xF: ['azbp',  'AZBP B%02x',        'u',   'B%d.P := 0',                   'u'  , 'no DI        ']},
    0xDD: {'*': ['mlim',  'MLIM M%02x,L%02x',  'uv',  'L%d := [M%d--]',               'vu' , 'no DI        ']},
    0xDE: {'*': ['mlip',  'MLIP M%02x,L%02x',  'uv',  'L%d := [M%d++]',               'vu' , 'no DI        ']},
    0xE0: {0x8: ['esi',   'ESI M%02x',         'u',   '[M%d] <- data/type',           'u'  , 'no DI        ']},
    0xE1: {'*': ['lmi',   'LMI M%02x,L%02x',   'uv',  '[M%d}] := L%d',                'uv' , 'no DI        ']},
    0xE2: {'*': ['lpmip', 'LPMIP M%02x,L%02x', 'uv',  '[M%d++] := ++L%d',             'uv' , 'no DI        ']},
    0xE5: {0xF: ['dcl',   'DCL L%02x',         'u',   'L%d--',                        'u'  , 'DI1 = zero   ']},
    0xE6: {'*': ['or',    'OR A%02x,B%02x',    'uv',  'A%d or B%d',                   'uv' , 'DI1 = zero   ']},
    0xE7: {'*': ['oreb',  'OREB A%02x,B%02x',  'uv',  'B%d := A%d xor B%d',           'vuv', 'DI1 = zero   ']},
    0xE8: {'*': ['tabm',  'TABM A%02x,B%02x',  'uv',  'B%d.M := A%d.M',               'vu' , 'no DI        ']},
    0xE9: {"*": ['tba',   'TBA A%02x,B%02x',   'uv',  'B%d := A%d',                   'vu' , 'no DI        ']},
    0xEA: {0x2: ['tdpa',  'TDPA A%02x',        'u',   'A%d <- con.P',                 'u'  , 'no DI        ']},
    0xEB: {0x8: ['esip',  'ESIP M%02x',        'u',   '[M%d++] <- data/type',         'u'  , 'no DI        ']},
    0xEC: {0x8: ['esim',  'ESIM M%02x',        'u',   '[M%d--] <- data/type',         'u'  , 'no DI        ']},
    0xED: {'*': ['lmim',  'LMIM M%02x,L%02x',  'uv',  '[M%d--] := L%d',               'uv' , 'no DI        ']},
    0xEE: {'*': ['lmip',  'LMIP M%02x,L%02x',  'uv',  '[M%d++] := L%d',               'uv' , 'no DI        ']},
    0xF1: {0x0: ['sei',   'SEI M%02x',         'u',   'data.BA <- [%Md]',             'u'  , 'no DI        ']},
    0xF5: {0xF: ['vrl',   'VRL L%02x',         'u',   'L%d==0',                       'u'  , 'DI1 = zero   ']},
    0xF6: {'*': ['ora',   'ORA A%02x,B%02x',   'uv',  'A%d := A%d or B%d',            'uuv', 'DI1 = zero   ']},
    0xF7: {0x0: ['seip',  'SEIP M%02x',        'u',   'data.BA <- [{M%d++]',          'u'  , 'no DI        ']},
    0xF8: {'*': ['tbap',  'TBAP A%02x,B%02x',  'uv',  'A%d.P := B%d.P',               'uv' , 'no DI        ']}, 
    0xF9: {'*': ['tbam',  'TBAM A%02x,B%02x',  'uv',  'A%d.M := B%d.M',               'uv' , 'no DI        ']}, 
    0xFA: {'*': ['tabc',  'TABC A%02x,B%02x',  'uv',  'con <- A%d,B%d',               'uv' , 'no DI        ']}, 
    0xFB: {0x8: ['dea',   'DEA L%02x',         'u',   'data.B <- B%d, A%d <- data.A', 'uu' , 'no DI        ']}, 
    0xFC: {0x0: ['dae',   'DAE L%02x',         'u',   'data.BA <- L%d',               'u'  , 'no DI        '], 
           0x2: ['cae',   'CAE L%02x',         'u',   'cmd.BA <- L%d',                'u'  , 'no DI        ']}, 
    0xFD: {0x0: ['seim',  'SEIM M%02x',        'u',   'data.BA <- [M%d--]',           'u'  , 'no DI        ']} 
}

def out(fout, s):
    fout.write(s)

def gen_decl_header(fout, desc):
    name = desc[0]
    args = desc[2]
    argdecl = ", ".join([f"u8 {var}" for var in args])
    out(fout, f"void op_{name}({argdecl});\n")

def gen_decl_method(fout, desc):
    name = desc[0]
    args = desc[2]
    com  = desc[3]
    dicom = desc[5]
    argdecl = ", ".join([f"u8 {var}" for var in args])
    out(fout, f"inline void puce_device::op_{name}({argdecl}) {{\n  //{com}\n  //{dicom}\n  op_illegal(NULL);\n}}\n\n")

def gen_decl(fout, fun):
    for _, desc in sorted(outer.items()):
        if desc is not None:
            fun(fout, desc)
    for _, mix in sorted(inner.items()):
        for _,desc in sorted(mix.items()):
            fun(fout, desc)

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
                done3 = genf(fout, desc[2], done2)
                gen_mode(fout, mode, desc)
                out(fout, " break; }\n")
            gen_illegal(fout, mode)
            out(fout, "} break;\n")
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


