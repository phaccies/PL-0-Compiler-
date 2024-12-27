/*
PROGRAM WRITTEN BY:
VERONICA VARGAS
DANIELLE ANDAL
*/

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_TOKEN_LENGTH 11
#define MAX_NUMBER_LENGTH 5
#define MAX_SYMBOL_TABLE_SIZE 500

typedef struct tk 
{ char name[1000];
  int token_value;
  int flag;
} tk;

typedef struct symbol {
  int kind;      // const = 1, var = 2, proc = 3
  char name[10]; // name up to 11 chars
  int val;       // number (ASCII value)
  int level;     // L level
  int addr;      // M address
  int mark;      // to indicate unavailable or deleted
} symbol;

// Array for tk (input token) structs.
tk arr[1000];
int s_tracker = 0;
int TK_tracker = 0;
// Array for symbol table.
symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
// Keeps track of how many instructions there are.
int instr_count = 1;
// Keeps track of address.
int address = 3;
int last_token;
int error_flag = 0;

FILE *output;

// Instruction array
int i_array[10000];
int tracker = 0;

int token_machine(char name[]);
int Symbol_Tablecheck(char name[]);
void print_instructions(int OP, int L, int M);
void emit(int OP, int L, int M);
void printSymTable();
void PROGRAM();
void BLOCK();
void CONST_DECLARATION();
int VAR_DECLARATION();
void STATEMENT();
void CONDITION();
void EXPRESSION();
void TERM();
void FACTOR();

int main(int argc, char *argv[]) 
{
  FILE *input;
  char *file_name = argv[1];;
  int tokens[1000];
  int token_tracker = 0;
  int file_copy;

  
  output = fopen("elf.txt", "w");
  
  
  // Initialize flag of each tk to 0.
  for (int k = 0; k < 1000; k++) 
  {
    arr[k].flag = 0;
  }

  // i will keep track of the index of the array.
  int i = 0;         // keeps track of the struct array
  int str = 0;       // Keeps track of the string identifier
  int isComment = 0; // Flag to see if there is a comment.

  // c will be a placeholder for scanning in characters one by one.
  char c;
  char prev_c = '\0'; // Initialize prev_c to a null character. Will hold the
                      // previous character.

  input = fopen(file_name, "r");
  // Continue reading from the file until we reach the end.
  while ((c = fgetc(input)) != EOF) // EOF -> end of file
  {
    //Print program code.
    putchar(c);
    fputc(c, output);

    // Peek at the next character.
    char next_c = fgetc(input);

    // Check if we are inside a comment block
    if (c == '/' && next_c == '*') {
      isComment = 1; // Set the flag to 1.
      continue;
    }
    ungetc(next_c, input); // Put back the next character into the input stream
    if (c == '/' && prev_c == '*' && isComment) {
      isComment = 0; // Exit the comment block.
      prev_c = '\0'; // Reset prev_c to avoid treating * as the start of another
                     // comment block.
      continue;      // Skip processing this character.
    } else if (c == '*' && prev_c == '/' && !isComment) {
      isComment = 1; // Entering comment block
      prev_c = '\0'; // Reset prev_c to avoid treating / as part of a token.
      continue;      // Skip processing this character.
    }
    if (!isComment && !isspace(c) && c != '\n' &&
        c != '\t') // If there is not a space and we are not in a comment,
                   // attach characters to the token.
    {
      // While the character is NOT a special character we connect them.
      if (isalnum(c)) 
      {
        if (!isalnum(prev_c) && !isspace(prev_c) && prev_c != '\0') 
        {
          arr[i].name[str] = '\0';
          i++;     // Move to the next token
          str = 0; // Reset the string index for the new token
        }
        arr[i].name[str++] = c; // Add the character to the current token
      } else {
        // Tokenizing special characters
        if (isalnum(prev_c) ||
            (!isalnum(prev_c) && !isspace(prev_c) && prev_c != '\0')) {

          if (prev_c == ':' && c == '=')
          {
            arr[i].name[str++] = c;
            //arr[i].name[str] = '\0';
            //i++;
            continue; // Continue to the next iteration.
          }

          if (prev_c == '>' || prev_c == '<') {
            if (c == '>') {
              arr[i].name[str++] = c;
              arr[i].name[str] = '\0';
              i++;
              str = 0;
              continue; // Continue to the next iteration.
            }
            if (c == '=') {
              arr[i].name[str++] = c;
              arr[i].name[str] = '\0';
              i++;
              str = 0;
              continue; // Continue to the next iteration.
            } else {
              arr[i].name[str++] = c;
              arr[i].name[str] = '\0';
              i++;
              str = 0;
              continue; // Continue to the next iteration.
            }
          }

          else {
            // If previous character was alphanumeric or a special char,
            // terminate current token
            arr[i].name[str] = '\0';
            i++;     // Move to the next token
            str = 0; // Reset the string index for the new token
          }
        }
        arr[i].name[str++] =
            c; // Add the special character to the current token
      }
    } else // If there is a space, we reset the variables and move on.
    {
      if (str > 0) {
        arr[i].name[str] =
            '\0'; // Null terminate the string once we reach the end.
        i++;
        str = 0; // Reset the string index for the next token.
      }
    }
    prev_c = c; // Update prev_c accordingly.
  }

  printf("\n\n");
  fprintf(output, "\n\n");

  // Add the last token if it hasn't been added yet.
  if (str > 0) {
    arr[i].name[str] = '\0'; // Null terminate the string once we reach the end.
    i++;                     // Increment i to include the last token.
  }

  // Assign token values.
  for (int n = 0; n < i; n++) {
    tokens[token_tracker] = token_machine(arr[n].name);
    arr[token_tracker].token_value = token_machine(arr[n].name);
    token_tracker++;
  }

  int i_tracker = 0;
  for (int k = 0; k < i; k++) {
    i_tracker = k + 1;
    if (strcmp(arr[k].name, "var") == 0 || strcmp(arr[k].name, "const") == 0 ||
        strcmp(arr[k].name, "procedure") == 0) {
      while (strcmp(arr[i_tracker].name, ";")) {
        if (isalpha(arr[i_tracker].name[0])) {
          arr[i_tracker].flag = 1;
        }
        i_tracker++;
      }
    }
  }

  last_token = i - 1; // Keeps track of last token.

  // Checks syntax from assignment 2.
  for (int k = 0; k < token_tracker; k++) {
    if (tokens[k] == -2) {
      printf("Error: %s Too many digits.\n", arr[k].name);
      fprintf(output, "Error: %s Too many digits.\n", arr[k].name);
      exit(0);
    } 
    else if (tokens[k] == -1) 
    {
      printf("We are at: %s\n", arr[k].name);
      printf("Error: %s invalid symbol.\n", arr[k].name);
     // exit(0);
    } else if (strlen(arr[k].name) > MAX_TOKEN_LENGTH) 
    {
      printf("Error: %s identifier name too long.\n", arr[k].name);
      exit(0);
    }
  }
  PROGRAM(arr, last_token); //Added parameters

  // Print instructions.
  printf("LINE\t\t\tOP\t\t\t\tL\t\t\tM\n");
  fprintf(output, "LINE\t\t\tOP\t\t\t\tL\t\t\tM\n");
  fprintf(output, "\n");
  printf("\n");
  //printf("%d\t\t\t\tJMP\t\t\t\t%d\t\t\t%d\n", 0, 0, 3);
  int num_line = 0;
  for (int x = 0; x < (instr_count * 3) - 3; x += 3) 
  {
    printf("%d", num_line);
    fprintf(output, "%d", num_line);
    print_instructions(i_array[x], i_array[x + 1], i_array[x + 2]); //need to make out to file
    num_line++;
  }
  printf("\n");
  fprintf(output, "\n");
// printSymTable();

  if(error_flag != 0)
  {
    printf("Error: Program contains error.\n");
    fprintf(output, "Error: Program contains error.\n");
  }
  else
  {
    printf("No errors, program is syntactically correct.\n");
    fprintf(output, "No errors, program is syntactically correct.\n");
  }
  fclose(input);
  fclose(output);
  
}

// Print symbol table.
void printSymTable() {
  printf("Kind  |  Name   |  Value  |  Level  |  Address  |  Mark\n");
  printf("------------------------------------------------------------\n");
  for (int z = 0; z < s_tracker; z++) {

    printf("  %d   |  %s    |     %d  |      %d  |      %d|         %d \n ",
           symbol_table[z].kind, symbol_table[z].name, symbol_table[z].val,
           symbol_table[z].level, symbol_table[z].addr, symbol_table[z].mark);
  }
}

// Function will return TRUE or FALSE to determine whether a given string is a
// digit or not.
bool isNumber(char name[]) {
  for (int i = 0; i < strlen(name); i++) // Loop through each character.
  {
    if (!isdigit(name[i]))
      return false; // If a given character is NOT a digit, return false; it's
                    // just a string.
  }
  return true; // If we determine every char is a digit, return true.
}

// Returns the appropriate token for the input string
int token_machine(char name[]) {

  if (strcmp(name, "ODD") == 0) {
    return 1;
  }

  // If passed a number, check it's length and return the appropriate token.

  if (isdigit(name[0])) // Checking for too many digits.
  {

    if (isNumber(name)) {
      if (strlen(name) >
          MAX_NUMBER_LENGTH) // If the string is a digit and it exceeds the
                             // digit length, return -2.
      {
        return -2;
      } else
        return 3; // Return 3 if it is a valid digit and stays in the length
                  // range.
    } else if (!isNumber(name)) {
      if (strlen(name) > MAX_TOKEN_LENGTH)
        return 0; // Return 0 if the string is too long.
      else
        return 2; // Return 2 if the string stays in the length range.
    }
  }

  // if the identifier is one char
  if (isalpha(name[0]) && name[1] == '\0') {
    return 2; // Identifier not a keyword
  }

  // if idetifier is a string
  if ((name[0] != '\0') && (name[1] != '\0')) {
    // if the string is not a special character
    if (isalpha(name[0])) {
      // we check if the string is a keyword
      if (strcmp(name, "const") != 0 && strcmp(name, "var") != 0 &&
          strcmp(name, "procedure") != 0 && strcmp(name, "call") != 0 &&
          strcmp(name, "begin") != 0 && strcmp(name, "end") != 0 &&
          strcmp(name, "if") != 0 && strcmp(name, "fi") != 0 &&
          strcmp(name, "then") != 0 &&
          //  strcmp(name, "else") != 0 &&
          strcmp(name, "while") != 0 && strcmp(name, "do") != 0 &&
          strcmp(name, "read") != 0 && strcmp(name, "write") != 0) {
        return 2;
      }
    }
  }

  if (strcmp(name, "+") == 0) {
    return 4;
  }
  if (strcmp(name, "-") == 0) {
    return 5;
  }
  if (strcmp(name, "*") == 0) {
    return 6;
  }
  if (strcmp(name, "/") == 0) {
    return 7;
  }
  if (strcmp(name, "fi") == 0) {
    return 8;
  }

  if (strcmp(name, "=") == 0) {
    return 9;
  }

  if (strcmp(name, "<>") == 0) {
    return 10;
  }
  if (strcmp(name, "<") == 0) {
    return 11;
  }
  if (strcmp(name, "<=") == 0) {
    return 12;
  }

  if (strcmp(name, ">") == 0) {
    return 13;
  }
  if (strcmp(name, ">=") == 0) {
    return 14;
  }

  if (strcmp(name, "(") == 0) {
    return 15;
  }
  if (strcmp(name, ")") == 0) {
    return 16;
  }

  if (strcmp(name, ",") == 0) {
    return 17;
  }
  if (strcmp(name, ";") == 0) {
    return 18;
  }

  if (strcmp(name, ".") == 0) {
    return 19;
  }

  if (strcmp(name, ":=") == 0) {
    return 20; // idk ab this one
  }

  if (strcmp(name, "begin") == 0) {
    return 21;
  }

  if (strcmp(name, "end") == 0) {
    return 22;
  }

  if (strcmp(name, "if") == 0) {
    return 23;
  }

  if (strcmp(name, "then") == 0) {
    return 24;
  }

  if (strcmp(name, "while") == 0) {
    return 25;
  }

  if (strcmp(name, ".") == 0) {
    return 19;
  }
  if (strcmp(name, "do") == 0) {
    return 26;
  }

  if (strcmp(name, "call") == 0) {
    return 27;
  }

  if (strcmp(name, "const") == 0) {
    return 28;
  }

  if (strcmp(name, "var") == 0) {
    return 29;
  }

  if (strcmp(name, "procedure") == 0) {
    return 30;
  }

  if (strcmp(name, "write") == 0) {
    return 31;
  }

  if (strcmp(name, "read") == 0) {
    return 32;
  }

  if (strcmp(name, "else") == 0) {
    return 33;
  }

  // return -1 if we get an undentified symbol/character
  return -1;
}

// Checks if the identifier is already in the symbol table.
int Symbol_Tablecheck(char name[]) 
{
  for (int i = 0; i < s_tracker; i++) {
    if (strcmp(symbol_table[i].name, name) == 0) {
      return i; // return index of the where the name is
    }
  }
  return -1; // Return -1 if the name is not in the symbol table.
}

void Add_Table(int kind, char *name, int val, int level) {
  if (Symbol_Tablecheck(name) == -1) {
    strcpy(symbol_table[s_tracker].name, name);
    // if arr name token is 2, then it is an identifier idk actually lol vs
    // constant
    symbol_table[s_tracker].kind = kind;
    symbol_table[s_tracker].val = val;
    symbol_table[s_tracker].level = level;
    symbol_table[s_tracker].addr = address;
    symbol_table[s_tracker].mark = 1;
    address++;   // Increment address for next use.
    s_tracker++; // Increment symbol table tracker for next use.
  }
}

//Function to print instructions that will be stored in an array.
void print_instructions(int OP, int L, int M)
{
  //LIT
  if(OP == 1)
  {
    printf("\t\t\t\tLIT\t\t\t\t%d\t\t\t%d\n", L, M);
    fprintf(output, "\t\t\t\tLIT\t\t\t\t%d\t\t\t%d\n", L, M);
  }

  //RETURN
  if(OP == 2)
  {
    if(M == 0)
    {
      printf("\t\t\t\tRTN\t\t\t\t%d\t\t\t%d\n", L, M);
      fprintf(output, "\t\t\t\tRTN\t\t\t\t%d\t\t\t%d\n", L, M);
    }
    else
    {
      //ADD
      if(M == 1)
      {
         printf("\t\t\t\tADD\t\t\t\t%d\t\t\t%d\n", L, M); 
         fprintf(output, "\t\t\t\tADD\t\t\t\t%d\t\t\t%d\n", L, M); 
      }

      //SUB
      if(M == 2)
      {
         printf("\t\t\t\tSUB\t\t\t\t%d\t\t\t%d\n", L, M); 
         fprintf(output, "\t\t\t\tSUB\t\t\t\t%d\t\t\t%d\n", L, M);
      }

      //MUL
      if(M == 3)
      {
         printf("\t\t\t\tMUL\t\t\t\t%d\t\t\t%d\n", L, M); 
         fprintf(output, "\t\t\t\tMUL\t\t\t\t%d\t\t\t%d\n", L, M); 
      }

      //DIV
      if(M == 4)
      {
         printf("\t\t\t\tDIV\t\t\t\t%d\t\t\t%d\n", L, M); 
         fprintf(output, "\t\t\t\tDIV\t\t\t\t%d\t\t\t%d\n", L, M);
      }

      //EQL
      if(M == 5)
      {
         printf("\t\t\t\tEQL\t\t\t\t%d\t\t\t%d\n",L, M);
         fprintf(output, "\t\t\t\tEQL\t\t\t\t%d\t\t\t%d\n", L, M); 
      }

      //NEQ
      if(M == 6)
      {
         printf("\t\t\t\tNEQ\t\t\t\t%d\t\t\t%d\n", L, M); 
        fprintf(output, "\t\t\t\tNEQ\t\t\t\t%d\t\t\t%d\n", L, M); 
      }

      //LSS
      if(M == 7)
      {
         printf("\t\t\t\tLSS\t\t\t\t%d\t\t\t%d\n",  L, M); 
         fprintf(output, "\t\t\t\tLSS\t\t\t\t%d\t\t\t%d\n",  L, M); 
      }

      //LEQ
      if(M == 8)
      {
         printf("\t\t\t\tLEQ\t\t\t\t%d\t\t\t%d\n", L, M); 
         fprintf(output, "\t\t\t\tLEQ\t\t\t\t%d\t\t\t%d\n", L, M); 
      }

      //GTR
      if(M == 9)
      {
         printf("\t\t\t\tGTR\t\t\t\t%d\t\t\t%d\n", L, M);
         fprintf(output, "\t\t\t\tGTR\t\t\t\t%d\t\t\t%d\n", L, M); 
      }

      //GEQ
      if(M == 10)
      {
         printf("\t\t\t\tGEQ\t\t\t\t%d\t\t\t%d\n", L, M); 
         fprintf(output, "\t\t\t\tGEQ\t\t\t\t%d\t\t\t%d\n", L, M); 
      }

      //ODD
      if(M == 11)
      {
         printf("\t\t\t\tODD\t\t\t\t%d\t\t\t%d\n", L, M); 
         fprintf(output, "\t\t\t\tODD\t\t\t\t%d\t\t\t%d\n", L, M); 
      }
    }
  }

  //LOD
  if(OP == 3)
  {
    printf("\t\t\t\tLOD\t\t\t\t%d\t\t\t%d\n", L, M);
    fprintf(output, "\t\t\t\tLOD\t\t\t\t%d\t\t\t%d\n", L, M);
  }

  //STO
  if(OP == 4)
  {
    printf("\t\t\t\tSTO\t\t\t\t%d\t\t\t%d\n",  L, M);
    fprintf(output,"\t\t\t\tSTO\t\t\t\t%d\t\t\t%d\n",  L, M);
  }

  //CAL
  if(OP == 5)
  {
    printf("\t\t\t\tCAL\t\t\t\t%d\t\t\t%d\n", L, M);
    fprintf(output, "\t\t\t\tCAL\t\t\t\t%d\t\t\t%d\n", L, M);
  }

  //INC
  if(OP == 6)
  {
    printf("\t\t\t\tINC\t\t\t\t%d\t\t\t%d\n", L, M);
    fprintf(output, "\t\t\t\tINC\t\t\t\t%d\t\t\t%d\n", L, M);
  }

  //JMP
  if(OP == 7)
  {
    printf("\t\t\t\tJMP\t\t\t\t%d\t\t\t%d\n",  L, M);
    fprintf(output, "\t\t\t\tJMP\t\t\t\t%d\t\t\t%d\n",  L, M);
  }

  //JPC
  if(OP == 8)
  {
    printf("\t\t\t\tJPC\t\t\t\t%d\t\t\t%d\n", L, M);
   fprintf(output, "\t\t\t\tJPC\t\t\t\t%d\t\t\t%d\n", L, M);
  }

  //SYS
  if(OP == 9)
  {

    //SOU
    if(M == 1)
    {
      printf("\t\t\t\tSOU\t\t\t\t%d\t\t\t%d\n", L, M); 
      fprintf(output, "\t\t\t\tSOU\t\t\t\t%d\t\t\t%d\n", L, M);
    }

    //SIN
    if(M == 2)
    {
       printf("\t\t\t\tSIN\t\t\t\t%d\t\t\t%d\n", L, M);
       fprintf(output, "\t\t\t\tSIN\t\t\t\t%d\t\t\t%d\n", L, M);
    }

    //Halt.
    if(M == 3)
    {
       printf("\t\t\t\tEOP\t\t\t\t%d\t\t\t%d\n", L, M);
       fprintf(output, "\t\t\t\tEOP\t\t\t\t%d\t\t\t%d\n", L, M);
    }
  }
}


void emit(int OP, int L, int M) 
{
  i_array[tracker++] = OP;
  i_array[tracker++] = L;
  i_array[tracker++] = M;
  instr_count++;
}

void PROGRAM(tk *arr, int last_token) 
  { 
    emit(7, 0, 6); // JMP)
    BLOCK();
    if(arr[last_token].token_value != 19)
    {
      printf("Error: Program must end with a period.\n");
      fprintf(output, "Error: Program must end with a period.\n");
    }
    emit(9, 0, 3);
  }

void PROCEDURE()
{
    emit(7, 0, 3); //change?

     Add_Table(1, arr[TK_tracker+1].name, 0, 0);
    int level = 0; //change? yes
    int space = 4; //change? yes
    TK_tracker++; 

    if(arr[TK_tracker].token_value != 2)
    {
      printf("Error: Procedure must be followed by an identifier.\n");
      fprintf(output, "Error: Procedure must be followed by an identifier.\n");
      exit(0);
    }
    TK_tracker++;
    if(strcmp(arr[TK_tracker].name, ";") != 0)
    {
      printf("Error: Procedure identifier must be followed with a semicolon.\n");
       fprintf(output,"Error: Procedure identifier must be followed with a semicolon.\n");
      exit(0);
    }
   // emit(6,0,space); //change? yes
    TK_tracker++;

    BLOCK();

  if (strcmp(arr[TK_tracker].name, "end") != 0 && arr[TK_tracker + 1].token_value != 18)
  {
    BLOCK();
  }

  else
    return;

  emit(2, 0, 0);
  //Add two since we look at end and ++.
  TK_tracker+=2;

}

void BLOCK() 
{
 //emit(7, 0, 3); //change?

  // See if we are declaring a constant.
  CONST_DECLARATION();
  // Get how many variables we are declaring.
   if(strcmp(arr[TK_tracker].name, "var") == 0)
  {
  int numVars = VAR_DECLARATION();
  emit(6, 0, numVars + 4); // INC M = 3 + numVars, per directions
  }

  while(strcmp(arr[TK_tracker].name, "procedure") == 0)
    {
      PROCEDURE();
    
     
    }
  if(strcmp(arr[TK_tracker].name, "call") == 0)
  {

    STATEMENT();
  }

  else
  {
  TK_tracker++;
  STATEMENT();
  }


  //VERONICA EDIT HERE!
  if (strcmp(arr[TK_tracker].name, "end") == 0 && arr[TK_tracker+1].token_value == 19 && TK_tracker+1 == last_token) 
  {
    emit(9, 0, 3); // halt
  }

  return;

}

void CONST_DECLARATION(char *name) 
{
  //printf("When I get here, I'm looking at %s\n", arr[TK_tracker].name);
  if (strcmp(arr[TK_tracker].name, "const") == 0) {
    do 
    {
      //Get next token
      TK_tracker++;
      if (arr[TK_tracker].token_value != 2) // If it is NOT an identifier:
      {
        printf("Error: Identifier %s not found.\n", arr[TK_tracker].name);
        fprintf(output,"Error: Identifier %s not found.\n", arr[TK_tracker].name);
        emit(9, 0, 3); // Stop the program. (Halt)
        exit(0);
      }
      if (Symbol_Tablecheck(name) != -1) 
      {
        printf("Error: Identifier already exists.\n");
        fprintf(output, "Error: Identifier already exists.\n");
        emit(9, 0, 3); // Stop the program. (Halt)
        exit(0);
      }
      TK_tracker++;
     

      //EDIT HERE!!!! 
      //If it's not '=' (Token value for = is 9)
      if(arr[TK_tracker].token_value != 9)
      {
        printf("Error: Incorrect asssignment.\n");
         fprintf(output,"Error: Incorrect asssignment.\n");
        emit(9, 0, 3); // Stop the program. (Halt)
        exit(0);
      }

      TK_tracker++;
     
      if (!isNumber(arr[TK_tracker].name)) 
      {
        printf("Error: Not a number.\n");
        fprintf(output, "Error: Not a number.\n");
        emit(9, 0, 3); // Stop the program. (Halt)
        exit(0);
      }
      // Convert to int.
      int value = atoi(arr[TK_tracker].name);
      Add_Table(1, arr[TK_tracker].name, value, 0);
      // Add to symbol table here, idk what function for that is yet.
      TK_tracker++;

    } while (strcmp(arr[TK_tracker].name, ",") == 0); // Where do while loop ends


  if(arr[TK_tracker].token_value != 18)
  {
    printf("Error: No semicolon at the end to signify end.\n");
     fprintf(output, "Error: No semicolon at the end to signify end.\n");
    exit(0);
  }

    TK_tracker++;
  }
}

int VAR_DECLARATION()
{
  int numVars = 0;
  if (strcmp(arr[TK_tracker].name, "var") == 0)
  {
    do {
      numVars++;
      TK_tracker++; // gets the next token

      if (arr[TK_tracker].token_value != 2) // If it is NOT an identifier
      {

        printf("Error: Identifier %s not found.\n", arr[TK_tracker].name);
        fprintf(output,"Error: Identifier %s not found.\n", arr[TK_tracker].name);
        emit(9, 0, 3); // Stop the program. (Halt)
        exit(0);

      }

      // doesnt make senses a bit
      /*
      if (Symbol_Tablecheck(arr[TK_tracker].name) != -1) 
      {

        printf("Is the error here?\n");
        printf("Error: Identifier %s not found.\n", arr[TK_tracker].name);
        exit(0);
      } */
      Add_Table(2, arr[TK_tracker].name, 0, 0);
      // Add to symbol table here, idk what function for that is yet.
      TK_tracker++;

    } while (arr[TK_tracker].token_value == 17); // while the token after identifier is a comma

   // printf("\n%s\n", arr[TK_tracker].name);
    if (arr[TK_tracker].token_value != 18) // 18 = ';'
    {
      printf("Error: No semicolon at the end to signify end. here\n");
      fprintf(output,"Error: No semicolon at the end to signify end. here\n");
      exit(0);
    }

    TK_tracker++;
  }
  return numVars;
}

void STATEMENT() 
{
  //printf("When we get here, we're looking at %s\n", arr[TK_tracker].name);
  if(strcmp(arr[TK_tracker].name, "call") == 0)
  {
   // Add_Table(1, arr[TK_tracker].name, 0, 1);
 // printf("Went into call\n");
    TK_tracker++;
    if(arr[TK_tracker].token_value != 2)
    {
      printf("Error: Call must be followed by an identifier.\n");
      fprintf(output,"Error: Call must be followed by an identifier.\n");
      exit(0);
    }
    TK_tracker++;
    if(arr[TK_tracker].token_value != 18)
    {
      printf("Error: Call identifier must be followed by a semicolon.\n");
      fprintf(output,"Error: Call identifier must be followed by a semicolon.\n");
      exit(0);
    }
    TK_tracker--;
    
    emit(6,0,4);
    int idx = Symbol_Tablecheck(arr[TK_tracker].name);
    emit(5,0,symbol_table[idx].addr);
    i_array[6]=symbol_table[idx].addr;
    TK_tracker++;
  }

  if (arr[TK_tracker].token_value == 2) // if it is an identifier
  {
    //printf("Identifier is: %s\n", arr[TK_tracker].name);
    // Check is identifier is valid
    int symbol_index = Symbol_Tablecheck(arr[TK_tracker].name);
    if (symbol_index == -1) {
      printf("Error: Identifier %s not found.\n", arr[TK_tracker].name);
      fprintf(output,"Error: Identifier %s not found.\n", arr[TK_tracker].name);
      exit(0);
    }

    if (arr[TK_tracker].token_value != 2) 
    {
      printf("Error: Not an identifier.\n");
      fprintf(output,"Error: Not an identifier.\n");
      exit(0);
    }

    TK_tracker++;

    if (arr[TK_tracker].token_value != 9 && arr[TK_tracker].token_value != 20) 
    {
      printf("Error: Not an assignment.\n");
      fprintf(output,"Error: Not an assignment.\n");
      exit(0);
    }

    TK_tracker++;
    // printf("Identifierfds is: %s\n ", arr[TK_tracker+1].name);
    EXPRESSION();

    //STO error is here.
    emit(4, 0, symbol_table[symbol_index].addr); // STO is here?
  }

  if (strcmp(arr[TK_tracker].name, "begin") == 0) 
  {

    do 
    { // come back
      TK_tracker++;
      STATEMENT(); // loop around the function
    } while (strcmp(arr[TK_tracker].name, "18") == 0); // while token is ';'

    TK_tracker++; // added
    /*if(strcmp(arr[TK_tracker].name, "end") != 0)
    {
      printf("Error: Needs to end.\n");
      exit(EXIT_FAILURE);
    }*/
    TK_tracker++;
  }

  // IF CASE IS HERE
  if (strcmp(arr[TK_tracker].name, "if") == 0) 
  {
    TK_tracker++;
    CONDITION();
    int jpc_index = TK_tracker;
    emit(8, 0, jpc_index);

    if (strcmp(arr[TK_tracker].name, "then") != 0) {
      printf("Error: then is needed.\n");
      fprintf(output,"Error: then is needed.\n");
      exit(0);
    }

    TK_tracker++;
    STATEMENT();
    // Added from here:
    if (strcmp(arr[TK_tracker].name, "fi") != 0) {
      printf("Error: fi is needed.\n");
      fprintf(output,"Error: fi is needed.\n");
      exit(0);
    }

    TK_tracker++;
    symbol_table[jpc_index].mark = TK_tracker;
  }



  if (strcmp(arr[TK_tracker].name, "while") == 0) 
  {
    TK_tracker++;
    int loop_index = TK_tracker;
    CONDITION();
    if (strcmp(arr[TK_tracker].name, "do") != 0) {
      printf("Error: Loop invalid.\n");
       fprintf(output,"Error: Loop invalid.\n");
      exit(0);
    }
    TK_tracker++;
    int jpc_index = TK_tracker;
    emit(8, 0, jpc_index);
    STATEMENT();
    emit(7, 0, loop_index);
    symbol_table[jpc_index].mark = TK_tracker;
  }

  if (strcmp(arr[TK_tracker].name, "read") == 0) 
  {
    TK_tracker++;
    // Check for all possible errors
    if (arr[TK_tracker].token_value == 2) {
      printf("Error: Identifier %s not found.\n", arr[TK_tracker].name);
      fprintf(output,"Error: Identifier %s not found.\n", arr[TK_tracker].name);
      exit(0);
    }
    int symbol_index = Symbol_Tablecheck(arr[TK_tracker].name);
    if (symbol_index == -1) {
      printf("Error: Identifier %s not found.\n", arr[TK_tracker].name);
      fprintf(output,"Error: Identifier %s not found.\n", arr[TK_tracker].name);
      exit(0);
    }

    if (arr[TK_tracker].token_value != 2) {
      printf("Error: Not a variable");
      fprintf(output,"Error: Not a variable");
      exit(0);
    }

    TK_tracker++;
    emit(9, 0, 2); // READ
    emit(4, 0, symbol_table[symbol_index].addr); // STORE
  }

  if (strcmp(arr[TK_tracker].name, "write") == 0) 
  {
    TK_tracker++;
    EXPRESSION();
    emit(9, 0, 1); // WRITE, prints the info on top of the stack
  }

  // Checks for period at the end of program.
  if (strcmp(arr[TK_tracker].name, ".") == 0 && TK_tracker == last_token) 
  {
    emit(9, 0, 3); // halt
  }

  if (strcmp(arr[last_token].name, ".") != 0) {
    printf("Error: Program must end with a period.\n");
    fprintf(output, "Error: Program must end with a period.\n");
    exit(0);
  }

  return;

}

void CONDITION() 
{
  // If it is odd:
  if (arr[TK_tracker].token_value % 2 != 0) {
    TK_tracker++;
    EXPRESSION();
    emit(2, 0, 11); // 11 = OPERATION CODE
  } else {
    EXPRESSION();
    if (arr[TK_tracker].token_value == 9) {
      TK_tracker++;
      EXPRESSION();
      emit(2, 0, 5);
    } else if (arr[TK_tracker].token_value == 10) {
      TK_tracker++;
      EXPRESSION();
      emit(2, 0, 6); // NOT EQUAL OPERATION CODE
    } else if (arr[TK_tracker].token_value == 11) {
      TK_tracker++;
      EXPRESSION();
      emit(2, 0, 7);
    } else if (arr[TK_tracker].token_value == 12) {
      TK_tracker++;
      EXPRESSION();
      emit(2, 0, 8);
    } else if (arr[TK_tracker].token_value == 13) {
      TK_tracker++;
      EXPRESSION();
      emit(2, 0, 9);
    } else if (arr[TK_tracker].token_value == 14) {
      TK_tracker++;
      EXPRESSION();
      emit(2, 0, 10);
    } else {
      printf("Error: Invalid condition.\n");
      fprintf(output, "Error: Invalid condition.\n");
      exit(0);
    }
  }
}

void EXPRESSION()
{
  if (arr[TK_tracker].token_value == 5) // MINUS
  {
    TK_tracker++;
    TERM();
    emit(2, 0, 2); // SUBTRACT OP , NEG???

    while (arr[TK_tracker].token_value == 4 ||
           arr[TK_tracker].token_value == 5) {
      if (arr[TK_tracker].token_value == 4) {
        TK_tracker++;
        TERM();
        emit(2, 0, 1); // ADD OP
      } else {
        TK_tracker++;
        TERM();
        emit(2, 0, 2); // SUBTRACT OP
      }
    }
  }
  else 
  {
    if (arr[TK_tracker].token_value == 4) 
    {
      TK_tracker++;
    }
    TERM();//danna
    while (arr[TK_tracker].token_value == 4 ||  arr[TK_tracker].token_value == 5) 
    {
      if (arr[TK_tracker].token_value == 4) 
      {
        TK_tracker++;
        TERM();
        emit(2, 0, 1); // ADD OP)
      } else {
        TK_tracker++;
        TERM();
        emit(2, 0, 2); // SUBTRACT OP
      }
    }
  }
  return;
}

void TERM() 
{
  FACTOR();
  while (arr[TK_tracker].token_value == 6 || arr[TK_tracker].token_value == 7 || arr[TK_tracker].token_value == 11) {
    if (arr[TK_tracker].token_value == 6) 
    {
      TK_tracker++; //* was here
      FACTOR();
      emit(2, 0, 3); // MULTIPLY OP
    } 
    else if (arr[TK_tracker].token_value == 7) 
    {

      TK_tracker++;
      FACTOR();
      emit(2, 0, 4); // DIVIDE OP
    } 
    else 
    {
      TK_tracker++;
      FACTOR();
      emit(2, 0, 11); // MOD OP
    }
  }

  return; // added this
}

void FACTOR() 
{
  // if we have an identifier
  if (arr[TK_tracker].token_value == 2) 
  {
    // Check if Identifier is valid
    int symbol_idx = Symbol_Tablecheck(arr[TK_tracker].name);
    if (symbol_idx == -1) {
      printf("Error: Identifier %s not found.\n", arr[TK_tracker].name);
      fprintf(output,"Error: Identifier %s not found.\n", arr[TK_tracker].name);
      exit(0);
    }
    if (symbol_table[symbol_idx].kind == 1) // if it is a constant)
    {
      emit(1, 0, symbol_table[symbol_idx].val); // LIT
    } 
    else// its a var
    {

      emit(3, 0, symbol_table[symbol_idx].addr); // LOD
    }
    TK_tracker++;
  }
  else if (arr[TK_tracker].token_value == 3) {

    emit(1, 0, atoi(arr[TK_tracker].name)); // i'm here lol
    TK_tracker++;
  } 
  else if (arr[TK_tracker].token_value == 15)
  {
    TK_tracker++;
    EXPRESSION();
    if (arr[TK_tracker].token_value != 16) {
      printf("Error: Missing Right Parenthesis.\n");
      fprintf(output,"Error: Missing Right Parenthesis.\n");
      exit(0);
    }
    TK_tracker++;
  } else 
  {
    printf("Error: Invalid Factor.\n");
    fprintf(output,"Error: Invalid Factor.\n");
    exit(0);
  }

  return; // added this
}