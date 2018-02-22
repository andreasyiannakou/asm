// Name: Andreas Yiannakou, Student Number: 1347357  Username: ay13695
// Declaration of authorship: I confirm that I am the sole author of the code in this file.
// The subfunctions beginning with Allocate are variations of AllocateNode found in the C notes.
// The instruction set struct table is a variation of the struct table used in the sequential search section of the C notes.

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

// Sets the maximum line length.
#define MAX   250
// Number of instructions in the instruction table.
#define NUM_INST 21
// Number of variables in the error array.
#define ERRORARRAY 2
// Finds the 2's complement of the numbers
#define CONVERTNEGATIVE 16777216

// Instruction set structure. 
struct instset {
  char *mne;
  int expop;
  int instnum;
  int value;
};
typedef struct instset INST;

// Structure for reading in the file.
struct oneread {
  char *line;
  int len, label, comment, instruction, operand;
  struct oneread *next;
};
typedef struct oneread READ;

// Structure containing all the object code, addresses, labels, instructions, operands and comments.
struct asm2obj {
  // First process.
  char *label, *mne, *operand, *comment;
  int address, print, instcode, expop;
  struct asm2obj *next;
  // Second process.
  int objcode;
};
typedef struct asm2obj CODE;

// Structure containing all the labels and their addresses, as well as whether they're used.
struct labels {
  char *label;
  int address, used;
  struct labels *next;
};
typedef struct labels LABELS;

int argchecker(int a);

int filechecker(FILE *file, char *argv[]);

READ *read2struct(FILE *file, int error[ERRORARRAY], FILE *logfile);

READ *AllocateRead(char *str);

CODE *AllocateCode();

LABELS *AllocateLabels(char *str);

int DupLabels(LABELS *n, char *s);

int OperandLabel(LABELS *n, char *s);

int LabelAddress(LABELS *n, char *s);

int LabelUsed(LABELS *n, int error[ERRORARRAY], FILE *logfile);

CODE *FirstProcess(READ *currentline, INST *table, int error[ERRORARRAY], FILE *logfile);

LABELS *LabelProcessing(CODE *current, int labels, LABELS *currentlabel, LABELS *firstlabel, int error[ERRORARRAY], FILE *logfile);

int MneChecks(int address, CODE *current, INST *table, char *str, int b, LABELS *firstlabel, int error[ERRORARRAY], FILE *logfile);

int FindMne(CODE *p, INST *l);

void PrintOutputFiles(CODE *l, char *argv[]);

void SecondProcess(CODE *current, LABELS *first, INST *table, int error[ERRORARRAY], FILE *logfile);

int CheckIfValue(CODE *current, INST *table);

FILE *CreateLogFile(char *argv[]);

void OperandIsLabel(CODE *current, INST *table, int error[ERRORARRAY], FILE *logfile);

int main(int argc, char *argv[])
{
  FILE *file, *logfile;
  READ *firstline;
  CODE *start;
  int error[ERRORARRAY] = {0, 0};
  INST table[NUM_INST] = {
    {"data", 1, 0, 1},
    {"ldc", 1, 0, 1},
    {"adc", 1, 1, 1},
    {"ldl", 1, 2, 0},
    {"stl", 1, 3, 0},
    {"ldnl", 1, 4, 0},
    {"stnl", 1, 5, 0},
    {"add", 0, 6, 0},
    {"sub", 0, 7, 0},
    {"shl", 0, 8, 0},
    {"shr", 0, 9, 0},
    {"adj", 1, 10, 1},
    {"a2sp", 0, 11, 0},
    {"sp2a", 0, 12, 0},
    {"call", 1, 13, 0},
    {"return", 0, 14, 0},
    {"brz", 1, 15, 0},
    {"brlz", 1, 16, 0},
    {"br", 1, 17, 0},
    {"HALT", 0, 18, 0}, 
    {"SET", 1, 0, 1} };
  argchecker(argc);
  file = fopen(argv[1], "r");
  filechecker(file, argv);
  logfile = CreateLogFile(argv);
  firstline = read2struct(file, error, logfile);
  fclose(file);
  start = FirstProcess(firstline, table, error, logfile);
  fprintf(logfile, "\nErrors: %d.   Warnings: %d. \n", error[0], error[1]);
  if(error[0] == 0) {
    PrintOutputFiles(start, argv);
    fprintf(logfile, "\nCode assembled successfully.\n");
  }
  else {
    fprintf(logfile, "\nSince the .asm file is not error free, the object code could not be properly converted, please rectify the errors stated above in this logfile and run the program again.\n");
  }
  fclose(logfile);
  return 0;
}

// Checks the correct number of arguments have been entered.
int argchecker(int a)
{
  if (a < 2) {
    fprintf(stderr, "Please enter a file name!\n");
    exit(1);
  }
  if (a > 2) {
    fprintf(stderr, "Too many arguments entered!\n");
    exit(1);
  }
  return 0;
}

// Checks the file exists.
int filechecker(FILE *file, char *argv[])
{
  int n = strlen(argv[1]);
  char *str = malloc(n*sizeof(char));
  strcpy(str, argv[1]);
  if(str[n-4] == '.' && str[n-3] == 'a' && str[n-2] == 's' && str[n-1] == 'm') {
  }
  else {
    fprintf(stderr, "File has wrong extension it should be .asm, rather than .%c%c%c.\n", str[n-3], str[n-2], str[n-1]);
    exit(1);
  }
  if (!file) {
    fprintf(stderr, "Can't read the file: %s\n", argv[1]);
    exit(1);
  }
  return 0;
}

// Reads the .asm file into a structure allowing it to be processed without needing a second pass.
READ *read2struct(FILE * file, int error[ERRORARRAY], FILE *logfile)
{
  int i, m = 0, n, count = 1;
  int comn, comc, labeln, labelc, insn, insc, opn, opc;
  char c[MAX], *str;
  READ *current, *start;
  while((fgets(c, MAX, file)) != NULL) {
    i = labelc = comc = insc = opc = labeln = opn = 0;
    if(c[i] != '\n') { 
      while(1) {
	if(c[i] == ' ' || c[i] == '\t') {
	  i++;
	}
	else {break;}
      }
      m = strlen(c);
      str =  malloc(m*sizeof(char));
      for(n = 0; n+i+1<m; n++) {
	str[n] = c[i+n];
      }
      if(count == 1) {
	start = current = AllocateRead(str);
      }
      else {
	current->next = AllocateRead(str);
	current = current->next;
      }
      comn = strlen(str);
      for(n = 0; n<=strlen(str); n++) {
	if(str[n] == ';') {
	  comn = n;
	  comc = 1;
	  break;
	}
      }
      for(n = 0; n<comn; n++) {
	if(labelc == 0) {
	  if(str[n] == ':') {
	    labeln = n+1;
	    labelc++;
	  }
	}
	else {
	  if(str[n] == ':') {
	    labelc++;
	  }
	}
      }
      insn = labeln;
      for(n = labeln; n<comn; n++) {
	if((str[n] >= 'a' && str[n] <= 'z') || (str[n] >= 'A' && str[n] <= 'Z') || str[n] == '2') {
	  if(str[n+1] == ' ' || str[n+1] == '\t' || str[n+1] == '\0' || str[n+1] == ';') {
	    insn = n+1;
	    insc = 1;
	    break;
	  }
	}
      }
      for(n = insn; n<comn; n++) {
	if((str[n] >= 'a' && str[n] <= 'z') || (str[n] >= 'A' && str[n] <= 'Z')
	   || (str[n] >= '0' && str[n] <= '9')) {
	  if(n+1 == comn) {
	    opn = n+1;
	    opc++;
	    break;
	  }  
	  if(!((str[n+1] >= 'a' && str[n+1] <= 'z') || (str[n+1] >= 'A' && str[n+1] <= 'Z') 
	       || (str[n+1] >= '0' && str[n+1] <= '9'))) {
	    opn = n+1;
	    opc++;
	  }
	}
      }
      if((i > 0) && (labelc >= 1)) {
	fprintf(logfile, "Line: %d  Warning: Space at the beginning of a line containing a label.\n", count);
	error[1]++;
      }
      if(comc == 1) {
	current->comment = comn;
      }
      if(labelc >= 1) {
	current->label = labeln;
	if(labelc > 1) {
	  fprintf(logfile, "Line: %d  Error: %d labels detected on the same line.\n", count, labelc);
	  error[0]++;
	}
      }
      if(insc == 1) {
	current->instruction = insn;
      }
      if(opc >= 1) {
	current->operand = opn;
	if(opc > 1) {
	  fprintf(logfile, "Line: %d  Error: %d operands detected.\n", count, opc);
	  error[0]++;
	}      
      }
      n = strlen(str);
    }
    count++;
  }
  return start;
}


// Allocates a new node of the Read structure type defined at the beginning.
READ *AllocateRead(char *str)
{
  READ *p;
  int n = 0;
  p = (READ *)malloc(sizeof(READ));
  if(p==NULL){
    fprintf(stderr, "Memory Allocation error.\n");
    exit(1);
  }
  p->line  = malloc(strlen(str)*sizeof(char));
  strcpy(p->line, str);
  n = strlen(p->line);
  p->len = n;
  p->label = 0; 
  p->comment = n; 
  p->instruction = 0; 
  p->operand = 0;
  p->next = NULL;
  return p;
}

// Allocates a new node of the Code structure type defined at the beginning.
CODE *AllocateCode()
{
  CODE *p;
  p = (CODE *)malloc(sizeof(CODE));
  if(p==NULL){
    fprintf(stderr, "Memory Allocation error.\n");
    exit(1);
  }
  p->address = 0;
  p->print = 1;
  p->label = malloc(MAX*sizeof(char));
  p->mne = malloc(MAX*sizeof(char));
  p->operand = malloc(MAX*sizeof(char));
  p->comment = malloc(MAX*sizeof(char));
  p->next = NULL;
  p->objcode = 0;
  p->instcode = 0;
  p->expop = 0;
  return p;
}

// Allocates a new node of the Labels structure type defined at the beginning.
LABELS *AllocateLabels(char *str)
{
  LABELS *p;
  int a = strlen(str)-1;
  p = (LABELS *)malloc(sizeof(LABELS));
  if(p==NULL){
    fprintf(stderr, "Memory Allocation error.\n");
    exit(1);
  }
  p->label  = malloc(a*sizeof(char));
  strncpy(p->label, str, a);
  p->address = 0;
  p->next = NULL;
  return p;
}

// Checks for duplicates labels.
int DupLabels(LABELS *n, char *s)
{
  int a = strlen(s)-1; 
  char *str  = malloc(a*sizeof(char));
  strncpy(str, s, a);
  do{
    if(strcmp(n->label, str) == 0){   
      return 1;
    }
    n = n->next;
  }while(n != NULL);
  return 0;
}

// Checks whether the operand is a label.
int OperandLabel(LABELS *n, char *s)
{
  do{
    if(strcmp(n->label, s) == 0){  
      n->used = 1;
      return 1;
    }
    n = n->next;
  }while(n != NULL);
  return 0;
}

// Finds and returns the address of the label.
int LabelAddress(LABELS *n, char *s)
{
  do{
    if(strcmp(n->label, s) == 0){   
      return n->address;
    }
    n = n->next;
  }while(n != NULL);
  return -1;
}

// Checks whether the label has been used, if it hasn't it prints a warning to the log file.
int LabelUsed(LABELS *n, int error[ERRORARRAY], FILE *logfile)
{
  do{
    if(n->used == 0){ 
      fprintf(logfile, "Address: %04X Warning: Unused label '%s'.\n", n->address, n->label);
      error[1]++;
    }
    n = n->next;
  }while(n != NULL);
  return 0;
}

// The first process through the assembly code, to find the label, instruction and comments. 
CODE *FirstProcess(READ *currentline, INST *table, int error[ERRORARRAY], FILE *logfile)
{  
  CODE *start, *current;
  LABELS *firstlabel, *currentlabel;
  int a, b, c, d, e, i, labels = 0, n = 0, count = 0, address = 0;
  char *ins, *ope, *com, *str;
  firstlabel = AllocateLabels(" ");
  while(currentline != NULL) {
    a = currentline->len;
    b = currentline->label;
    c = currentline->instruction;
    d = currentline->operand;
    e = currentline->comment;
    ins = malloc(a*sizeof(char));
    ope = malloc(a*sizeof(char));
    com = malloc(a*sizeof(char));
    str = malloc(a*sizeof(char));
    char *pEnd = malloc(sizeof(char));
    strcpy(str, currentline->line);
    if(b + c > 0) {
      if(count == 0) {
	start = current = AllocateCode();
	if(b>0 && c == 0) {
	  address--;
	}
	count = 1;
      }
      else {
	current->next = AllocateCode();
	current = current->next;
	if(b>0 && c == 0) {
	  address++;
	  current->address = address;
	  address--;
	}
	else {
	  address++;
	  current->address = address;
	}
      }
    
    if(b > 0) {
      n = 0;
      current->print *= 2;
      strncpy(current->label, str, b);
      for(i = 1; i < b; i++) {
	if(!((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z') || str[i] == ':')) {
	  fprintf(logfile, "Address: %04X Error: The label %s is not alphanumeric.\n", current->address, current->label);
	  error[0]++;
	  break;
	}
      }
      if(str[0] >= '0' && str[0] <= '9') {
	fprintf(logfile, "Address: %04X Error: The label %s starts with a number not a letter.\n", current->address, current->label);
	error[0]++;
      }
      labels++;
      if(labels == 1) {
	firstlabel = currentlabel = AllocateLabels(current->label);
      }
      else {
	currentlabel = LabelProcessing(current, labels, currentlabel, firstlabel, error, logfile);
      }
      currentlabel->address = current->address;
    }
	
    if(c > 0) {
      current->print *= 3;
      n = 0;   
      for(i = b; i < c; i++) {
	if(str[i] != ' ' && str[i] != '\t') {
	  ins[n] = str[i];
	  n++;
	}
      }
      ins[n] = '\0';
      strcpy(current->mne, ins);
      address = MneChecks(address, current, table, str, b, firstlabel, error, logfile);
    }

    if(d > 0) {
      if(current->expop != 1) {
	fprintf(logfile, "Address: %04X Error: The instruction '%s' does not expect an operand.\n", current->address, current->mne);
	error[0]++;
      }
      current->print *= 5;
      n = 0;
      for(i = c; i < d; i++) {
	if(str[i] != ' ' && str[i] != '\t') {
	    ope[n] = str[i];
	    n++;
	  }
	}
	ope[n] = '\0';
	strcpy(current->operand, ope);
	if(strcmp(current->mne, "SET") == 0) {
	  currentlabel->address = strtol(current->operand, &pEnd, 0); 
	}
      }
      else {
	current->operand = "";
	if(current->expop != 0) {
	  fprintf(logfile, "Address: %04X Error: The instruction '%s' expects an operand.\n", current->address, current->mne);
	  error[0]++;
	}
      }				 	
      if(e != a) {
	current->print *= 7;
	n = 0;
	for(i = e; i < a; i++) {
	  com[n] = str[i];
	  n++;
	}
      }
      com[n] = '\0';
      strcpy(current->comment, com);
    }

    if((str[c] == ';') && c>0) {
      fprintf(logfile, "Address: %04X Warning: '%s%s' No gap between the instruction and comment.\n", current->address, current->mne, current->comment);
      error[1]++;
    }
    if((str[d] == ';') && d>0) {
      fprintf(logfile, "Address: %04X Warning: '%s%s' No gap between the operand and comment.\n", current->address, current->operand, current->comment);
      error[1]++;
    }
    currentline = currentline->next;
  }
  SecondProcess(start, firstlabel, table, error, logfile);
  return start;
}

// Processes the labels after an initial label has been created.
LABELS *LabelProcessing(CODE *current, int labels, LABELS *currentlabel, LABELS *firstlabel, int error[ERRORARRAY], FILE *logfile)
{ 
  if(DupLabels(firstlabel, current->label) == 1) {
    fprintf(logfile, "Address: %04X Error: %s is a duplicate label.\n", current->address, current->label);
    error[0]++;
  }
  else{
    currentlabel->next = AllocateLabels(current->label);
    currentlabel = currentlabel->next;
  }
  return currentlabel;
}

// Checks the validity of the mnemonic  in the assembly code and issues warnings and error accordingly.
int MneChecks(int address, CODE *current, INST *table, char *str, int b, LABELS *firstlabel, int error[ERRORARRAY], FILE *logfile)
{
  int mnecheck = 0;
  if((mnecheck = FindMne(current, table)) == 0) {
    fprintf(logfile, "Address: %04X Error: Instruction '%s' not valid\n", current->address, current->mne);
    error[0]++;
  }
  if(mnecheck == 2) {
    fprintf(logfile, "Address: %04X Warning: Instruction '%s' isn't in all small caps in asm file.\n", current->address, current->mne);
    error[1]++;
  }
  if(mnecheck == 3) {
    fprintf(logfile, "Address: %04X Warning: Instruction '%s' isn't in all capitals in asm file.\n", current->address, current->mne);
    error[1]++;
  }
  if((str[b] != ' ' && str[b] != '\t') && b>0) {
    fprintf(logfile, "Address: %04X Warning: '%s%s' No gap between the label and instruction.\n", current->address, current->label, current->mne);
    error[1]++;
  }
  if(strcmp(current->mne, "SET") == 0) {
    address--;
  }
  if(strcmp(current->mne, "SET") == 0 && b == 0) {
    fprintf(logfile, "Address: %04X Error: SET instruction '%s' must be on the same line as a label\n", current->address, current->mne);
    error[0]++;
  }
  return address;
}

// Finds the mnemonic instruction by searching through the instruction table defined in main.
int FindMne(CODE *p, INST *l)
{
  int i, j, n = strlen(p->mne);
  char *lowermne = malloc(n*sizeof(char));
  char *uppermne = malloc(n*sizeof(char));
  strcpy(lowermne, p->mne);
  for(i = 0; i <n; i++) {
    lowermne[i] = tolower(lowermne[i]);
  }
  for(i = 0; i <n; i++) {
    uppermne[i] = toupper(lowermne[i]);
  }
  for(j=0; j<NUM_INST; j++) {
   if(strcmp(p->mne, l[j].mne) == 0){
     p->instcode = l[j].instnum;
     p->expop = l[j].expop;
     return 1;
   }
   if(strcmp(lowermne, l[j].mne) == 0){
     p->instcode = l[j].instnum;
     p->expop = l[j].expop;
     strcpy(p->mne, lowermne);
     return 2;
   }
   if(strcmp(uppermne, l[j].mne) == 0){
     p->instcode = l[j].instnum;
     p->expop = l[j].expop;
     strcpy(p->mne, uppermne);
     return 3;
   }
 }
 return 0;
}

// Prints the Listing and Object code files.
void PrintOutputFiles(CODE *l, char *argv[])
{
  FILE *lfp, *ofp;
  int a, n = strlen(argv[1]);
  char *filename = malloc(n*sizeof(char));
  strncpy(filename, argv[1], n-3);
  filename[n-3] = 'o';
  ofp = fopen(filename, "w");
  if (ofp == NULL) {
    fprintf(stderr, "Can't open output file %s!\n","filename");
    exit(1);
  }
  filename[n-3] = 'l';
  filename[n-2] = 's';
  filename[n-1] = 't';
  lfp = fopen(filename, "w");
  if (lfp == NULL) {
    fprintf(stderr, "Can't open output file %s!\n","filename");
    exit(1);
  }
  fprintf(lfp, "LISTING FILE\n");
  do{
    fprintf(lfp, "%04X ", l->address);
      if(l->print % 3 == 0) {
	if((a = strcmp(l->mne, "SET")) == 0) {
	  fprintf(lfp, "         ");
	}
	else {
	  if((a = strcmp(l->mne, "data")) != 0) {
	    fprintf(lfp, "%06X", l->objcode);
	    fprintf(ofp, "%06X", l->objcode);
	  }
	  else {
	    fprintf(lfp, "%08X ", l->objcode);
	    fprintf(ofp, "%08X\n", l->objcode);
	  }
	  if((a = strcmp(l->mne, "data")) != 0) {
	    fprintf(lfp, "%02X ", l->instcode);
	    fprintf(ofp, "%02X\n", l->instcode);
	  }
	}
      }
      else {
	fprintf(lfp, "         ");
      }
      if(l->print % 2 == 0) {
	fprintf(lfp, "%s ", l->label); 
      }
      if(l->print % 3 == 0) {
	if((a = strcmp(l->mne, "data")) == 0) {
	  fprintf(lfp, "%s ", l->mne);
	}
	else {
	  fprintf(lfp, "%s ", l->mne);
	}
      }
      if(l->print % 5 == 0) {
	fprintf(lfp, "%s ", l->operand);
      }
      if(l->print % 7 == 0) {
	fprintf(lfp, "%s ", l->comment);
      }
      fprintf(lfp, "\n");
    l = l->next;
  }while(l != NULL);
  fprintf(lfp, "LISTING FILE END\n");
  fclose(lfp);
  fclose(ofp);
}

// This processes the operand (not possible before due to the possibility of forward referencing labels).
void SecondProcess(CODE *current, LABELS *firstlabel, INST *table, int error[ERRORARRAY], FILE *logfile)
{ 
  int i, n;
  char *str = malloc(sizeof(char));
  char *pEnd = malloc(sizeof(char));
  while(current != NULL) {
    if(strcmp(current->operand, "") != 0) {
      strcpy(str, current->operand);
      n = 0;
      for(i=0; i<strlen(str); i++) {
	if((str[i] >= '0' && str[i] <= '9') || str[i] == '+' || str[i] == '-' || str[i] == 'x' || str[i] == 'X' || (str[i] >= 'a' && str[i] <= 'f') || (str[i] >= 'A' && str[i] <= 'F')) {
	  n++;
	}
      }
      if(((str[0] >= '0' && str[0] <= '9') || str[0] == '+' || str[0] == '-') && n == strlen(str)) {
	current->objcode = strtol(current->operand, &pEnd, 0);  
	if((current->objcode < 0) &&(strcmp(current->mne, "data") != 0)) {
	  current->objcode += CONVERTNEGATIVE;
	}
      }
      else {
	if(OperandLabel(firstlabel, str) == 1) {
	  current->objcode = LabelAddress(firstlabel, str);
	  OperandIsLabel(current, table, error, logfile);
	}
	else {
	  fprintf(logfile, "Address: %04X Error: Operand '%s' neither a number nor a label.\n", current->address, current->operand);	  
	  error[0]++;
	}
      }
    }
    current = current->next;
  }
  if(strcmp(firstlabel->label, " ") != 0) {
    LabelUsed(firstlabel, error, logfile);
  }
}

// Checks whether the instruction requires a value or an offset.
int CheckIfValue(CODE *current, INST *table) 
{
  int i;
  for(i = 0; i< NUM_INST; i++) {
    if(strcmp(current->mne, table[i].mne) == 0) {
      return table[i].value;
    }
  }
  return 0;
}

// Initalises the log file.
FILE *CreateLogFile(char *argv[])
{
  FILE *logfile;
  int n = strlen(argv[1]);
  char *filename = malloc(n*sizeof(char)); 
  strncpy(filename, argv[1], n-3);
  filename[n-3] = 'l';
  filename[n-2] = 'o';
  filename[n-1] = 'g';
  logfile = fopen(filename, "w");
  if (logfile == NULL) {
    fprintf(stderr, "Can't open output file %s!\n","filename");
    exit(1);
  }
  return logfile;
}

// Checks whether the operand is a label, and deals with the object code accordingly.
void OperandIsLabel(CODE *current, INST *table, int error[ERRORARRAY], FILE *logfile)
{
  if(current->objcode == current->address) {
    fprintf(logfile, "Address: %04X Warning: Label '%s' called on same line as label defined.\n", current->address, current->operand);	  
    error[1]++;
    if(current->instcode == 13 || current->instcode == 17) {
      fprintf(logfile, "Address: %04X Warning: Infinite loop created:  %s %s %s.\n", current->address, current->label, current->mne, current->operand);	  
      error[1]++;
    }
    if(current->instcode == 15 || current->instcode == 16) {
      fprintf(logfile, "Address: %04X Warning: Possibility of an infinite loop created:  %s %s %s.\n", current->address, current->label, current->mne, current->operand);	  
      error[1]++;
    }
  }
  if(current->objcode < 0 && strcmp(current->mne, "data") != 0) {
    current->objcode += CONVERTNEGATIVE;
  }
  if(CheckIfValue(current, table) == 0) {
    current->objcode -= (current->address + 1);
    if(current->objcode < 0) {
      current->objcode += CONVERTNEGATIVE;
    }
  }
}
