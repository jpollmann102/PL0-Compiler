/*****************************************************************/
			PL/0 Compiler

To compile: gcc -o pm PMdriver.c PMscanner.c PMparser.c PMvm.c

To execute a file:
	With scanner output: 	./pm -l <filename>
	With parser output:		./pm -a <filename>
	With VM output:			./pm -v <filename>
	With all:				./pm -l -a -v <filename>

The compiler outputs the results of each component to files
after every run:
	Scanner: 	lexOutput.txt
	Parser:		parseOutput.txt
	VM:			vmOutput.txt

/*****************************************************************/