#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "lex.h"
#include "parser.h"

int main(int argc, char ** argv) {
  if(argc < 2) {
    puts("No input file or switches given.");
    puts("Switches are \"-l\" \"-a\" and \"-v\"");
    puts("Usage: ./compile [-l] [-a] [-v] prog (in any order)");
  }

  char * progname;
  char * switches[] = { "-l", "-a", "-v" };
  int switchez[3];

  switchez[0] = switchez[1] = switchez[2] = 0;

  int i;
  for(int j=1; j < argc; j++) {
    if((i = findIndex(argv[j], switches, 0, 3)) >= 0) {
      switchez[i] = 1;
    }
    else
      progname = argv[j];
  }

  if(progname == NULL) {
    puts("No input file");
    return 0;
  }

  int lo = lexscan(progname, switchez[0]);
  if(lo)
    return printerr(lo, "compiler");
  
  if(!lo) {
    FILE * lexe = fopen("lexemeList", "r");
    char ** lex;
    int lexl = 0;

    char word[15];
    
    while(fscanf(lexe, "%s", word) != EOF) lexl++;
    lex = (char **) malloc(lexl * sizeof(char *));

    rewind(lexe);

    int h = 0;
    while(fscanf(lexe, "%s", word) != EOF) {
      lex[h] = malloc((sizeof(char) * strlen(word)) + 1);
      strcpy(lex[h], word);
      h++;
    }

    rewind(lexe);
    //    printfile(lexe);

    puts("\n\n");

    if(switchez[0]) {
      puts("Lexeme List:");
      for(int i = 0; i < lexl; i++)
	printf("%s ", lex[i]);

      puts("\n");
    }

    putchar('\n');

    //    for(int i = 0; i < lexl; i++)
    //      printf("%-3d", i);


    fclose(lexe);

    //    printf("token count: %d\n", lexl);

    if(lexl == 0)
      return printerr(9, "compiler");

    int p = programp(lex, lexl);

    if(p)
      printf("Program is syntactically correct!\n\n");

    if(p && switchez[1])
      for(int i = 0; i < ci; i++)
	printf("%2d: %s %d %d\n", i, opname(code[i].op), code[i].l, code[i].m);

    if(p) {
      puts("\n\n");
      vm(code, switchez[2]);
    }
    
    for(int i = 0; i < lexl; i++)
      free(lex[i]);

    free(lex);
  }

  remove("lexemeList");
}
