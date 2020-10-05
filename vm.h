#include <stdio.h>

#define MAX_DATA_STACK_HEIGHT 1000

typedef struct {
  int op; // opcode
  int l; // lexicographic level
  int m; // modifier
} instr;

// for use in printing activation records in the stack
// is char array because char is smallest (1 byte), and we only need boolean values 1 and 0
char ar[MAX_DATA_STACK_HEIGHT];

// memory
int stack[MAX_DATA_STACK_HEIGHT];
// instr code[MAX_CODE_LENGTH];

// registers
int sp = MAX_DATA_STACK_HEIGHT;
int bp = MAX_DATA_STACK_HEIGHT - 1; // sp - 1 causes compilation errors
int pc = 0;
instr ir = (instr) {0,0,0}; // equivalent of 0, 
// we could use masking and keep ir as int

// l stand for L in the instruction format
int base(int l, int base) {
  int b1 = base; // find base L levels down
  while (l > 0) {
    b1 = stack[b1 - 1];
    l--;
  }
  return b1;
}

char * opname(int op) {
  switch(op) {
  case 1: return "lit"; break;
  case 2: return "opr"; break;
  case 3: return "lod"; break;
  case 4: return "sto"; break;
  case 5: return "cal"; break;
  case 6: return "inc"; break;
  case 7: return "jmp"; break;
  case 8: return "jpc"; break;
  case 9: return "sio"; break;
  case 10: return "sio"; break;
  case 11: return "sio"; break;
  default: return "unk"; break;
  }
}

// pretty print instructions for 1st part of output
// also stores instructions in code array

/*
void pprint_instrs(FILE * instrs, int ccode) {
  char line[50];
  int i = 0;

  puts("Line  OP    L    M");
  while ((fgets(line, 50, instrs)) != NULL) {
    sscanf(line, "%d %d %d", &code[i].op, &code[i].l, &code[i].m);
    if (ccode)
      printf("%3d %4s %4d %4d\n", i, opname(code[i].op), code[i].l, code[i].m);
    i++;
  }
}
*/

void vm(instr * code, int runtime) {
 
  //  FILE * text = fopen(prog, "r");
  //  pprint_instrs(text, ccode);
  //  fclose(text);

  // 2nd part of output
  if(runtime) {
    printf("%25s %6s %6s %6s\n", "pc", "bp", "sp", "stack");
    printf("%-18s %6d %6d %6d\n", "Initial values", pc, bp, sp);
  }

  int run = 1;

  while(run) {
    // fetch
    ir = code[pc++];

    if(runtime)
      printf("%2d %3s %2d %2d", pc-1, opname(ir.op), ir.l, ir.m);

    // execute
    switch(ir.op) {

      // lit
    case 1: stack[--sp] = ir.m; break;

      // opr
    case 2:
      switch (ir.m) {
      case 0: sp = bp + 1; pc = stack[sp - 4]; bp = stack[sp - 3]; break; // ret
      case 1: stack[sp] = 0 - stack[sp]; break;                           // neg
      case 2: sp++; stack[sp] = stack[sp] + stack[sp-1]; break;           // add
      case 3: sp++; stack[sp] = stack[sp] - stack[sp-1]; break;           // sub
      case 4: sp++; stack[sp] = stack[sp] * stack[sp-1]; break;           // mul
      case 5: sp++; stack[sp] = stack[sp] / stack[sp-1]; break;           // div
      case 6: stack[sp] = stack[sp] % 2; break;                           // odd
      case 7: sp++; stack[sp] = stack[sp] % stack[sp-1]; break;           // mod
      case 8: sp++; stack[sp] = stack[sp] == stack[sp-1]; break;          // eql
      case 9: sp++; stack[sp] = stack[sp] != stack[sp-1]; break;          // neq
      case 10: sp++; stack[sp] = stack[sp] < stack[sp-1]; break;          // lss
      case 11: sp++; stack[sp] = stack[sp] <= stack[sp-1]; break;         // leq
      case 12: sp++; stack[sp] = stack[sp] > stack[sp-1]; break;          // gtr
      case 13: sp++; stack[sp] = stack[sp] >= stack[sp-1]; break;         // geq
      }

      break;

      // lod
    case 3:
      stack[--sp] = stack[base(ir.l, bp) - ir.m];
      break;

      // sto
    case 4:
      stack[base(ir.l, bp) - ir.m] = stack[sp++];
      break;

      // cal
    case 5:
      stack[sp-1] = 0;
      stack[sp-2] = base(ir.l, bp);
      stack[sp-3] = bp;
      stack[sp-4] = pc;
      bp = sp - 1;
      pc = ir.m;

      ar[sp] = 1;
      break;

      // inc
    case 6:
      sp = sp - ir.m;
      break;

      // jmp
    case 7:
      pc = ir.m;
      break;

      // jpc
    case 8:
      if (stack[sp] == 0)
	pc = ir.m;
      else
	sp++;

      break;

      // sio (m=1)
    case 9:
      //sp++;
      printf("\n%d", stack[sp++]);
      break;

      // sio (m=2)
    case 10:
      printf("enter a number: ");
      scanf("%d", stack + (--sp));
      break;

      // sio (m=3) aka halt
    case 11:
      run = 0;
      sp = MAX_DATA_STACK_HEIGHT;
      break;
    }

    // if ir.op != 5 = cal
    if(ar[sp] && ir.op != 5)
      ar[sp] = 0;

    if(runtime)
      printf("%13d %6d %6d", pc, bp, sp);

    // print stack
    if(runtime)
      printf("%2s", "");

    if(runtime)
      for(int i = MAX_DATA_STACK_HEIGHT - 1; i >= sp; i--)
	printf(sp != i && ar[i] ? "%d | " : "%d ", stack[i]);

    if(runtime)
      putchar('\n');
  }
}
