/*
* PM/0
* COP 3402
* Joshua Pollmann
* jo629932
*/

#include "connector.h"

void compile(char *);

int lFlag = 0;
int aFlag = 0;
int vFlag = 0;

int main(int argc, char *argv[])
{

	if(argc < 1)
	{
		// must pass a filename
		printf("Please provide a filename to be compiled\n");
		return -1;
	}
	
	int i;
	for(i = 0; i < argc; i++)
	{
		if(strcmp(argv[i], "-l") == 0)
		{
			lFlag = 1;
		}else if(strcmp(argv[i], "-a") == 0)
		{
			aFlag = 1;
		}else if(strcmp(argv[i], "-v") == 0)
		{
			vFlag = 1;
		}
	}

	int filenameIndex = lFlag + aFlag + vFlag;
	char *filename = argv[filenameIndex + 1];

	compile(filename);
}

void compile(char *filename)
{
	parse(lex(filename));
	vm(code);
}