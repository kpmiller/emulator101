#!/usr/bin/env python3
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
	print("Usage: %s ops.csv" % (sys.argv[0]))
	print("  where ops.csv is a csv file with the format")
	print("  opcode,nmeonic,addressing mode,bytes,cycles,flags")
	sys.exit(1)
	
ops=csv.reader(open(sys.argv[1], 'r'))

opdict = {}
for op in ops:
	#ignore lines of formats I don't recognize
	# allows for blanks and comments
	if len(op) == 0:
		continue
	if op[0] == 'opcode':
		continue #header
	if len(op) == 6:
		opcode=int(op[0],16)
		opdict[opcode] = [op[1], op[2], op[3], op[4], op[5]]
		

opcodes = list(opdict.keys())
opcodes.sort()

print("<table>")
print("""<tr>
<th>Opcode</th>
<th>Instruction</th>
<th>Addressing Mode</th>
<th>Flags</th>
</tr>
""")
for op in opcodes:
	print("<tr><td>", end=' ')
	print("$%02x" % (op), end=' ')
	
	addressingmode = opdict[op][1]
	print("</td><td>" + opdict[op][0] + " ", end=' ')
	
	if addressingmode == "IMP":
		print("</td><td>Implied</td>", end=' ')
	elif addressingmode == "ACC":
		print("A</td><td>Accumulator</td>", end=' ')
	elif addressingmode == "IMM":
		print("#$NN</td><td>Immediate</td>", end=' ')
	elif addressingmode == "ABS":
		print("$NNNN</td><td>Absolute</td>", end=' ')
	elif addressingmode == "IND":
		print("$NN</td><td>Indirect</td>", end=' ')
	elif addressingmode == "ABSX":
		print("$NNNN,X</td><td>Absolute,X</td>", end=' ')
	elif addressingmode == "ABSY":
		print("$NNNN,Y</td><td>Absolute,Y</td>", end=' ')
	elif addressingmode == "INDX":
		print("($NN,X)</td><td>Indexed Indirect</td>", end=' ')
	elif addressingmode == "INDY":
		print("($NN),Y</td><td>Indirect Indexed</td>", end=' ')
	elif addressingmode == "ZP":
		print("$NN</td><td>Zero Page</td>", end=' ')
	elif addressingmode == "ZPX":
		print("$NN,X</td><td>Zero Page,X</td>", end=' ')
	elif addressingmode == "ZPY":
		print("$NN,Y</td><td>Zero Page,Y</td>", end=' ')
	elif addressingmode == "REL":
		print("$NN</td><td>Relative</td>", end=' ')
		
	flags = opdict[op][4]
	
	print("<td>" + printFlags[opdict[op][4]] + "</td></tr>")

print("</table>")

