/*
* PM/0
* COP 3402
* Joshua Pollmann
* jo629932
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXIDENT 11
#define MAXNUMDIGITS 5
#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3

typedef struct node
{
    char var[MAXIDENT];
    int token;
    struct node *next;
} node;

typedef struct instruction
{
	int num; // instruction number
	int op; // operation code
	int r;  // register
	int l;  // lexi level
	int m;  // number/address/identity
} instruction;

typedef enum { nulsym = 1, identsym, numbersym, plussym, minussym, multsym,  
    slashsym, oddsym, eqsym, neqsym, lessym, leqsym, gtrsym, geqsym, 
    lparentsym, rparentsym, commasym, semicolonsym, periodsym, becomessym,
    beginsym, endsym, ifsym, thensym, whilesym, dosym, callsym, constsym,
    varsym, procsym, writesym, readsym , elsesym, retsym} token_type;

typedef enum {equalInstead = 0, eqFollowedByNum, idFollowedByEq, cvpFollowedById,
              missingSemiOrComma, incorrectSymbolAfterProc, expectedStatement, incorrectSymbolAfterStatementBlock,
              expectedPeriod, missingSemiBetweenStatements, undeclaredIdentifier, invalidAssignment,
              expectedAssignment, callFollowedById, meaninglessCall, expectedThen,
              expectedSemiOrEnd, expectedDo, incorrectSymbolAfterStatement, expectedRelational,
              cannotContainProcId, missingRightPar, invalidPrecFactor, invalidNumError, 
              invalidStructError, alreadyDeclared, invalidVarName, invalidCharacter, expectedIdent, expectedEq,
              exceededMaxLexiError, programTooLongError} errorType;

extern instruction *code[MAX_CODE_LENGTH];
extern char *errorMessages[32];
extern int errorFlag;
extern int aFlag;
extern int vFlag;
extern int lFlag;

node *lex(char *);
void parse(node *);
void vm(instruction *[]);