
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define IDENSYM 2
#define NUMSYM 3


char * tokens[] = {
   [0] = "",
   [1] = "", // null
   [2] = "", // identsym
   [3] = "", // numbersym
   [4] = "+",
   [5] = "-",
   [6] = "*",
   [7] = "/",
   [8] = "odd",
   [9] = "=",
   [10] = "!=",
   [11] = "<",
   [12] = "<=",
   [13] = ">",
   [14] = ">=",
   [15] = "(",
   [16] = ")",
   [17] = ",",
   [18] = ";",
   [19] = ".",
   [20] = ":=",
   [21] = "begin", // {
   [22] = "end",   // }
   [23] = "if",
   [24] = "then",
   [25] = "while",
   [26] = "do",
   [27] = "call",
   [28] = "const",
   [29] = "var",
   [30] = "procedure",
   [31] = "write",
   [32] = "read",
   [33] = "else"
};

int member(char c, char * str) {
  for(int i = 0; str[i] != '\0'; i++)
    if (str[i] == c)
      return 1;

  return 0;
}

void preprocess_program(FILE * in, FILE * out) {
  char c, nc;
  while((c = fgetc(in)) != EOF) {
    if(member(c, ";,()+-.")) // missing {}
      fprintf(out, " %c ", c);
    else {
      nc = fgetc(in);
      if ((c == '/' && nc == '*') ||
	  (c == '*' && nc == '/') ||
	  (c == ':' && nc == '=') ||
	  (c == '>' && nc == '=') ||
	  (c == '<' && nc == '=') ||
	  (c == '!' && nc == '='))
	fprintf(out, " %c%c ", c, nc);
      else if(member(c, "<>/*=")) {
	fprintf(out, " %c ", c);
	ungetc(nc, in);
      }
      else {
	fputc(c, out);
	ungetc(nc, in);
      }
    }
  }
}

int numberp(char * str) {
  for(int i = 0; str[i] != '\0'; i++)
    if (!member(str[i], "0123456789"))
      return 0;

  return 1;
}

void printfile(FILE * f) {
  char c;
  while((c = fgetc(f)) != EOF)
    putchar(c);
}

int lexscan(char * prog, int print) {
  int ret = 0;
  
  srand(time(0)); 

  FILE * in = fopen(prog, "r");
  FILE * pin;
  FILE * lexeme;
  
  int pinsz = strlen(prog) + 10;
  char pinn[pinsz];

  snprintf(pinn, pinsz-1, "tmp%d%d%d%d%s", 
	   rand()%10, rand()%10, rand()%10, rand()%10, prog);
  
  pin = fopen(pinn, "w");
  preprocess_program(in, pin);
  fclose(pin);
  rewind(in);
  printf("Input file: \n");
  printfile(in);
  putchar('\n');
  fclose(in);
  pin = fopen(pinn, "r");

  char * lexn = "lexemeList";
  lexeme = fopen(lexn, "w");

  if(print) {
    puts("\n\nLexeme Table:");
    printf("%-15s%-12s\n", "lexeme", "token type");
  }

  char sym[100];
  int comment = 0;

  while(1) {
  read:
    if(fscanf(pin, "%s", sym) == EOF) break;
    if(comment && strcmp(sym, "*/")) continue;

    for(int k = 0; k < 34; k++)
      if(!strcmp(tokens[k], sym)) {
	if(print) printf("%-15s%-12d\n", sym, k);
	fprintf(lexeme, "%d ", k);
	goto read;
      }

    if(!strcmp(sym, "/*"))
      comment = 1;
    else if (!strcmp(sym, "*/"))
      comment = 0;
    else if (numberp(sym)) {
      if(strlen(sym) > 5) {
	if(print) printf("number \"%s\" is too long\n", sym);
	ret = 25;
	goto leave;
      }
      else {
	if(print) printf("%-15s%-12d\n", sym, NUMSYM);
	fprintf(lexeme, "%d %s ", NUMSYM, sym);
      }
    }
    else {
      if(strlen(sym) > 11) {
	printf("Identifier \"%s\" is too long\n", sym);
	ret = 26;
	goto leave;
      }
      else if(!isalpha(sym[0])) {
	printf("Identifier \"%s\" first character is not alphabetic\n", sym);
	ret = 27;
	goto leave;
      }
      else {
	int alphanum = 1;
	for(int i = 0; sym[i] != '\0'; i++)
	  alphanum = alphanum && isalnum(sym[i]);

	if(alphanum) {
	  if(print) printf("%-15s%-12d\n", sym, IDENSYM);
	  fprintf(lexeme, "%d %s ", IDENSYM, sym);
	}
	else {
	  printf("Invalid symbol: Identifier \"%s\" is not alphanumeric\n", sym);
	  ret = 28;
	  goto leave;
	}
      }
    }
  }

  //  fclose(lexeme);
  //  lexeme = fopen(lexn, "r");
  //  puts("\n\nLexeme List:");
  //  printfile(lexeme);
 leave:
  fclose(lexeme);
  //  remove(lexn);
  fclose(pin);
  remove(pinn);
  return ret;
}
