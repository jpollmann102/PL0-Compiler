/*
* PM/0
* COP 3402
* Joshua Pollmann
* jo629932
*/

#include "connector.h"

char *reservedWords[14] = {"const", "var", "procedure", "call", "begin",
                            "end", "if", "then", "else", "while", "do",
                            "read", "write", "odd"};

char specialSymbols[13] = {'+', '-', '*', '/', '(', ')', '=', ',', '.',
                            '<', '>', ';', ':'};

char *errorMessages[32]  = {"Use = instead of := ", "= must be followed by a number ", "Identifier must be followed by a number ",
                          "const, var, procedure must be followed by identifier ", "Semicolon or comma missing ", 
                          "Incorrect symbol after procedure declaration ", "Statement expected ", "Incorrect symbol after statement part in block ",
                          "Period expected ", "Semicolon between statements missing ", "Undeclared identifier ", "Assignment to constant or procedure is not allowed ",
                          "Assignment operator expected ", "call must be followed by an identifier", "Call of a constant or variable is meaningless ",
                          "then expected ", "Semicolon or end expected ", "do expected", "Incorrect symbol following statement ",
                          "Relational operator expected ", "Expression must not contain a procedure identifier ", 
                          "Right parenthesis missing ", "The preceding factor cannot begin with this symbol ", "This number is too large ",
                          "Invalid structure ", "Var already declared ", "Invalid var name ", "Invalid character ", "Expected identifier ", "Expected = ",
                          "Exceeded max lexi level ", "Program too long "};

node *createNode(int, char *);
void insertNodeRecursive(node *, char *, int);
void printListLine(node *);
int isReserved(char *);
void printListTable(node *);
void printListLineNames(node *);
int isSpecialSymbol(char);
char *fromEnum(int);
char getNextChar(FILE *);
void printLexemeTableToFile(FILE *, node *);
void printLexemeNamesToFile(FILE *, node *);

node *lex(char *filename)
{
    FILE* ifp;

    ifp = fopen(filename, "r");
    if(ifp == NULL)
    {
        printf("Error opening file\n");
        return NULL;
    }
    
    node *lexemeTable = createNode(0, "START");

    int i;
    int j = 0;
    int k = 0;
    
    int error;
    int c; 
    int comments = 0;
    int lookAhead = 0;

    char line[256];
    ifp = fopen(filename,"r");
    c = getNextChar(ifp);
    while(c != EOF)
    {
        // skip whitespace
        if(c == ' ' || c == '\t' ||c == '\r' || c == '\n')
        {
            c = getNextChar(ifp);
            lookAhead = 0;
            continue;
        }

        if(isalpha(c)){
            char characterString[MAXIDENT];
            memset(characterString, 0, sizeof characterString);
            int index = 0;
            characterString[index] = c;
            index++;
            lookAhead = 1;
            while(isalpha(c = getNextChar(ifp)) || isdigit(c)){
                if(index > MAXIDENT){
                    printf("*****Error number %d, %s\n", invalidVarName, errorMessages[invalidVarName]);
                    exit(0);
                }
                characterString[index] = c;
                index++;
            }

            // check if reserved
            int reservedSwitch = isReserved(characterString);

            // if reserved, add to table
            switch(reservedSwitch)
            {
                // const
                case 0:
                    insertNodeRecursive(lexemeTable, "const", constsym);
                    break;
                // var
                case 1:
                    insertNodeRecursive(lexemeTable, "var", varsym);
                    break;
                // procedure
                case 2:
                    insertNodeRecursive(lexemeTable, "procedure", procsym);
                    break;
                // call
                case 3:
                    insertNodeRecursive(lexemeTable, "call", callsym);
                    break;
                // begin
                case 4:
                    insertNodeRecursive(lexemeTable, "begin", beginsym);
                    break;
                // end
                case 5:
                    insertNodeRecursive(lexemeTable, "end", endsym);
                    break;
                // if
                case 6:
                    insertNodeRecursive(lexemeTable, "if", ifsym);
                    break;
                // then
                case 7:
                    insertNodeRecursive(lexemeTable, "then", thensym);
                    break;
                // else
                case 8:
                    insertNodeRecursive(lexemeTable, "else", elsesym);
                    break;
                // while
                case 9:
                    insertNodeRecursive(lexemeTable, "while", whilesym);
                    break;
                // do
                case 10:
                    insertNodeRecursive(lexemeTable, "do", dosym);
                    break;
                // read
                case 11:
                    insertNodeRecursive(lexemeTable, "read", readsym);
                    break;
                // write
                case 12:
                    insertNodeRecursive(lexemeTable, "write", writesym);
                    break;
                // odd
                case 13:
                    insertNodeRecursive(lexemeTable, "odd", oddsym);
                    break;

                default:
                    insertNodeRecursive(lexemeTable, characterString, identsym);
                    break;
            }
        }else if(isdigit(c))
        {
            char numString[MAXNUMDIGITS];
            int index = 0;
            lookAhead = 1;

            numString[index++] = c;

            while(isdigit(c = getNextChar(ifp)))
            {
                if(index + 1 > MAXNUMDIGITS)
                {
                    // number too big
                    printf("*****Error number %d, %s\n", invalidNumError, errorMessages[invalidNumError]);
                    exit(0);
                }else numString[index++] = c;
            }

            if(isalpha(c))
            {
                // this covers something like var 9foo
                printf("*****Error number %d, %s\n", invalidVarName, errorMessages[invalidVarName]);
                exit(0);
            }

            numString[index] = '\0';

            insertNodeRecursive(lexemeTable, numString, numbersym);
        }else 
        {
            lookAhead = 0;
            int special = isSpecialSymbol(c);
            // if special, add to table
            switch(special){
                // +
                case 0:
                    insertNodeRecursive(lexemeTable, "+", plussym);
                    break;
                // -
                case 1:
                    insertNodeRecursive(lexemeTable, "-", minussym);
                    break;
                // *
                case 2:
                    insertNodeRecursive(lexemeTable, "*", multsym);
                    break;

                // comments
                case 3:
                    c = getNextChar(ifp);
                    lookAhead = 1;
                    if(c == '*')
                    {
                        comments = 1;
                        lookAhead = 0;
                        c = getNextChar(ifp);
                        while(comments == 1)
                        {
                            if(c == '*')
                            {
                                c = getNextChar(ifp);
                                if(c == '/') comments = 0;
                            }
                            else c = getNextChar(ifp);
                        }
                    }
                    else
                    {
                        insertNodeRecursive(lexemeTable, "/", slashsym);
                    }
                    break;
                // (
                case 4:
                    insertNodeRecursive(lexemeTable, "(", lparentsym);
                    break;
                // )
                case 5:
                    insertNodeRecursive(lexemeTable, ")", rparentsym);
                    break;
                // =
                case 6:
                    insertNodeRecursive(lexemeTable, "=", eqsym);
                    break;
                // ,
                case 7:
                    insertNodeRecursive(lexemeTable, ",", commasym);
                    break;
                // .
                case 8:
                    insertNodeRecursive(lexemeTable, ".", periodsym);
                    break;
                // <>
                case 9:
                    c = getNextChar(ifp);
                    lookAhead = 1;
                    if(c == '>')
                    {
                        insertNodeRecursive(lexemeTable, "<>", neqsym);
                        lookAhead = 0;
                    }
                    // <=
                    else if(c == '=')
                    {
                        insertNodeRecursive(lexemeTable, "<=", leqsym);
                        lookAhead = 0;
                    }
                    //  <
                    else insertNodeRecursive(lexemeTable, "<", lessym);
                    break;
                // >=
                case 10:
                    c = getNextChar(ifp);
                    lookAhead = 1;
                    if(c == '=')
                    {
                        insertNodeRecursive(lexemeTable, ">=", geqsym);
                        lookAhead=0;
                    }
                    // >
                    else insertNodeRecursive(lexemeTable, ">", gtrsym);
                    break;
                // ;
                case 11:
                    insertNodeRecursive(lexemeTable, ";", semicolonsym);
                    break;
                // :=
                case 12:
                    c = getNextChar(ifp);
                    if(c == '=')
                    {
                        insertNodeRecursive(lexemeTable, ":=", becomessym);
                    }else
                    {
                        // invalid symbol after :
                        printf("*****Error number %d, %s\n", expectedAssignment, errorMessages[expectedAssignment]);
                        exit(0);
                    }
                    break;
                default:
                    // some other invalid symbol
                    printf("*****Error number %d, %s\n", invalidCharacter, errorMessages[invalidCharacter]);
                    exit(0);
                    break;
            }
        }

        if(lookAhead == 0)
        {
            c = getNextChar(ifp);
        }
    }

    printf("\n");

    if(lFlag)
    {
        printListLine(lexemeTable);
        printf("\n");
        printListLineNames(lexemeTable);
        printf("\n");
    }

    FILE* ofp;
    ofp = fopen("lexOutput.txt", "w");
    printLexemeTableToFile(ofp, lexemeTable);
    fclose(ofp);
    ofp = fopen("lexOutput.txt", "a");
    fprintf(ofp, "\n");
    printLexemeNamesToFile(ofp, lexemeTable);
    fclose(ofp);

    fclose(ifp);
    return lexemeTable;
}

void printLexemeTableToFile(FILE *ofp, node *root)
{
    if(root == NULL) 
    {
        fprintf(ofp, "\n");
        return;
    }
    if(root->token != 0) fprintf(ofp, "%d|", root->token);
    if(root->token == 2 || root->token == 3) fprintf(ofp, "%s|", root->var);
    printLexemeTableToFile(ofp, root->next);
}

void printLexemeNamesToFile(FILE *ofp, node *root)
{
    if(root == NULL)
    {
        fprintf(ofp, "\n");
        return;
    }
    if(root->token != 0) fprintf(ofp, "%s|", fromEnum(root->token));
    if(root->token == 2 || root->token == 3) fprintf(ofp, "%s|", root->var);
    printLexemeNamesToFile(ofp, root->next);
}

char getNextChar(FILE *ifp)
{
    char ret = fgetc(ifp);
    //printf("%c", ret);
    return ret;
}

int isReserved(char *var)
{
    int i;
    for(i = 0; i < 13; i++)
    {
        if(strcmp(var, reservedWords[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

int isSpecialSymbol(char var)
{
    int i;
    for(i = 0; i < 13; i++)
    {
        if(var == specialSymbols[i]) return i;
    }
    return -1;
}

node *createNode(int value, char *var)
{
    node *ret = malloc(sizeof(node));
    ret->token = value;
    strncpy(ret->var, var, MAXIDENT);
    ret->next = NULL;
    return ret;
}

void insertNodeRecursive(node *root, char *var, int value)
{
    if(root == NULL)
    {
        root = createNode(value, var);
        return;
    }
    if(root->next == NULL)
    {
        root->next = createNode(value, var);
        return;
    }
    insertNodeRecursive(root->next, var, value);
    return;
}

void printListLine(node *root)
{
    if(root == NULL) 
    {
        printf("\n");
        return;
    }
    if(root->token != 0) printf("%d|", root->token);
    if(root->token == 2 || root->token == 3) printf("%s|", root->var);
    printListLine(root->next);
    return;
}

void printListLineNames(node *root)
{
    if(root == NULL)
    {
        printf("\n");
        return;
    }
    if(root->token != 0) printf("%s|", fromEnum(root->token));
    if(root->token == 2 || root->token == 3) printf("%s|", root->var);
    printListLineNames(root->next);
    return;
}

void printListTable(node *root)
{
    if(root == NULL) 
    {
        printf("\n");
        return;
    }
    if(root->token != 0) printf("%s\t%d\n", root->var, root->token);
    printListTable(root->next);
    return;
}

char *fromEnum(int token)
{
    switch(token)
    {
        case 1:
            return "nulsym";
            break;
        case 2:
            return "identsym";
            break;
        case 3:
            return "numbersym";
            break;
        case 4:
            return "plussym";
            break;
        case 5:
            return "minussym";
            break;
        case 6:
            return "multsym";
            break;
        case 7:
            return "slashsym";
            break;
        case 8:
            return "oddsym";
            break;
        case 9:
            return "eqsym";
            break;
        case 10:
            return "neqsym";
            break;
        case 11:
            return "lessym";
            break;
        case 12:
            return "leqsym";
            break;
        case 13:
            return "gtrsym";
            break;
        case 14:
            return "geqsym";
            break;
        case 15:
            return "lparentsym";
            break;
        case 16:
            return "rparentsym";
            break;
        case 17:
            return "commasym";
            break;
        case 18:
            return "semicolonsym";
            break;
        case 19:
            return "periodsym";
            break;
        case 20:
            return "becomessym";
            break;
        case 21:
            return "beginsym";
            break;
        case 22:
            return "endsym";
            break;
        case 23:
            return "ifsym";
            break;  
        case 24:
            return "thensym";
            break;
        case 25:
            return "whilesym";
            break;
        case 26:
            return "dosym";
            break;
        case 27:
            return "callsym";
            break;
        case 28:
            return "constsym";
            break;
        case 29:
            return "varsym";
            break;
        case 30:
            return "procsym";
            break;
        case 31:
            return "writesym";
            break;
        case 32:
            return "readsym";
            break;
        case 33:
            return "elsesym";
            break;
    }
}