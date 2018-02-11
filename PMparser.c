/*
* PM/0
* COP 3402
* Joshua Pollmann
* jo629932
*/

#include "connector.h"

#define MAX_SYMBOL_TABLE_SIZE 1000

typedef struct symbol
{
	int kind;				// const = 1, var = 2, proc = 3
	char name[MAXIDENT];	// name
	int val;				// number (ASCII val)
	int level;				// L level
	int addr;				// M address
} symbol;

void program();
void block(int);
void statement(int);
void expression();
void condition();
void term();
void factor();
int isRelational(int);
void writeCodeToFile(FILE *);
void emit(int, int , int, int);
int getPosition(int, char *);

int codeIndex;
int regPtr;
int errorFlag = 0;
int tableIndex;
symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
instruction *code[MAX_CODE_LENGTH];
node *toParse;
FILE* ofp;

void parse(node *fromScanner)
{
	if(fromScanner == NULL)
	{
		// there was some error in the scanning process, return
		return;
	}

	toParse = fromScanner->next;
	int i;
	for(i = 0; i < MAX_CODE_LENGTH; i++)
	{
		code[i] = malloc(sizeof(instruction));
		code[i]->op = 0;
		code[i]->r = 0;
		code[i]->l = 0;
		code[i]->m = 0;
	}

	regPtr = 0;
	codeIndex = 0;
	tableIndex = 0;

	program();
	ofp = fopen("parseOutput.txt", "w");
	writeCodeToFile(ofp);
	fclose(ofp);

	if(aFlag && !errorFlag)
	{
		int i;
		for(i = 0; i < codeIndex; i++)
		{
			printf("%d\t%d\t%d\t%d\n", code[i]->op, code[i]->r,
						code[i]->l, code[i]->m);
		}
		printf("\n");
	}
	
	if(!errorFlag) printf("No errors, program is syntactically correct\n\n");
}

void program()
{
	block(0);
	if(toParse->token != periodsym)
	{
		// error, no period at end of program
		printf("*****Error number %d, %s\n", expectedPeriod, errorMessages[expectedPeriod]);
		errorFlag = 1;
		exit(0);
	}else
	{
		// end program
		emit(9, 0, 0, 3);
		code[codeIndex] = NULL;
	}
}

void block(int level)
{
	if(level > MAX_LEXI_LEVELS)
	{
		// error, too many levels
		printf("*****Error number %d, %s\n", exceededMaxLexiError, errorMessages[exceededMaxLexiError]);
		errorFlag = 1;
		exit(0);
	}

	int initialCIndex = codeIndex;
	int dataIndex;
	int lTableIndex;
	int lCodeIndex;
	int position;

	dataIndex = 4;
	lTableIndex = tableIndex;

	// inc
	// the 0 will change at the end
	emit(6, 0, 0, 0);

	if(toParse->token == constsym)
	{
		do
		{
			toParse = toParse->next;
			if(toParse->token != identsym)
			{
				// error, expected ident
				printf("*****Error number %d, %s\n", cvpFollowedById, errorMessages[cvpFollowedById]);
				errorFlag = 1;
				exit(0);
			}
			position = getPosition(level, toParse->var);
			if(position != -1)
			{
				// error, already declared
				printf("*****Error number %d, %s\n", alreadyDeclared, errorMessages[alreadyDeclared]);
				errorFlag = 1;
				exit(0);
			}
			strcpy(symbol_table[tableIndex].name, toParse->var);
			toParse = toParse->next;
			if(toParse->token != eqsym)
			{
				if(toParse->token == becomessym)
				{
					// error, expected eqsign
					printf("*****Error number %d, %s\n", equalInstead, errorMessages[equalInstead]);
					errorFlag = 1;
					exit(0);
				}else
				{
					// error, expected eqsign
					printf("*****Error number %d, %s\n", expectedEq, errorMessages[expectedEq]);
					errorFlag = 1;
					exit(0);
				}
				
			}else
			{
				toParse = toParse->next;
				if(toParse->token != numbersym)
				{
					// error, expected number
					printf("*****Error number %d, %s\n", eqFollowedByNum, errorMessages[eqFollowedByNum]);
					errorFlag = 1;
					exit(0);
				}else
				{
					symbol_table[tableIndex].kind = 1; // 1 is const
					symbol_table[tableIndex].val = atoi(toParse->var);
					symbol_table[tableIndex].level = level;
					symbol_table[tableIndex].addr = dataIndex++;
					// lit
					emit(1, regPtr++, 0, atoi(toParse->var));
					emit(4, regPtr - 1, level - symbol_table[tableIndex].level, dataIndex - 1);
					regPtr--;
					tableIndex++;
				}
			}	
			
			toParse = toParse->next;
		}while(toParse->token == commasym);

		if(toParse->token != semicolonsym)
		{
			// error, expected ;
			printf("*****Error number %d, %s\n", expectedSemiOrEnd, errorMessages[expectedSemiOrEnd]);
			errorFlag = 1;
			exit(0);
		}

		toParse = toParse->next;
	}

	if(toParse->token == varsym)
	{
		do
		{
			toParse = toParse->next;
			if(toParse->token != identsym)
			{
				// error, expected ident
				printf("*****Error number %d, %s\n", cvpFollowedById, errorMessages[cvpFollowedById]);
				errorFlag = 1;
				exit(0);
			}
			position = getPosition(level, toParse->var);
			if(position != -1)
			{
				// error, already declared
				printf("*****Error number %d, %s\n", alreadyDeclared, errorMessages[alreadyDeclared]);
				errorFlag = 1;
				exit(0);
			}
			strcpy(symbol_table[tableIndex].name, toParse->var);
			symbol_table[tableIndex].kind = 2; // 2 is var
			symbol_table[tableIndex].level = level;
			symbol_table[tableIndex].addr = dataIndex++;
			tableIndex++;
			
			toParse = toParse->next;
		}while(toParse->token == commasym);

		if(toParse->token != semicolonsym)
		{
			// error, expected ;
			printf("*****Error number %d, %s\n", expectedSemiOrEnd, errorMessages[expectedSemiOrEnd]);
			errorFlag = 1;
			exit(0);
		}
		toParse = toParse->next;
	}
	while(toParse->token == procsym)
	{
		toParse = toParse->next;
		if(toParse->token != identsym)
		{
			// error, expected ident
			printf("*****Error number %d, %s\n", cvpFollowedById, errorMessages[cvpFollowedById]);
			errorFlag = 1;
			exit(0);
		}
		int diff = codeIndex;
		strcpy(symbol_table[tableIndex].name, toParse->var);
		symbol_table[tableIndex].kind = 3; // 3 is proc
		symbol_table[tableIndex].addr = codeIndex;
		symbol_table[tableIndex].level = level;
		tableIndex++;
		// jmp
		// jump past procedure code, 0 will be changed
		emit(7, 0, 0, 0);

		toParse = toParse->next;
		if(toParse->token != semicolonsym)
		{
			// error, expected ;
			printf("*****Error number %d, %s\n", expectedSemiOrEnd, errorMessages[expectedSemiOrEnd]);
			errorFlag = 1;
			exit(0);
		}else
		{
			toParse = toParse->next;
			block(level + 1);
			statement(level + 1);
			if(toParse->token != semicolonsym)
			{
				// error, expected ;
				printf("*****Error number %d, %s\n", expectedSemiOrEnd, errorMessages[expectedSemiOrEnd]);
				errorFlag = 1;
				exit(0);
			}else toParse = toParse->next;
			// return
			emit(2, 0, 0, 0);
			diff = codeIndex - diff;
			code[codeIndex - diff]->m = codeIndex;
		}	
		
	}
	code[initialCIndex]->m = dataIndex;
	statement(level);
}

void statement(int level)
{
	int lCodeIndex1;
	int lCodeIndex2;
	int position;

	if(toParse->token == identsym)
	{
		position = getPosition(level, toParse->var);
		toParse = toParse->next;
		if(toParse->token != becomessym)
		{
			// error, expected :=
			printf("*****Error number %d, %s\n", expectedAssignment, errorMessages[expectedAssignment]);
			errorFlag = 1;
			exit(0);
		}
		if(symbol_table[position].kind == 1 || symbol_table[position].kind == 3)
		{
			// error, trying to assign to a const or procedure
			printf("*****Error number %d, %s\n", invalidAssignment, errorMessages[invalidAssignment]);
			errorFlag = 1;
			exit(0);
		}
		toParse = toParse->next;
		expression(level);
		if(position != -1)
		{
			// sto
			emit(4, regPtr - 1, level - symbol_table[position].level, symbol_table[position].addr);
			regPtr--;
		}
		
		
	}else if(toParse->token == callsym)
	{
		toParse = toParse->next;
		if(toParse->token != identsym)
		{
			// error, expected ident
			printf("*****Error number %d, %s\n", callFollowedById, errorMessages[callFollowedById]);
			errorFlag = 1;
			exit(0);
		}
		position = getPosition(level, toParse->var);
		if(position == -1)
		{
			// error, undeclared identifier
			printf("*****Error number %d, %s\n", undeclaredIdentifier, errorMessages[undeclaredIdentifier]);
			errorFlag = 1;
			exit(0);
		}else if(symbol_table[position].kind == 3)
		{
			// called procedure
			// cal
			emit(5, 0, level, symbol_table[position].addr + 1);
		}else
		{
			// error, meaningless call of variable
			printf("*****Error number %d, %s\n", meaninglessCall, errorMessages[meaninglessCall]);
			errorFlag = 1;
			exit(0);
		}
		
		toParse = toParse->next;
	}else if(toParse->token == beginsym)
	{
		toParse = toParse->next;
		statement(level);
		while(toParse->token == semicolonsym)
		{
			toParse = toParse->next;
			statement(level);
		}

		if(toParse->token != endsym)
		{
			// error, expected end
			printf("*****Error number %d, %s\n", expectedSemiOrEnd, errorMessages[expectedSemiOrEnd]);
			errorFlag = 1;
			exit(0);
		}

		if(toParse->next == NULL)
		{
			// error, needs to be a period if this is the last end
			errorFlag = 1;
			exit(0);
		}else toParse = toParse->next;

	}else if(toParse->token == ifsym)
	{
		toParse = toParse->next;
		condition(level);
		if(toParse->token != thensym)
		{
			// error, expected then
			printf("*****Error number %d, %s\n", expectedThen, errorMessages[expectedThen]);
			errorFlag = 1;
			exit(0);
		}
		int diff = codeIndex;
		int elseIndex;
		toParse = toParse->next;
		int tempReg = regPtr;
		// jpc
		// put 0 in for now, will be changed after statement
		emit(8, regPtr - 1, 0, 0);
		statement(level);
		diff = codeIndex - diff;
		elseIndex = codeIndex - diff;
		code[codeIndex - diff]->m = codeIndex;
		while(toParse->token == semicolonsym)
		{
			toParse = toParse->next;
			statement(level);
		}
		if(toParse->token == elsesym)
		{
			code[elseIndex]->m++;
			diff = codeIndex;
			toParse = toParse->next;
			// jmp
			// put 0 in for now, will be changed after statements
			emit(7, 0, 0, 0);
			statement(level);
			diff = codeIndex - diff;
			code[codeIndex - diff]->m = codeIndex;
		}
		
	}else if(toParse->token == whilesym)
	{
		lCodeIndex1 = codeIndex;
		toParse = toParse->next;
		condition(level);
		lCodeIndex2 = codeIndex;
		// jpc
		emit(8, regPtr - 1, 0, code[lCodeIndex1]->m);
		if(toParse->token != dosym)
		{
			// error, expected do
			printf("*****Error number %d, %s\n", expectedDo, errorMessages[expectedDo]);
			errorFlag = 1;
			exit(0);
		}
		toParse = toParse->next;
		statement(level);
		// jmp
		emit(7, 0, 0, lCodeIndex1);
		code[lCodeIndex2]->m = codeIndex;
	}else if(toParse->token == writesym)
	{
		toParse = toParse->next;
		if(toParse->token != identsym)
		{
			// error, expected ident
			printf("*****Error number %d, %s\n", expectedIdent, errorMessages[expectedIdent]);
			errorFlag = 1;
			exit(0);
		}
		position = getPosition(level, toParse->var);
	
		if(symbol_table[position].kind == 2)
		{
			// const value
			emit(3, regPtr, level - symbol_table[position].level, symbol_table[position].addr);
			// write
			emit(9, regPtr, 0, 1);
		}else if(symbol_table[position].kind == 1)
		{
			// var value
			emit(1, regPtr, 0, symbol_table[position].val);
			// write
			emit(9, regPtr, 0, 1);
		}else
		{
			// error, trying to write a procedure
			printf("*****Error number %d, %s\n", expectedIdent, errorMessages[expectedIdent]);
			errorFlag = 1;
			exit(0);
		}
		
		toParse = toParse->next;
	}else if(toParse->token == readsym)
	{
		toParse = toParse->next;
		// read
		emit(9, regPtr++, 0, 2);
		if(toParse->token != identsym)
		{
			// error, expected ident
			printf("*****Error number %d, %s\n", expectedIdent, errorMessages[expectedIdent]);
			errorFlag = 1;
			exit(0);
		}
		position = getPosition(level, toParse->var);
		if(position == -1)
		{
			// error, undeclared identifier
			printf("*****Error number %d, %s\n", undeclaredIdentifier, errorMessages[undeclaredIdentifier]);
			errorFlag = 1;
			exit(0);
		}else if(symbol_table[position].kind != 2)
		{
			// error, trying to assign to constant or procedure
			printf("*****Error number %d, %s\n", invalidAssignment, errorMessages[invalidAssignment]);
			errorFlag = 1;
			exit(0);
			position = 0;
		}
		if(position != 0)
		{
			// sto
			emit(4, regPtr - 1, level - symbol_table[position].level, symbol_table[position].addr);
		}
		
		toParse = toParse->next;
	}
}

void condition(int level)
{
	int tempToken;
	int position;

	if(toParse->token == oddsym)
	{
		toParse = toParse->next;
		expression(level);
		// odd
		emit(15, regPtr - 1, regPtr - 1, 0);
	}
	expression(level);
	if(!isRelational(toParse->token))
	{
		// error, expected relational statement
		printf("*****Error number %d, %s\n", expectedRelational, errorMessages[expectedRelational]);
		errorFlag = 1;
		exit(0);
	}else
	{
		tempToken = toParse->token;
		toParse = toParse->next;
		expression(level);
		switch(tempToken)
		{
			case 9:
				// eql i = j == k
				emit(17, regPtr - 2, regPtr - 2, regPtr - 1);
				regPtr--;
				break;
			case 10:
				// neq i = j != k
				emit(18, regPtr - 2, regPtr - 2, regPtr - 1);
				regPtr--;
				break;
			case 11:
				// lss i = j < k
				emit(19, regPtr - 2, regPtr - 2, regPtr - 1);
				regPtr--;
				break;
			case 12:
				// leq i = j <= k
				emit(20, regPtr - 2, regPtr - 2, regPtr - 1);
				regPtr--;
				break;
			case 13:
				// gtr i = j > k
				emit(21, regPtr - 2, regPtr - 2, regPtr - 1);
				regPtr--;
				break;
			case 14:
				// geq i = j >= k
				emit(22, regPtr - 2, regPtr - 2, regPtr - 1);
				regPtr--;
				break;
		}
	}
		
	
}

void expression(int level)
{
	int op;
	if(toParse->token == plussym || toParse->token == minussym)
	{
		op = toParse->token;
		toParse = toParse->next;
		term(level);
		if(op == minussym) emit(10, regPtr - 1, regPtr - 1, 0); // neg
	}else term(level);
	
	while(toParse->token == plussym || toParse->token == minussym)
	{
		op = toParse->token;
		toParse = toParse->next;
		term(level);
		if(op == minussym) // sub
		{
			emit(12, regPtr - 2, regPtr - 2, regPtr - 1);
		}else emit(11, regPtr - 2, regPtr - 2, regPtr - 1); // add

		regPtr--;
	}
}

void term(int level)
{
	int op;
	factor(level);
	while(toParse->token == multsym || toParse->token == slashsym)
	{
		op = toParse->token;
		toParse = toParse->next;
		factor(level);
		if(op == multsym) 
		{
			emit(13, regPtr - 2, regPtr - 2, regPtr - 1); // mul
		}else emit(14, regPtr - 2, regPtr - 2, regPtr - 1); // div
		regPtr--;
	}
}

void factor(int level)
{
	int position;
	if(toParse->token == identsym)
	{
		position = getPosition(level, toParse->var);
		if(position == -1)
		{
			// error, undeclared identifier
			printf("*****Error number %d, %s\n", undeclaredIdentifier, errorMessages[undeclaredIdentifier]);
			errorFlag = 1;
			exit(0);
		}
		if(symbol_table[position].kind == 2) // var
		{
			// lod
			emit(3, regPtr++, level - symbol_table[position].level, symbol_table[position].addr);
		}else if(symbol_table[position].kind == 1) // const
		{
			// lit
			emit(1, regPtr++, 0, symbol_table[position].val);
		}else
		{
			// error, procedure in expression
			printf("*****Error number %d, %s\n", cannotContainProcId, errorMessages[cannotContainProcId]);
			errorFlag = 1;
			exit(0);
		}
		
		toParse = toParse->next;
	}else if(toParse->token == 3) // number
	{
		// lit
		emit(1, regPtr++, 0, atoi(toParse->var));
		toParse = toParse->next;
	}else if(toParse->token == lparentsym)
	{
		toParse = toParse->next;
		expression(level);
		if(toParse->token != rparentsym)
		{
			// error, expected )
			printf("*****Error number %d, %s\n", missingRightPar, errorMessages[missingRightPar]);
			errorFlag = 1;
			exit(0);
		}
		toParse = toParse->next;
	}else
	{
		// error
		printf("*****Error number %d, %s\n", invalidPrecFactor, errorMessages[invalidPrecFactor]);
		errorFlag = 1;
		exit(0);
	}
}

void emit(int op, int r, int l, int m)
{
	if(codeIndex > MAX_CODE_LENGTH)
	{
		printf("*****Error number %d, %s\n", programTooLongError, errorMessages[programTooLongError]);
		errorFlag = 1;
		exit(0);
	}else
	{
		code[codeIndex]->num = codeIndex;
		code[codeIndex]->op = op;
		code[codeIndex]->r = r;
		code[codeIndex]->l = l;
		code[codeIndex]->m = m;
		codeIndex++;
	}
}

int getPosition(int level, char *id)
{
	int lTableIndex = tableIndex;
	while(lTableIndex > -1)
	{
		if(strcmp(symbol_table[lTableIndex].name, id) == 0)
		{
			if(symbol_table[lTableIndex].level <= level)
			{
				return lTableIndex;
			}
		}
		lTableIndex--;
	}
	return lTableIndex;
}

int isRelational(int token)
{
	if(token == 8)
	{
		// odd
		return 1;
	}else if(token == 10)
	{
		// neq
		return 1;
	}else if(token == 11)
	{
		// <
		return 1;
	}else if(token == 12)
	{
		// <=
		return 1;
	}else if(token == 13)
	{
		// >
		return 1;
	}else if(token == 14)
	{
		// >=
		return 1;
	}else return 0;
}

void writeCodeToFile(FILE *ofp)
{
	int op;
	int r;
	int l;
	int m;
	int i;

	for(i = 0; i < codeIndex; i++)
	{
		op = code[i]->op;
		r = code[i]->r;
		l = code[i]->l;
		m = code[i]->m;
		fprintf(ofp, "%d\t%d\t%d\t%d\n", op, r, l, m);
	}
	
}