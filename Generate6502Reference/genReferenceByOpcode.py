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
		

opcodes = opdict.keys()
opcodes.sort()

print "<table>"
print """<tr>
<th>Opcode</th>
<th>Instruction</th>
<th>Addressing Mode</th>
<th>Flags</th>
</tr>
"""
for op in opcodes:
	print "<tr><td>" ,
	print "$%02x" % (op) ,
	
	addressingmode = opdict[op][1]
	print "</td><td>" + opdict[op][0] + " " ,
	
	if addressingmode == "IMP":
		print "</td><td>Implied</td>" ,
	elif addressingmode == "ACC":
		print "A</td><td>Accumulator</td>" ,
	elif addressingmode == "IMM":
		print "#$NN</td><td>Immediate</td>" ,
	elif addressingmode == "ABS":
		print "$NNNN</td><td>Absolute</td>" ,
	elif addressingmode == "IND":
		print "$NN</td><td>Indirect</td>" ,
	elif addressingmode == "ABSX":
		print "$NNNN,X</td><td>Absolute,X</td>" ,
	elif addressingmode == "ABSY":
		print "$NNNN,Y</td><td>Absolute,Y</td>" ,
	elif addressingmode == "INDX":
		print "($NN,X)</td><td>Indexed Indirect</td>" ,
	elif addressingmode == "INDY":
		print "($NN),Y</td><td>Indirect Indexed</td>" ,
	elif addressingmode == "ZP":
		print "$NN</td><td>Zero Page</td>" ,
	elif addressingmode == "ZPX":
		print "$NN,X</td><td>Zero Page,X</td>" ,
	elif addressingmode == "ZPY":
		print "$NN,Y</td><td>Zero Page,Y</td>" ,
	elif addressingmode == "REL":
		print "$NN</td><td>Relative</td>" ,
		
	flags = opdict[op][4]
	
	print "<td>" + printFlags[opdict[op][4]] + "</td></tr>"

print "</table>"

