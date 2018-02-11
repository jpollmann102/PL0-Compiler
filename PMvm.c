/*
* PM/0
* COP 3402
* Joshua Pollmann
* jo629932
*/

#include "connector.h"

typedef struct IRegister
{
	struct instruction *ins;
} IRegister;

typedef struct Stack
{
	int arIndex[10];
	int stack[MAX_STACK_HEIGHT];
	int size;
} Stack;

int base(int, int);
char *opToString(int);
void fetch();
void execute();
void printCode(instruction *[], int);
void printStack(Stack *);
IRegister *createIR();
Stack *createStack();
node *createNode(int);
void insertNode(node *, int);
void deleteTail(node *);
void printStackToFile(FILE *, Stack *);
void printCodeToFile(FILE *, instruction*[], int);
void printInstr(instruction *);

int output;
int halt;
int SP;
int BP;
int PC;
int arIndexInt;
int registerFile[16] = {0};
int outputBuffer[MAX_CODE_LENGTH] = {0};
int oBIndex = 0;
Stack *stack;
IRegister *IR;
FILE *ofp;

void vm(instruction *code[])
{
	if(errorFlag)
	{
		// there were errors compiling, don't execute
		return;
	}

	//char line[256];
	instruction *temp;
	stack = createStack();
	IR = createIR();
	arIndexInt = 0;
	SP = 0;
	BP = 1;
	PC = 0;
	halt = 0;

	// begin program execution

	// print stack to a file
	ofp = fopen("vmOutput.txt", "w");

	fprintf(ofp, "Program:\n");
	printCodeToFile(ofp, code, 0);
	fprintf(ofp, "\n\n");
	fprintf(ofp, "Program Op Printout:\n");
	printCodeToFile(ofp, code, 1);
	fprintf(ofp, "\n\n");
	fprintf(ofp, "Stack Trace:\n");
	fprintf(ofp, "Initial Values                pc    bp    sp\n");

	if(vFlag)
	{
		printf("Program:\n");
		printCode(code, 0);
		printf("\n\n");
		printf("Program Op Printout:\n");
		printCode(code, 1);
		printf("\n\n");
		printf("Program Stack Trace:\n");
		printf("Initial Values                pc    bp    sp\n");
	}
	
	while(!halt)
	{
		fetch();
		execute();
	}

	fclose(ofp);

	printf("\n");
	int i = 0;
	while(i < oBIndex)
	{
		printf("%d\n", outputBuffer[i++]);
	}
}

void fetch()
{
	IR->ins = code[PC++];
}

void execute()
{
	int num = IR->ins->num;
	int op = IR->ins->op;
	int i = IR->ins->r;
	int j = IR->ins->l;
	int k = IR->ins->m;

	switch(op)
	{
		case 1:
			// LIT
			registerFile[i]= k;
			break;
		case 2:
			// RTN
			SP = BP - 1;
			BP = stack->stack[SP + 3];
			PC = stack->stack[SP + 4];
			break;
		case 3:
			// LOD
			registerFile[i] = stack->stack[base(j, BP) + k];
			break;
		case 4:
			// STO
			stack->stack[base(j, BP) + k] = registerFile[i];
			break;
		case 5:
			// CAL
			// call procedure at code index k
			// generates new AR
			stack->arIndex[arIndexInt++] = SP;
			stack->stack[SP + 1] = 0;				//
			stack->stack[SP + 2] = base(j, BP);		// static link
			stack->stack[SP + 3] = BP;				// dynamic link
			stack->stack[SP + 4] = PC;				// return address
			BP = SP + 1;
			PC = k;
			break;
		case 6:
			// INC
			// allocate k locals
			// 1) functional value
			// 2) static link
			// 3) dynamic link
			// 4) return address
			SP += k;
			stack->size += k;
			break;
		case 7:
			// JMP
			PC = k;
			break;
		case 8:
			// JPC
			if(registerFile[i] == 0) PC = k;
			break;
		case 9:
			// SIO
			if(k == 1)
			{
				// print
				outputBuffer[oBIndex++] = registerFile[i];
			}else if(k == 2)
			{
				// read
				printf("Enter input: ");
				scanf("%d", &registerFile[i]);
			}else if(k == 3)
			{
				// halt
				PC = 0;
				SP = 0;
				halt = 1;
				memset(stack->stack, 0, MAX_STACK_HEIGHT);
				stack->size = 0;
			}
			break;
		case 10:
			// NEG
			registerFile[i] = registerFile[j] * -1;
			break;
		case 11:
			// ADD
			registerFile[i] = registerFile[j] + registerFile[k];
			break;
		case 12:
			// SUB
			registerFile[i] = registerFile[j] - registerFile[k];
			break;
		case 13:
			// MUL
			registerFile[i] = registerFile[j] * registerFile[k];
			break;
		case 14:
			// DIV
			registerFile[i] = registerFile[j] / registerFile[k];
			break;
		case 15:
			// ODD
			registerFile[i] = registerFile[i] % 2 == 0;
			break;
		case 16:
			// MOD
			registerFile[i] = registerFile[j] % registerFile[k];
			break;
		case 17:
			// EQL
			registerFile[i] = registerFile[j] == registerFile[k];
			break;
		case 18:
			// NEQ
			registerFile[i] = registerFile[j] != registerFile[k];
			break;
		case 19:
			// LSS
			registerFile[i] = registerFile[j] < registerFile[k];
			break;
		case 20:
			// LEQ
			registerFile[i] = registerFile[j] <= registerFile[k];
			break;
		case 21:
			// GTR
			registerFile[i] = registerFile[j] > registerFile[k];
			break;
		case 22:
			// GEQ
			registerFile[i] = registerFile[j] >= registerFile[k];
			break;
	}

	if(vFlag)
	{
		printf("%-3d\t%s %2d    %-2d    %-2d    %-2d    %-2d    %-2d    ", num, opToString(op), i, j, k, PC, BP, SP);
		printStack(stack);
	}

	fprintf(ofp, "%-2d\t%s %2d    %-2d    %-2d    %-2d    %-2d    %-2d    ", num, opToString(op), i, j, k, PC, BP, SP);
	printStackToFile(ofp, stack);
}

void printStackToFile(FILE *ofp, Stack *stack)
{
	int i;
	int j = 0;
	for(i = 1; i <= SP; i++)
	{
		fprintf(ofp, "%-2d", stack->stack[i]);
		if(stack->arIndex[j] == (i))
		{
			fprintf(ofp, "%c ", '|');
			j++;
		}else fprintf(ofp, " ");
	}
	fprintf(ofp, "\n");
}

void printStack(Stack *stack)
{
	int i;
	int j = 0;
	for(i = 1; i <= SP; i++)
	{
		printf("%-2d", stack->stack[i]);
		if(stack->arIndex[j] == (i))
		{
			printf("%c ", '|');
			j++;
		}else printf(" ");
	}
	printf("\n");
}

int base(int l, int bp)
{
	int bl = bp;
	while(l > 0)
	{
		bl = stack->stack[bl + l];
		l--;
	}
	return bl;
}

char *opToString(int op)
{
	switch(op)
	{
		case 1:
			// LIT
			return "lit";
		case 2:
			// RTN
			return "rtn";
		case 3:
			// LOD
			return "lod";
		case 4:
			// STO
			return "sto";
		case 5:
			// CAL
			return "cal";
		case 6:
			// INC
			return "inc";
		case 7:
			// JMP
			return "jmp";
		case 8:
			// JPC
			return "jpc";
		case 9:
			// SIO
			return "sio";
		case 10:
			// NEG
			return "neg";
		case 11:
			// ADD
			return "add";
		case 12:
			// SUB
			return "sub";
		case 13:
			// MUL
			return "mul";
		case 14:
			// DIV
			return "div";
		case 15:
			// ODD
			return "odd";
		case 16:
			// MOD
			return "mod";
		case 17:
			// EQL
			return "eql";
		case 18:
			// NEQ
			return "neq";
		case 19:
			// LSS
			return "lss";
		case 20:
			// LEQ
			return "leq";
		case 21:
			// GTR
			return "gtr";
		case 22:
			// GEQ
			return "geq";
	}

	return "error";
}

void printCode(instruction *code[], int translated)
{
	int i = 0;
	while(code[i] != NULL)
	{
		if(translated == 1) printf("%-2d %s %-2d %-2d %-2d\n", i, opToString(code[i]->op), code[i]->r, code[i]->l, code[i]->m);
		else printf("%-2d %-2d %-2d %-2d\n", code[i]->op, code[i]->r, code[i]->l, code[i]->m);
		i++;
	}
}

void printCodeToFile(FILE *ofp, instruction *code[], int translated)
{
	int i = 0;
	while(code[i] != NULL)
	{
		if(translated == 1) fprintf(ofp, "%-2d %s %-2d %-2d %-2d\n", i, opToString(code[i]->op), code[i]->r, code[i]->l, code[i]->m);
		else fprintf(ofp, "%-2d %-2d %-2d %-2d\n", code[i]->op, code[i]->r, code[i]->l, code[i]->m);
		i++;
	}
}

void printInstr(instruction *instr)
{
	printf("%-2d %-2d %-2d %-2d\n", instr->op, instr->r, instr->l, instr->m);
}

IRegister *createIR()
{
	IRegister *ret = malloc(sizeof(IRegister));
	ret->ins = NULL;
	return ret;
}

Stack *createStack()
{
	Stack *ret = malloc(sizeof(Stack));
	memset(ret->stack, 0, MAX_STACK_HEIGHT);
	memset(ret->arIndex, 55555, 10);
	ret->size = 0;
	return ret;
}