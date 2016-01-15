#!/usr/bin/python
import os, sys, csv


#opcode,nmeonic,addressing mode,bytes,cycles

############################################################################
#  handle command line arguments and variable setup
############################################################################

if len(sys.argv) != 2:
	print "Usage: %s ops.csv" % (sys.argv[0])
	print "  where ops.csv is a csv file with the format"
	print "  opcode,nmeonic,addressing mode,bytes,cycles,flags"
	sys.exit(1)
	
ops=csv.reader(open(sys.argv[1], 'rb'))

opdict = {}
for op in ops:
	#ignore lines of formats I don't recognize
	# allows for blanks and comments
	if len(op) == 6:
		opcode=int(op[0],16)
		opdict[opcode] = [op[1], op[2], op[3], op[4], op[5]]

print opdict

opnums = opdict.keys()
opnums.sort()
for op in opnums:
	#disassemble depends on addressing mode
	addressingmode = opdict[op][1]
	if addressingmode == "IMP":
		print 'case 0x%02x: sprintf(opstr, "%s"); break;' % (op, opdict[op][0])
	elif addressingmode == "ACC":
		print 'case 0x%02x: sprintf(opstr, "%s A"); break;' % (op, opdict[op][0])
	elif addressingmode == "IMM":
		print 'case 0x%02x: sprintf(opstr, "%s %s", opcodes[1]); count = 2; break;' % (op, opdict[op][0],"#$%02x")
	elif addressingmode == "ABS":
		print 'case 0x%02x: sprintf(opstr, "%s %s", opcodes[2], opcodes[1]); count = 3; break;' % (op, opdict[op][0], "$%02x%02x")
	elif addressingmode == "IND":
		print 'case 0x%02x: sprintf(opstr, "%s (%s)", opcodes[2], opcodes[1]); count = 3; break;' % (op, opdict[op][0], "$%02x%02x")
	elif addressingmode == "ABSX":
		print 'case 0x%02x: sprintf(opstr, "%s %s,X", opcodes[2], opcodes[1]); count = 3; break;' % (op, opdict[op][0], "$%02x%02x")
	elif addressingmode == "ABSY":
		print 'case 0x%02x: sprintf(opstr, "%s %s,Y", opcodes[2], opcodes[1]); count = 3; break;' % (op, opdict[op][0], "$%02x%02x")
	elif addressingmode == "INDX":
		print 'case 0x%02x: sprintf(opstr, "%s (%s,X)", opcodes[1]); count = 2; break;' % (op, opdict[op][0], "$%02x")
	elif addressingmode == "INDY":
		print 'case 0x%02x: sprintf(opstr, "%s (%s),Y", opcodes[1]); count = 2; break;' % (op, opdict[op][0], "$%02x")
	elif addressingmode == "ZP":
		print 'case 0x%02x: sprintf(opstr, "%s %s", opcodes[1]); count = 2; break;' % (op, opdict[op][0], "$%02x")
	elif addressingmode == "ZPX":
		print 'case 0x%02x: sprintf(opstr, "%s %s,X", opcodes[1]); count = 2; break;' % (op, opdict[op][0], "$%02x")
	elif addressingmode == "ZPY":
		print 'case 0x%02x: sprintf(opstr, "%s %s,Y", opcodes[1]); count = 2; break;' % (op, opdict[op][0], "$%02x")
	elif addressingmode == "REL":
		print 'case 0x%02x: sprintf(opstr, "%s %s", opcodes[1]); count = 2; break;' % (op, opdict[op][0], "$%02x")
	else:
		print 'case 0x%02x: sprintf(opstr, "UNKNOWN"); break;' % (op)
