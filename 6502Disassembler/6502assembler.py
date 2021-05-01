#!/usr/bin/env python3

import os, sys, csv
import argparse, string

parser = argparse.ArgumentParser()
parser.add_argument("--ops",      help="filename of a csv file that contains info about 6502 ops in the expected format")
parser.add_argument("--debugtokenize", help="print information useful in programming the parser", action="store_true")
parser.add_argument("--debugcodegen", help="print information useful in programming the parser", action="store_true")
parser.add_argument("--source",   help="filename of 6502 source code to assemble")
parser.add_argument("--bin",   help="filename of binary to write", default="")
parser.add_argument("--version",  help="print version of the assembler and exit", action="store_true")
args = parser.parse_args()


if args.version:
    sys.stdout.write("6502 assembler version 1.0")
    sys.exit(0)

fh = sys.stdout

######################
#  Read info from CSV file
######################

instinfo = {}
instructions = set()
with (open(args.ops, 'r', )) as f:
    ops = csv.reader(f)
    for op in ops:
        if len(op)>0 and op[0] == "opcode":
            continue
        #ignore lines of formats I don't recognize
        # allows for blanks and comments
        if len(op) == 6:
            opcode=int(op[0],16)
            instinfo[opcode] = [op[1], op[2], op[3], op[4], op[5]]

for i in instinfo:
    instructions.add(instinfo[i][0])
    
###########################################
#  Tokenize
###########################################
#
#  Instruction = ADD, ADC, DEC, PHP, etc.
#                all the ones from the csv
#                file
#  
#  whitespace is space or tab
#  delmiter is end of line
#
#  .byte is treated like operation
#
#  special handling for .byte with quoted
#   strings
#
#  classifying lines between defines, instructions, and data
#  
#  main job of this routine is to strip out most of the string handling
#   (exception is figuring out addressing mode from operand)
#

def ParseHex(hexstr):
    h = hexstr.strip()
    hint = 0
    if h.startswith('$'):
        hint = int(h[1:],16)
    elif h.startswith('#$'):
        hint = int(h[2:],16)
    elif h.startswith('#"'):
        hint = ord(h[2])
    else:
        hint = int(h)
    return hint

INSTRUCTION = 1
OPERAND     = 2
LABEL       = 3
BYTE        = 4
DATA        = 5
BASE        = 6
DEFINE      = 7
RESOLVE     = 8

def Tokenize(program, fh=sys.stdout, verbose=False):
    """program is a list of the source of a program broken up into
        a list, like will be returned from readlines() of a file
       
       expects the global "instructions" to be defined as a set of
         keywords recognized as assembler instructions

       instruction .byte handled specially
     
       Returns a dictionary with the lines tokenized.
    """
    linenum = 0
    programtokens = []
    
    while linenum < len(program):
        error = False
        if verbose: fh.write("%d: %s\n" % (linenum, program[linenum].strip()))
        # strip out comment
        linesrc = program[linenum].strip().split(';')[0].upper()
        if len(linesrc) == 0:
            linenum = linenum + 1
            continue
        line    = linesrc.split()

        if len(line) == 3 and (line[1] == '=' or line[1] == 'EQU'):
            t = [linenum, [(DEFINE, line[0], line[2])]]
            programtokens.append(t)
            if verbose: fh.write(str(t) + "\n")
            linenum = linenum + 1
            continue

        haveinstruction = False
        linetokens = []

        while len(line) > 0:
            token = line[0]
            if token[0] == '*':
                #if this is a code base change, process it as one whole chunk
                # and move to the next line
                basestr = linesrc.split('=')
                if len(basestr) != 2:
                    if verbose: fh.write("Unrecognized base define\n")
                    error = True
                else:
                    num = basestr[1].strip().split()[0]
                    linetokens.append((BASE, ParseHex(num))) 
                line =[]
            elif token == '.BYTE':
                # if this is a data definition, process all at once
                linetokens.append((BYTE, token))
                dataoffset = linesrc.find(token)+5
                datastr    = linesrc[dataoffset:].lstrip()
                if datastr[0] == '"':
                    ds = datastr
                    while ds[-1] != '"':
                        #consume lines until the end quote
                        linenum = linenum + 1
                        ds = ds + program[linenum].rstrip()
                        if verbose: fh.write("%d: (continuing) %s\n" % (linenum, program[linenum].strip()))
                    stringbytes = []
                    for char in ds:
                        if char != '"':
                            stringbytes.append(ord(char))
                    linetokens.append((DATA, stringbytes))
                    line = []
                else:
                    datastr      = datastr.split(';')[0]
                    datastr      = datastr.replace(" ", "")
                    datastr      = datastr.replace("\t", "")
                    datastr_elems = datastr.split(',')
                    data = []
                    for d in datastr_elems:
                        data.append(ParseHex(d))
                    linetokens.append((DATA, data))
                    line = []
            elif token in instructions:
                linetokens.append((INSTRUCTION, token))
                haveinstruction = True
            elif haveinstruction:
                linetokens.append((OPERAND, token))
            else:
                linetokens.append((LABEL, token))
            
            if len(line) > 0:
                #move to next token
                line.pop(0)
        
        if error:
            sys.stderr.write("An error occurred on line %d\n" % (linenum))
            sys.stderr.write(linesrc)
            sys.stderr.write("\n")
            sys.exit(1)
        
        if linetokens:
            #save the source line number for debugging
            t = [linenum,linetokens]
            if verbose: fh.write(str(t) + "\n")
            programtokens.append(t)
        linenum = linenum + 1
        
    if verbose:
        i = 0
        for line in programtokens:
            fh.write("%04d %-80s %s\n" % (i, line, program[line[0]].strip()))
            i = i + 1
    return programtokens


def FindInfo(mnemonic,addressmode, operand):
    """Figure out which opcode to use for this instruction based on
    addressing mode.  The text doesn't definitively determine the addressing
    mode, so use the text along with what is available for that mnemonic to
    decide which mode to pick.
    """
    if operand == None:
        operand = "z"
    for ii in instinfo:
        info = instinfo[ii]
        am = addressmode
        if info[0] == mnemonic:
            #['LDA', 'INDX', '2', '6', 'cZidbvN']
            if addressmode == "XXX":
                if len(operand) < 6:
                    am = "ZPX"
                else:
                    am = "ABSX"
            elif addressmode == "YYY":
                if len(operand) < 6:
                    am = "ZPY"
                else:
                    am = "ABSY"
            if (am == info[1]):
                ret = [ii]
                ret.extend(info)
                return ret

    #If I'm still here, and searching for ,x ,y modes
    # or an opcode without a operand that could be ACC, try
    # the others
    for ii in instinfo:
        info = instinfo[ii]
        am = addressmode
        if info[0] == mnemonic:
            #['LDA', 'INDX', '2', '6', 'cZidbvN']
            if addressmode == "XXX":
                am = "ABSX"
            elif addressmode == "YYY":
                am = "ABSY"
            elif addressmode == "IMP":
                am = "ACC"
            if (am == info[1]):
                ret = [ii]
                ret.extend(info)
                return ret
    return None

def HandleOperand(s, defines):
    """
    This is here mostly to handle special cases like using symbols
    with offsets "value+16".
    """
    if s[0] == '#':
        return s
    offset = 0
    suffix = ""
    base   = ""
    us = s
    
    ussplit = us.split(',')
    if len(ussplit) > 1:
        suffix = ","+ussplit[1]
    base = ussplit[0]

    if s.rfind('+') != -1:
        t = base.split('+')
        offset = int(t[1])
        base = t[0]
    elif s.rfind('-') != -1:
        t = base.split('+')
        offset = -1*int(t[1])
        base = t[0]

    if base not in defines:
        return base+suffix
    else:
        base = ParseHex(defines[base])
    
    ret = "$%x%s" % (base+offset,suffix)
    return ret

def Is2ByteBranch(op):
    branches = set([
"BCC","BCS","BEQ","BMI","BNE","BPL","BVC","BVS","BRA"
    ])
    if op in branches:
        return True
    return False

def Is3ByteBranch(op):
    branches = set([
        "JMP","JMP","JMP","JSR","BBR0","BBR1","BBR2","BBR3","BBR4","BBR5","BBR6","BBR7","BBS0","BBS1","BBS2","BBS3","BBS4","BBS5","BBS6","BBS7"
    ])
    if op in branches:
        return True
    return False

###########################################
#  GenerateCode
###########################################
#
#  Enforces the grammar onto the tokenized program
#  and generates machine code
#
#  Grammar recognizes the following lines
#
#  LABEL
#  BYTE DATA
#  LABEL BYTE DATA
#  BASE
#  INSTRUCTION
#  INSTRUCTION OPERAND
#  LABEL INSTRUCTION
#  LABEL INSTRUCTION OPERAND
#
#  This takes the tokenized
#  

def GenerateCode(lexed_program, fh=sys.stdout, verbose=False):
    """Transforms the token list from Tokenize into a list that contains
    the machine code for each line.
    """
    defines = {}
    labels  = {}
    lexidx = 0
    
    #Pass #1
    #Find labels and defines
    # labels are identifiers on the beginning of lines
    # defines are structures like
    # IOPORT  =  $34
    #  usually these are zero page registers of one byte
    #  sometimes they are absolute code addresses
    
    for line in lexed_program:
        instr = line[1]
        if instr[0][0] == DEFINE:
            d = line[1][0]
            name = instr[0][1]
            value = instr[0][2]
            defines[name] = value
        if instr[0][0] == LABEL:
            labels[instr[0][1]] = lexidx
        lexidx = lexidx + 1
    
    ir      = []  #ir is ordered
    pc      = 0
    errors  = {}
    lexedPgmIdx  = 0
    
#INSTRUCTION = 1
#OPERAND     = 2
#LABEL       = 3
#BYTE        = 4
#DATA        = 5
#BASE        = 6
#DEFINE      = 7
    #Pass #2
    # generate machine code
    # for each tokenized line
    # recognize the instruction and pick an opcode
    #   use defines{} struture to resolve symbols
    #
    while lexedPgmIdx < len(lexed_program):
        lpline = lexed_program[lexedPgmIdx]
        if verbose: fh.write (str(lpline)+"\n")
        tokens = lpline[1]
        i = 0
        while i < len(tokens):
            token = lpline[1][i]
            if (i+1 == len(tokens)):
                nexttoken = None
            else:
                nexttoken = lpline[1][i+1]

            if token[0] == LABEL:
                # Labels reference forward and backward, so I haven't seen
                # them all yet.  For now, write down the line number where the label occurs.
                # Then after all instruction addresses are recorded, I can go back
                # and give each label an address.
                labels[token[1]] = len(ir)
                i = i + 1
            elif token[0] == BYTE:
                # BYTE is just data to be put into the machine code stream
                # likely these are referenced by a label
                if nexttoken == None or nexttoken[0] != DATA:
                    errors[lexedPgmIdx] = str(lpline[0]) + " .byte not followed by data"
                    break
                pl = (pc, len(ir), DATA, nexttoken[1])
                ir.append(pl)
                pc = pc + len(nexttoken[1])
                i = i + 2
            elif token[0] == DEFINE:
                #already handled these above
                i = i + 1
            elif token[0] == BASE:
                #just reset the PC to whatever the program wants it to be
                pc = token[1]
                i = i + 1
            elif token[0] == INSTRUCTION:
                # this is a line of code
                if token[1] not in instructions:
                    i = i + 1
                    errors[lexedPgmIdx] = str(lpline[0]) + " instruction not recognized"
                    break
                mnemonic = token[1]
                addressmode = None
                if nexttoken != None and nexttoken[0] != OPERAND:
                    #if there are more tokens after INSTRUCTION, my grammar says
                    # it has to be an OPERAND, otherwise error
                    i = i + 1
                    errors[lexedPgmIdx] = str(lpline[0]) + " illegal token following instruction"
                    break
                #determine addressing mode based on the format of any operand
                if nexttoken == None:
                    addressmode = "IMP"
                else:
                    operand = nexttoken[1]
                    operand = HandleOperand(operand, defines)
                    if operand[0] == '#':
                        addressmode = "IMM"
                    elif operand[0] == '(':
                        fh.write("don't know how to handle indirect")
                        fh.write(str(lexed_program[lexedPgmIdx]))
                        sys.exit(1)
                    elif operand.endswith(",X"):
                        addressmode = "XXX"
                    elif operand.endswith(",Y"):
                        addressmode = "YYY"
                    elif Is2ByteBranch(mnemonic):
                        addressmode = "REL"
                    elif Is3ByteBranch(mnemonic):
                        addressmode = "ABS"
                    elif operand in labels:
                        addressmode = "ABS"
                    elif operand.startswith("$"):
                        if len(operand) < 4:
                            addressmode = "ZP"
                        else:
                            addressmode = "ABS"
                    else:
                        addressmode = "UNK"
                #using the mnemonic and address mode, pick a machine code
                info = FindInfo(mnemonic,addressmode, operand)
                if info == None:
                    errors[lexedPgmIdx] = str(lpline[0]) + " instruction not found for %s address mode %s" % (mnemonic, addressmode)
                    break
                numbytes = int(info[3])
                #generate code
                if numbytes == 1:
                    pl = (pc, len(ir), INSTRUCTION, [info[0]])
                elif numbytes == 2:
                    num = operand.split(',')[0]
                    if operand[0] == '$' or operand[0] == '#':
                        pl = (pc, len(ir), INSTRUCTION, [info[0], ParseHex(num)])
                    else:
                        #if a label, note it down to resolve later
                        pl = (pc, len(ir), RESOLVE, [info[0], 0], num)
                elif numbytes == 3:
                    num = operand.split(',')[0]
                    if operand[0] == '$':
                        addr = ParseHex(num)
                        pl = (pc, len(ir), INSTRUCTION, [info[0], addr & 0xff, (addr >> 8)&0xff]) # little endian order
                    else:
                        #if a label, note it down to resolve later
                        pl = (pc, len(ir), RESOLVE, [info[0], 0, 0], num)

                ir.append(pl)
                pc = pc + numbytes
                i = 100 # done with this line
            else:
                errors[lexedPgmIdx] = str(lpline[0]) + " unhandled token %s" % (str(token))
                i = i + 1

        if verbose and len(ir) > 0:
            fh.write(str(ir[-1]) + "\n")
        if len(errors) != 0:
            fh.write("exiting due to error\n")
            fh.write(str(errors) + "\n")
            fh.write(str(lexed_program[lexedPgmIdx]) + "\n")
            sys.exit(1)
        lexedPgmIdx = lexedPgmIdx + 1
    
    
    if verbose:  #debugging info
        i = 0
        for line in ir:
            fh.write("%04d %s\n" % (i, line))
            i = i + 1
    
    # Transform (Pass #3) to resolve labels
    # Now that we have the PC for every instruction, the labels
    #  can be resolved.
    # I have the line number for each lable, loop through the
    # labels, get the line number of the label and update the value to
    # the PC of that line.
    for name in labels:
        irline       = labels[name]
        irlinepc     = ir[irline][0]
        labels[name] = irlinepc
        
    # Pass #4
    # use the resolved labels to fix all the branches to labels
    ResolveBranches({ "ir" : ir, "labels" : labels})
    return ir
      
def ResolveBranches(ir):
    #each line is
    # PC, irLineNumber, [RESOLVE,INSTRUCTION], instruction bytes, optional Target Label
    for line in ir["ir"]:
        if line[2] == RESOLVE:
            if len(line[3]) == 2:
                #all 2-bytes are relative branches
                target = ir["labels"][line[4]]
                mypc   = line[0]
                offset = target-mypc
                line[3][1] = offset & 0xff
            else:
                #all 3 bytes are absolute jumps
                targetpc = ir["labels"][line[4]]
                line[3][1] = targetpc & 0xff
                line[3][2] = (targetpc >> 8) & 0xff






###############################
# I/O routines and main program
###############################
def CodeToStdout(ir, fh=sys.stdout):
    pc = ir[0][0]
    fh.write("Origin is $%x\n" % pc)
    byte = 0
    for line in ir:
        while pc < line[0]:
            fh.write("00 ")
            byte = byte + 1
            if byte == 16:
                fh.write("\n")
                byte = 0
            pc = pc + 1
        for by in line[3]:
            fh.write("%02x "% by)
            byte = byte + 1
            if byte == 16:
                fh.write("\n")
                byte = 0
            pc = pc + 1

def CodeToFile(ir, filename, fh=sys.stdout):
    pc = ir[0][0]
    fh.write("Origin is $%x\n" % pc)
    ba = []
    for line in ir:
        while pc < line[0]:
            ba.append(0)
            pc = pc + 1
        for by in line[3]:
            ba.append(by)
            pc = pc + 1
    fh = open(filename, "wb")
    fh.write(bytearray(ba))
    fh.close()
    
f = open(args.source, "r")
program = f.readlines()
f.close()

tokens = Tokenize(program, verbose=args.debugtokenize)
code = GenerateCode(tokens, verbose=args.debugcodegen)

#####################
# code is a list of 
# machine code of the format
# (PC, ignore, ignore, bytes)
# To write out to a file, have to decide how to handle this
# Most of the time the PC base won't be zero,
# so we will start writing bytes at the first PC in the file
#####################

if args.bin == "":
    CodeToStdout(code)
else:
    CodeToFile(code, args.bin)

sys.exit(0)
