
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SYMTAB_SIZE 10000
#define MAX_CODE_LENGTH 500
#define HSIZE 5000

#define IDENTSYM "2"
#define NUMBERSYM "3"
#define PLUSSYM "4"
#define MINUSSYM "5"
#define STARSYM "6"
#define SLASHSYM "7"
#define ODDSYM "8"
#define EQSYM "9"
#define NEQSYM "10"
#define LESYM "11"
#define LEQSYM "12"
#define GESYM "13"
#define GEQSYM "14"
#define LPARSYM "15"
#define RPARSYM "16"
#define COMMASYM "17"
#define SEMICOLSYM "18"
#define DOTSYM "19"
#define BECOMESYM "20"
#define BEGINSYM "21"
#define ENDSYM "22"
#define IFSYM "23"
#define THENSYM "24"
#define WHILESYM "25"
#define DOSYM "26"
#define CALLSYM "27"
#define CONSTSYM "28"
#define VARSYM "29"
#define PROCSYM "30"
#define WRITESYM "31"
#define READSYM "32"
#define ELSESYM "33"


typedef struct
{
  int kind;       // const = 1, var = 2, procedure = 3
  char * name;    // name up to 11 chars
  int val;        // number (ASCII value)
  int level;      // L level
  int addr;       // M address
  int mark;       // to indicate unavailable or delete d
} symbol;

symbol symtab[MAX_SYMTAB_SIZE];
instr code[MAX_CODE_LENGTH];
int l = 0, si = 0, ci = 0;

int psi[HSIZE], cvpn[HSIZE], vn[HSIZE];
int psii = 0, cvpni = 0, vni = 0;
int block = 1;

// function prototypes
char * error(int);
int programp(char **, int);
int blockp(char **, int, int, int);
int constdeclp(char **, int, int);
int vardeclp(char **, int, int);
int procdeclp(char **, int, int);
int statementp(char **, int, int);
int conditionp(char **, int, int);
int expressionp(char **, int, int);
int termp(char **, int, int);
int factorp(char **, int, int);


char * error(int err) {
  switch(err) {
  case 2: return "= must be followed by a number."; break;
  case 3: return "Identifier must be followed by =."; break;
  case 4: return "const, var, read, write , procedure and call must be followed by identifier."; break;
  case 5: return "Semicolon or comma missing."; break;
  case 7: return "Statement expected."; break;
  case 9: return "Period expected."; break;
  case 11: return "Undeclared identifier."; break;
  case 12: return "Assignment to constant is not allowed."; break;
  case 13: return "Assignment operator expected."; break;
  case 16: return "then expected after an if."; break;
  case 17: return "Semicolon or end expected."; break;
  case 18: return "do expected after a while."; break;
  case 20: return "Relational operator expected."; break;
  case 22: return "Unbalanced parentheses."; break;
  case 25: return "Number is too long."; break;
  case 26: return "Identifier is to long"; break;
  case 27: return "Identifier first character is not alphabetic."; break;
  case 28: return "Identifier is not alphanumeric"; break;
  case 29: return "Const declarations must occur before proc/var declarations and statements."; break;
  case 30: return "Var declarations must occur before proc declarations and statements."; break;
  case 31: return "Const declaration has less than 5 tokens (Minimum is 5)."; break;
  case 32: return "Var declaration has less than 3 tokens (Minimum is 3)."; break;
  case 33: return "Statement not in form \"ident := expr\""; break;
  case 35: return "Bad read/write/call statement. Must be \"(read|write|call) ident\"."; break;
  case 36: return "During expression form, term was expected but not found."; break;
  case 37: return "During termp form, factor was expected but not found."; break;
  case 38: return "Symbol already exists."; break;
  case 40: return "Too much code."; break;
  case 41: return "Condition expected but not found."; break;
  case 42: return "Wrong symbol kind."; break;
  case 44: return "Procedure expected."; break;
  case 45: return "Block expected."; break;
  case 46: return "Procedure declarations must occur before statemtements"; break;
  }
}

int printerr(int err, char * from) {
  printf("In %s, Error %d: %s\n", from, err, error(err));
  return 0;
}

int newsymp(char * name, int cl) {
  for(int lv = l; lv >= (cl ? l : 0); lv--)
    for(int i = si-1; i >= 0; i--) {
      if(!strcmp(symtab[i].name, name) && !symtab[i].mark && symtab[i].level == lv)
	return 0;
    }
  return 1;
}

symbol getsym(char * name) {
  for(int lv = l; lv >= 0; lv--)
    for(int i = si-1; i >= 0; i--)
      if(!strcmp(symtab[i].name, name) && symtab[i].level == lv && !symtab[i].mark)
	return symtab[i];
}

void printlex(char ** lex, int start, int end) {
  for(int i = start; i < end; i++) {
    if(!strcmp(lex[i], NUMBERSYM)) {
      printf("3 %s ", lex[++i]);
      continue;
    }

    if(!strcmp(lex[i], IDENTSYM)) {
      printf("2 %s ", lex[++i]);
      continue;
    }

    int k = atoi(lex[i]);
    if(!(strlen(tokens[k]) == 0))
      printf("%s ", tokens[k]);
  }
  puts("\n");
}

void printst() {
  puts("Symbol table:");
  for(int i = 0; i < si; i++)
    printf("symbol = %s, kind = %d, level = %d, addr = %d, mark = %d\n",
	   symtab[i].name, symtab[i].kind,
	   symtab[i].level, symtab[i].addr, symtab[i].mark);

  puts("");
}

void printstk() {
  printf("cvpn: ");
  for(int i = 0; i <= cvpni; i++)
    printf("%d ", cvpn[i]);

  puts("");

  printf("vn: ");
  for(int i = 0; i <= vni; i++)
    printf("%d ", vn[i]);

  puts("");
}

int findIndex(char * x, char ** list, int start, int end) {
  for(int i = start; i < end; i++) {
    if(!strcmp(list[i], NUMBERSYM)) {
      i++; continue;
    }
    if(!strcmp(x, list[i]))
      return i;
  }
  
  return -1;
}


int topMatch(char * left, char * right, char * get, char ** lex, int start, int end) {
  int bal = 0;

  for(int i = start; i < end; i++) {
    if(!strcmp(lex[i], NUMBERSYM)) {
      i++;
      continue;
    }

    if(!strcmp(lex[i], left))
      bal++;

    if(!bal)
      if(!strcmp(lex[i], get))
	return i;

    if(!strcmp(lex[i], right))
      bal--;
  }

  return -1;
}

int topProcSemi(char ** lex, int start, int end) {
  int cvb = 0;
  int s = 0;
  
  for(int i = start; i < end; i++) {
    if(!strcmp(lex[i], NUMBERSYM)) {
      i++;
      continue;
    }

    if(!strcmp(lex[i], BEGINSYM))
      s++;

    if(!strcmp(lex[i], ENDSYM))
      s--;

    if(s != 0)
      continue;

    if(!strcmp(lex[i], CONSTSYM) ||
       !strcmp(lex[i], VARSYM))
      cvb++;

    else if(cvb && !strcmp(lex[i], SEMICOLSYM))
      cvb--;

    else if(!strcmp(lex[i], PROCSYM)) {
      int j = topProcSemi(lex, i+4, end);
      if(j < 0)
	return -1;

      i = j;
    }

    else if(!cvb && !strcmp(lex[i], SEMICOLSYM))
      return i;
  }

  return -1;
}


int emit(int op, int l, int m) {
  if(ci >= MAX_CODE_LENGTH)
    return printerr(40, "emit");
  
  code[ci].op = op;
  code[ci].l = l;
  code[ci].m = m;
  ci++;
  return 1;
}

int pr(char * from, int val) {
  //      printf("%s returning %d\n", from, val);
  return val;
}

int countProcs(char ** lex, int start, int end) {
  int count = 0;
  for(int i = start; i < end; i++) {
    if(!strcmp(lex[i], NUMBERSYM)) {
      i++;
      continue;
    }

    if(!strcmp(lex[i], PROCSYM))
      count++;
  }

  return count;
}

int programp(char ** lex, int size) {
  if(strcmp(lex[size-1], DOTSYM))
    return printerr(9, "programp");

  if(!blockp(lex, 0, size-1, 0))
    return printerr(45, "programp");

  return pr("programp", emit(11, 0, 3));
}

void mark(int n) {
  for(int i = si-1; i >= 0 && n > 0; i--) {
    if(!symtab[i].mark) {
      symtab[i].mark = 1;
      n--;
    }
  }
}

int blockp(char ** lex, int start, int end, int cvs) {
  // printf("blockp: "); printlex(lex, start, end); // printstk();
  
  int P = 8;
  int C = 4;
  int V = 2;
  int S = 1;

  if(!block) {
    cvpni++;
    vni++;
    block = 1;
  }

  if(start >= end)
    goto leave;
  //    return pr("statementp", 1);

  // if token == const
  if(!strcmp(lex[start], CONSTSYM)) {
    int sc = findIndex(SEMICOLSYM, lex, start, end);
    if(sc < 0)
      return printerr(17, "blockp");
    else if(cvs & (P | V | S))
      return printerr(29, "blockp");
    else
      return pr("blockp", constdeclp(lex, start, sc+1) &&
	        blockp(lex, sc+1, end, cvs | C));
  }

  // if token == var
  if(!strcmp(lex[start], VARSYM)) {
    int sc = findIndex(SEMICOLSYM, lex, start, end);
    if(sc < 0)
      return printerr(17, "blockp");
    else if (cvs & (P | S))
      return printerr(30, "blockp");
    else
      return pr("blockp", vardeclp(lex, start, sc+1) &&
		blockp(lex, sc+1, end, cvs | V));
  }

  // if token == procedure
  if(!strcmp(lex[start], PROCSYM)) {
    if(cvs & S)
      return printerr(46, "blockp");

    int j = topProcSemi(lex, start+4, end);
    if(j < 0)
      return printerr(44, "blockp");

    if(!procdeclp(lex, start, j+1))
      return printerr(44, "blockp");

    return pr("blockp", blockp(lex, j+1, end, cvs | P));
  }
  
  // must be a statement then
  int cd = findIndex(CONSTSYM, lex, start, end);
  int vd = findIndex(VARSYM, lex, start, end);
  int pd = findIndex(PROCSYM, lex, start, end);

  if (cd >= 0)
    return printerr(29, "blockp");
  if (vd >= 0)
    return printerr(30, "blockp");
  if (pd >= 0)
    return printerr(46, "blockp");


 leave:
  emit(6, 0, 4 + vn[vni]);
  
  if(!statementp(lex, start, end))
    return printerr(7, "blockp");

  mark(cvpn[cvpni]);
  cvpn[cvpni] = 0;
  vn[vni] = 0;
  cvpni--;
  vni--;
  return pr("blockp", 1);
}

int constdeclp(char ** lex, int start, int end) {
  if(start >= end)
    return pr("constdeclp", 1);

  if((end - start) < 7)
    return printerr(31, "constdeclp");

  int j = 0, k = start;
  int c = !strcmp(lex[k++], CONSTSYM);

  while(strcmp(lex[k], SEMICOLSYM)) {
    switch(j%4) {
    case 0: if(strcmp(lex[k++], IDENTSYM))
	      return printerr(4, "constdeclp");
            else {
	      if(newsymp(lex[k], 1)) {
		symbol s;
		s.kind = 1;
		s.name = lex[k];
		s.level = l;
		s.addr = 0;
		s.mark = 0;
		symtab[si] = s;
	      }
	      else
		return printerr(38, "constdeclp");
	    }
      break;
    case 1: if(strcmp(lex[k], EQSYM)) return printerr(3, "constdeclp"); break;
    case 2: if(strcmp(lex[k++], NUMBERSYM))
  	       return printerr(2, "constdeclp");
            else
	      symtab[si++].val = atoi(lex[k]);
      cvpn[cvpni]++;
      break;
    case 3: if(strcmp(lex[k], COMMASYM)) return printerr(5, "constdeclp"); break;
    }

    k++; j++;
  }

  return pr("constdeclp", c && !strcmp(lex[k], SEMICOLSYM) && k == end-1);
}

int vardeclp(char ** lex, int start, int end) {
  if(start >= end)
    return pr("vardeclp", 1);

  if((end - start) < 4)
    return printerr(32, "vardeclp");

  int j = 0, k = start;
  int v = !strcmp(lex[k++], VARSYM);
  int vnn = 0;

  while(strcmp(lex[k], SEMICOLSYM)) {
    switch(j%2) {
    case 0: if(strcmp(lex[k++], IDENTSYM))
	      return printerr(4, "vardeclp"); 
           else {
	     if(newsymp(lex[k], 1)) {
	       symbol s;
	       s.kind = 2;
	       s.name = lex[k];
	       s.level = l;
	       s.addr = (vnn++) + 4;
	       s.mark = 0;
	       symtab[si++] = s;
	     }
	     else
	       return printerr(38, "vardeclp");
	   }

           cvpn[cvpni]++;
	   vn[vni]++;
           break;
    case 1: if(strcmp(lex[k], COMMASYM)) return printerr(5, "vardeclp"); break;
    }

    k++; j++;
  }

  if(strcmp(lex[k], SEMICOLSYM))
    return printerr(17, "vardeclp");

  return pr("vardeclp", v && k == end-1);
}

int procdeclp(char ** lex, int start, int end) {
  // printf("procdeclp: "); printlex(lex, start, end);

  l++;

  int pci;

  if(start >= end)
    return 1;

  if((end - start) < 5)
    return 0;

  int proc = !strcmp(lex[start], PROCSYM);
  int iden = !strcmp(lex[start+1], IDENTSYM);
  int semi = !strcmp(lex[start+3], SEMICOLSYM);
  int semie = !strcmp(lex[end-1], SEMICOLSYM);

  if(!proc)
    return printerr(44, "procdeclp");

  if(!iden)
    return printerr(4, "procdeclp");

  if(newsymp(lex[start+2], 1)) {
    pci = ci;
    emit(7, 0, 0);
    symbol s;
    s.kind = 3;
    s.name = lex[start+2];
    s.level = l-1;
    s.addr = ci;
    s.mark = 0;
    psi[psii++] = si;
    symtab[si++] = s;
  }
  else
    return printerr(38, "procdeclp");

  if(!semi)
    return printerr(5, "procdeclp");

  if(!semie)
    return printerr(5, "procdeclp");

  cvpn[cvpni]++;
  block = 0;
  
  if(!blockp(lex, start+4, end-1, 0))
    return printerr(45, "procdeclp");

  l--;
  emit(2, 0, 0);
  code[pci].m = ci;
  
  return pr("procdeclp", 1);
}

int statementp(char ** lex, int start, int end) {
  // printf("statementp: "); printlex(lex, start, end);
  if(start >= end)
    return pr("statementp", 1);

  int ident = !strcmp(lex[start], IDENTSYM);
  int call = !strcmp(lex[start], CALLSYM);
  int begin = !strcmp(lex[start], BEGINSYM);
  int iff = !strcmp(lex[start], IFSYM);
  int whilee = !strcmp(lex[start], WHILESYM);
  int read = !strcmp(lex[start], READSYM);
  int write = !strcmp(lex[start], WRITESYM);

  if(ident) {
    if((end - start) < 3)
      return printerr(33, "statementp ident");

    if(newsymp(lex[start+1], 0))
      return printerr(11, "statementp");

    if(getsym(lex[start+1]).kind == 1)
      return printerr(12, "statementp");
    
    int bec = !strcmp(lex[start+2], BECOMESYM);
    if(!bec)
      return printerr(13, "statementp ident");

    symbol s = getsym(lex[start+1]);
    if(!expressionp(lex, start+3, end))
      return 0;

    return emit(4, l - s.level, s.addr);
  }

  if(call) {
    if(start == end-1)
      return printerr(4, "statementp call");
    else if((end - start) != 3)
      return printerr(35, "statementp call");

    int iden = !strcmp(lex[start+1], IDENTSYM);
    if(!iden)
      return printerr(35, "statementp call");

    if(newsymp(lex[start+2], 0))
      return printerr(11, "statementp call");

    symbol s = getsym(lex[start+2]);

    if(s.mark)
      return 0;
    
    if(s.kind != 3)
      return printerr(42, "statementp call");

    emit(5, l - s.level, s.addr);

    return pr("statementp call", 1);
  }

  if(begin) {
    int en = !strcmp(lex[end-1], ENDSYM);
    if(!en) return printerr(17, "statementp begin");

    int s = start + 1;
    int e = end-1;
    int i;

    while((i = topMatch(BEGINSYM, ENDSYM, SEMICOLSYM, lex, s, e)) >= 0) {
      if (!statementp(lex, s, i))
	return printerr(7, "statementp begin");

      s = i+1;
    }

    return pr("statementp begin", statementp(lex, s, e) ? 1 : printerr(7, "statementp begin"));
  }

  if(iff) {
    int then = findIndex(THENSYM, lex, start, end);
    if(then < 0)
      return printerr(16, "statementp if");

    if (!conditionp(lex, start+1, then))
      return printerr(41, "statementp if");

    int oci = ci;
    emit(8, 0, 0);

    int elsee = topMatch(IFSYM, ELSESYM, ELSESYM, lex, then+1, end);

    if(elsee >= 0) {
      if(!statementp(lex, then+1, elsee))
	return printerr(7, "statementp if");

      code[oci].m = ci+1;

      oci = ci;
      emit(7, 0, 0);

      if(!statementp(lex, elsee+1, end))
	return printerr(7, "statementp if else");

      code[oci].m = ci;
    }
    else {
      if (!statementp(lex, then+1, end))
	return printerr(7, "statementp if");

      code[oci].m = ci;
    }

    return pr("statementp if", 1);
  }

  if(whilee) {
    int doo = findIndex(DOSYM, lex, start, end);
    if(doo < 0)
      return printerr(18, "statementp while");

    int ci1 = ci;
    if(!conditionp(lex, start+1, doo))
      return printerr(41, "statementp while");

    int ci2 = ci;
    emit(8, 0, 0);

    if(!statementp(lex, doo+1, end))
      return printerr(7, "statementp while");

    emit(7, 0, ci1);

    code[ci2].m = ci;

    return pr("statementp while", 1);
  }

  if(read) {
    if(start == end-1)
      return printerr(4, "statementp read");
    else if ((end - start) != 3)
      return printerr(35, "statementp read");

    int iden = !strcmp(lex[start+1], IDENTSYM);
    if(!iden)
      return printerr(35, "statementp read");

    if(newsymp(lex[start+2], 0))
      return printerr(11, "statementp read");

    symbol s = getsym(lex[start+2]);
    if(s.kind == 1)
      return printerr(12, "statementp read/write");

    if(s.kind == 3)
      return 0;

    emit(10, 0, 2);
    emit(4, l - s.level, s.addr);
    return pr("statementp read", 1);
  }

  if(write) {
    if(!expressionp(lex, start+1, end))
      return 0;
    
    return pr("statementp write", emit(9, 0, 1));
  }

  return pr("statementp", 0);
}

int conditionp(char ** lex, int start, int end) {
  //printf("conditionp: start = %d, end = %d\n", start, end);  printlex(lex, start, end);

  if (start >= end)
    return pr("conditionp", 0);

  int odd = !strcmp(lex[start], ODDSYM);
  if (odd)
    return pr("conditionp", expressionp(lex, start+1, end) && emit(2, 0, 6));

  int ind[6];
  ind[0] = findIndex(EQSYM, lex, start, end);
  ind[1] = findIndex(NEQSYM, lex, start, end);
  ind[2] = findIndex(LESYM, lex, start, end);
  ind[3] = findIndex(LEQSYM, lex, start, end);
  ind[4] = findIndex(GESYM, lex, start, end);
  ind[5] = findIndex(GEQSYM, lex, start, end);

  for(int i = 0; i < 6; i++) {
    if(ind[i] >= 0) {
      return pr("conditionp", expressionp(lex, start, ind[i]) &&
		expressionp(lex, ind[i] + 1, end) && emit(2, 0, i + 8));
    }
  }

  return pr("conditionp", printerr(20, "conditionp"));
}


int bpp(char ** lex, int start, int end) {
  int bal = 0;
  for(int i = start; i < end; i++) {
    if(!strcmp(lex[i], NUMBERSYM)) {
      i++; continue;
    }
    if(!strcmp(lex[i], LPARSYM))
      bal++;
    if(!strcmp(lex[i], RPARSYM))
      if(!bal)
	return 0;
      else bal--;
  }

  return !bal;
}

int nextopOP(int plmn, char ** lex, int start, int end) {
  int f = topMatch(LPARSYM, RPARSYM, plmn ? PLUSSYM : STARSYM, lex, start, end);
  if(f < 0)
    f = topMatch(LPARSYM, RPARSYM, plmn ? MINUSSYM : SLASHSYM, lex, start, end);

  return f < 0 ? -1 : f;
}

int expressionp(char ** lex, int start, int end) {
  //printf("expressionp: start = %d, end = %d\n", start, end);  printlex(lex, start, end);
  if(start >= end)
    return pr("expressionp", 0);

  if(!bpp(lex, start, end))
    return printerr(22, "expressionp");

  int plus = !strcmp(lex[start], PLUSSYM);
  int minus = !strcmp(lex[start], MINUSSYM);

  int i, e = 0, s = start;

  if(plus || minus) s++;

  if(s == end)
    return pr("expressionp", 0);

  do {
    i = nextopOP(1, lex, s, end);
    
    if(i < 0) {
      if(termp(lex, s, end)) {
	if(e) emit(2,0,e);
	if(minus) emit(2,0,1);
	return pr("expressionp", 1);
      }
      else
	return pr("expressionp", 0);
    }

    if(!termp(lex, s, i))
      return printerr(36, "expressionp");

    if(e) emit(2, 0, e);
    e = !strcmp(lex[i], PLUSSYM) ? 2 : 3;

    s = i + 1;
  } while(i < end);

  if(e) emit(2,0,e);
  
  if(minus)
    emit(2, 0, 1);

  return pr("exprpr", 1);
}

int termp(char ** lex, int start, int end) {
  //   printf("termp: start = %d, end = %d\n", start, end);  printlex(lex, start, end);

  if (start >= end)
    return pr("termp", 0);

  int i, e = 0, s = start;

  do {
    i = nextopOP(0, lex, s, end);

    if(i < 0) {
      if(factorp(lex, s, end))
	return pr("termpp", e ? emit(2,0,e) : 1);
      else
	return pr("termppp", 0);
    }

    if(!factorp(lex, s, i))
      return printerr(37, "termp");

    if(e) emit(2,0,e);
    e = !strcmp(lex[i], STARSYM) ? 4 : 5;
    
    s = i + 1;
  } while(i < end);

  if(e) emit(2,0,e);
  
  return pr("termpppp", 1);
}

int factorp(char ** lex, int start, int end) {
  //  printf("factorp: start = %d, end = %d\n", start, end);  printlex(lex, start, end);
  if(start >= end)
    return 0;
  
  int iden = !strcmp(lex[start], IDENTSYM);
  int numb = !strcmp(lex[start], NUMBERSYM);
  int lpar = !strcmp(lex[start], LPARSYM);

  if(iden || numb) {
    if(iden)
      if(newsymp(lex[start+1], 0))
	return printerr(11, "factorp");

    if((end - start) != 2) {
      return pr("factorp", 0);
    }

    if(iden) {
      symbol s = getsym(lex[start+1]);
      if(s.kind == 1)
	emit(1, 0, s.val);

      if(s.kind == 2)
	emit(3, l - s.level, s.addr);

      if(s.kind == 3)
	return 0;
    }
    else
      emit(1, 0, atoi(lex[start+1]));
  }
  else {
    if(!lpar)
      return pr("factorp", 0);

    int rpar = !strcmp(RPARSYM, lex[end-1]);
    if (!rpar)
      return printerr(22, "factorp");

    return pr("factorp", expressionp(lex, start+1, end-1));
  }
}
