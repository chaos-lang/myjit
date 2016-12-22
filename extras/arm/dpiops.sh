#!/bin/sh

OPCODES="AND EOR SUB RSB ADD ADC SBC RSC ORR BIC"
CMP_OPCODES="TST TEQ CMP CMN"
MOV_OPCODES="MOV MVN"

# $1: opcode list
# $2: template
gen() {
	for i in $1; do
		sed "s/<Op>/$i/g" $2.th
	done
}



echo  "/* Macros for DPI ops, auto-generated from template */\n"

echo  "\n/* mov/mvn */\n"
gen "$MOV_OPCODES" mov_macros

echo  "\n/* DPIs, arithmetic and logical */\n"
gen "$OPCODES" dpi_macros

echo  "\n\n"

echo  "\n/* DPIs, comparison */\n"
gen "$CMP_OPCODES" cmp_macros

echo  "\n/* end generated */\n"
