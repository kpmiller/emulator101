#!/usr/bin/python
import os, sys, csv


#opcode,nmeonic,addressing mode,bytes,cycles

printFlags = {
	'cziDbvn': '-&nbsp;-&nbsp;-&nbsp;D-&nbsp;-&nbsp;-&nbsp;', 
	'CZidbvN': 'CZ-&nbsp;-&nbsp;-&nbsp;-&nbsp;N', 
	'Czidbvn': 'C-&nbsp;-&nbsp;-&nbsp;-&nbsp;-&nbsp;-&nbsp;', 
	'CZIDBVN': 'CZIDBVN', 
	'czIdbvn': '-&nbsp;-&nbsp;I-&nbsp;-&nbsp;-&nbsp;-&nbsp;', 
	'czidbVn': '-&nbsp;-&nbsp;-&nbsp;-&nbsp;-&nbsp;V-&nbsp;', 
	'cZidbvN': '-&nbsp;Z-&nbsp;-&nbsp;-&nbsp;-&nbsp;N', 
	'czidbvn': '-&nbsp;-&nbsp;-&nbsp;-&nbsp;-&nbsp;-&nbsp;-&nbsp;', 
	'cZidbVN': '-&nbsp;Z-&nbsp;-&nbsp;-&nbsp;VN', 
	'CZidbVN': 'CZ-&nbsp;-&nbsp;-&nbsp;VN'
	}

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

# I want the instructions sorted in alphabetial order, but always
# in the same order of addressing modes.  So I'm going to append a letter
# to the nemonic, then sort that, then use that as the print order

addressModeSort = {
	"IMP" : "A",
	"IMM" : "B",
	"ACC" : "C",
	"REL" : "D",
	"ABS" : "E",
	"ABSX" : "F",
	"ABSY" : "G",
	"ZP" : "H",
	"ZPX" : "I",
	"ZPY" : "J",
	"IND" : "K",
	"INDX" : "L",
	"INDY" : "M",
}

alphainstructions = {}
opcodes = opdict.keys()

for op in opcodes:
	addressingMode = opdict[op][1]
	opname = opdict[op][0] + addressModeSort[addressingMode]
	alphainstructions[opname] = op

instructions = alphainstructions.keys()
instructions.sort()

print "<table>"
print """<tr>
<th>Instruction</th>
<th>Addressing Mode</th>
<th>Opcode</th>
<th>Flags</th>
</tr>
"""
for i in instructions:
	opcode = alphainstructions[i]
	opinfo = opdict[opcode]
	
	
	addressingmode = opinfo[1]
	
	row = "<tr><td>" + opinfo[0] + " "
	
	if addressingmode == "IMP":
		row = row + "</td><td>Implied</td>"
	elif addressingmode == "ACC":
		row = row + "A</td><td>Accumulator</td>"
	elif addressingmode == "IMM":
		row = row + "#$NN</td><td>Immediate</td>"
	elif addressingmode == "ABS":
		row = row + "$NNNN</td><td>Absolute</td>"
	elif addressingmode == "IND":
		row = row + "$NN</td><td>Indirect</td>"
	elif addressingmode == "ABSX":
		row = row + "$NNNN,X</td><td>Absolute,X</td>"
	elif addressingmode == "ABSY":
		row = row + "$NNNN,Y</td><td>Absolute,Y</td>"
	elif addressingmode == "INDX":
		row = row + "($NN,X)</td><td>Indexed Indirect</td>"
	elif addressingmode == "INDY":
		row = row + "($NN),Y</td><td>Indirect Indexed</td>"
	elif addressingmode == "ZP":
		row = row + "$NN</td><td>Zero Page</td>"
	elif addressingmode == "ZPX":
		row = row + "$NN,X</td><td>Zero Page,X</td>"
	elif addressingmode == "ZPY":
		row = row + "$NN,Y</td><td>Zero Page,Y</td>" 
	elif addressingmode == "REL":
		row = row + "$NN</td><td>Relative</td>"
	
	row = row + "<td>$%02x</td>" % (opcode)
	row = row + "<td>" + printFlags[opdict[opcode][4]] + "</td></tr>"
	print row
	 
print "</table>"

