// Name: Andreas Yiannakou, Student Number: 1347357  Username: ay13695
// Declaration of authorship: I confirm that I am the sole author of the code in this file. 

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// Used to read in the .o file.
#define CODELENGTH 9
// Memory limit to 0xFFF.
#define MEMORYLIMIT 4095
// Values for the log files.
#define INITIAL 1
#define FINAL   2
// Values for the registers array.
#define A   1
#define B   2
#define PC  3
#define SP  4
#define MAX 5
// Finds the 2's complement of the numbers
#define CONVERTNEGATIVE 16777216
// Checks whether the number is negative
#define ISNEGATIVE 8388608

// Stores the data from the .o file.
struct objectcode {
  char *line;
  int opcode, value, offset, objcode, address;
  struct objectcode *next;
};
typedef struct objectcode CODE;

int argchecker(int a);

int filechecker(FILE *file, char *argv[]);

CODE *AllocateCode(char *str);

CODE *Read2Memory(FILE *file, int memory[MEMORYLIMIT]);

void FindOpcode(CODE *current, char objcode[CODELENGTH]);

void FindValueOffset(CODE *current, char objcode[CODELENGTH]);

void ApplyInstruction(CODE *current, int memory[MEMORYLIMIT], int registers[MAX]);

int ProgramCounterCheck(int memory[MEMORYLIMIT], int registers[MAX]);

int MemoryAccessCheck(CODE *current, int memory[MEMORYLIMIT], int a);

int BranchChecks(CODE *current, int memory[MEMORYLIMIT], int registers[MAX]);

CODE *FindNextInstruction(int a, CODE *current);

int CreateMemoryLog(int a, char *argv[], int memory[MEMORYLIMIT], int registers[MAX]);

void PrintMemoryLog(FILE *file, int memory[MEMORYLIMIT], int registers[MAX]);

void RunProgram(CODE *current, int memory[MEMORYLIMIT], int registers[MAX], CODE *start);

int main(int argc, char *argv[])
{
  FILE *file;
  int memory[MEMORYLIMIT] = {0};
  int registers[MAX] = {0, 0, 0, 0, 4095};
  CODE *start, *current;
  argchecker(argc);
  file = fopen(argv[1], "r");
  filechecker(file, argv);
  start = current = Read2Memory(file, memory);
  fclose(file);
  CreateMemoryLog(INITIAL, argv, memory, registers);
  printf("A = %08X, B = %08X, PC = %03X, SP = %03X.\n", registers[A], registers[B], registers[PC], registers[SP]);
  RunProgram(current, memory, registers, start);
  CreateMemoryLog(FINAL, argv, memory, registers);
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
  if(str[n-1] != 'o' && str[n-2] != '.') {
    fprintf(stderr, "File has wrong extension it should be .o.\n");
    exit(1);
  }
  if (!file) {
    fprintf(stderr, "Can't read the file: %s\n", argv[1]);
    exit(1);
  }
  return 0;
}

// Allocates a new node of the Code structure type defined at the beginning.
CODE *AllocateCode(char *str)
{
  char *ptr = malloc(sizeof(char));
  CODE *p;
  p = (CODE *)malloc(sizeof(CODE));
  if(p==NULL){
    fprintf(stderr, "Memory Allocation error.\n");
    exit(1);
  }
  p->line  = malloc(strlen(str)*sizeof(char));
  strcpy(p->line, str);
  p->opcode = p->value = p->offset = p->address = 0; 
  p->objcode = strtol(str, &ptr, 16);
  p->next = NULL;
  return p;
}

// Reads the object code from file into the CODE structure and the memory array.
CODE *Read2Memory(FILE *file, int memory[MEMORYLIMIT])
{
  char obj[CODELENGTH];
  int count = 0;
  CODE *start, *current;
  while((fgets(obj, CODELENGTH, file)) != NULL) {
    if(obj[0] != '\n') {
      if(count == 0) {
	start = current = AllocateCode(obj);
      }
      else {
	current->next = AllocateCode(obj);
	current = current->next;
      }
      current->address = count;
      memory[count] = current->objcode;
      FindOpcode(current, obj);
      FindValueOffset(current, obj);
      count++;
    }
  }
  return start;
}

// Finds the  operation code for the given object code.
void FindOpcode(CODE *current, char objcode[CODELENGTH]) 
{
  char *opcode = malloc(2*sizeof(char));
  char *ptr = malloc(sizeof(char));
  strncpy(opcode, current->line + 6, 2);
  current->opcode = strtol(opcode, &ptr, 16);
}

// Finds the value/offset accompanying the instruction
void FindValueOffset(CODE *current, char objcode[CODELENGTH]) 
{
  char *value = malloc(6*sizeof(char));
  char *ptr = malloc(sizeof(char));
  strncpy(value, current->line, 6);
  current->value = current->offset = strtol(value, &ptr, 16);  
  //8388608 is 2^23 which is equivalent to 800000 in hexidecimal, this finds the 2's complement.
  if(current->value >= ISNEGATIVE) {  
    current->value = current->offset -= CONVERTNEGATIVE;
  }
}

// Applies the instruction the Program Counter is pointing at, by using a switch statement for each different instruction.
void ApplyInstruction(CODE *current, int memory[MEMORYLIMIT], int registers[MAX])
{
  int a = current->opcode;
  registers[PC]++;
  ProgramCounterCheck(memory, registers);
  switch(a) {
  case 0: // Instruction: ldc, Load accumulator with the value specified.
    registers[B] = registers[A];
    registers[A] = current->value;
    return;
  case 1: // Instruction: adc, Add the value specified to the accumulator.
    registers[A] += current->value;
    return;
  case 2: // Instruction: ldl, Load local.
    MemoryAccessCheck(current, memory, registers[SP]);
    registers[B] = registers[A];
    registers[A] = memory[registers[SP] + current->offset];
    return;
  case 3: // Instruction: stl, Store local.
    MemoryAccessCheck(current, memory, registers[SP]);
    memory[registers[SP] + current->offset] = registers[A];
    registers[A] = registers[B];
    return;
  case 4: // Instruction: ldnl, Load non-local.
    MemoryAccessCheck(current, memory, registers[A]);
    registers[A] = memory[registers[A] + current->offset];
    return; 
  case 5: // Instruction: stnl, Store non-local.
    MemoryAccessCheck(current, memory, registers[A]);
    memory[registers[A] + current->offset] = registers[B];
    return;
  case 6: // Instruction: add, Addition.
    registers[A] += registers[B];
    return;
  case 7: // Instruction: sub, Subtraction.
    registers[A] = registers[B] - registers[A];
    return;
  case 8: // Instruction: shl, Shift left.
    registers[A] = registers[B] << registers[A];
    return;
  case 9: // Instruction: shr, Shift right.
    registers[A] = registers[B] >> registers[A];
    return;
  case 10: // Instruction: adj, Adjust SP.
    registers[SP] += current->value;
    return;
  case 11: // Instruction: a2sp, Transfer A to SP.
    registers[SP] = registers[A];
    registers[A] = registers[B];
    return;
  case 12: // Instruction: sp2a, Transfer SP to A.
    registers[B] = registers[A];
    registers[A] = registers[SP];
    return;
  case 13: // Instruction: call, Call procedure.
    registers[B] = registers[A];
    registers[A] = registers[PC];
    BranchChecks(current, memory, registers);
    return;
  case 14: // Instruction: return, Return from procedure.
    if(registers[A] == registers[B] && registers[A] == (registers[PC]-1)) {
      printf("INFINITE LOOP. Infinite loop has been detected in address %04X.\n", registers[A]);
      exit(1);
    }
    registers[PC] = registers[A];
    registers[A] = registers[B];
    ProgramCounterCheck(memory, registers);
    return;
  case 15: // Instruction: brz, If accumulator is zero, branch to specified offset.
    if(registers[A] == 0) {
      BranchChecks(current, memory, registers);
    }
    return;
  case 16: // Instruction: brlz, If accumulator is less than zero, branch to specified offset.
    if(registers[A] < 0) {
      BranchChecks(current, memory, registers);
    }
    return; 
  case 17: // Instruction: br, Branch to specified offset.
    BranchChecks(current, memory, registers);
    return;
  case 18: // Instruction: HALT, Stop the emulator. This is not a `real' instruction, but needed to tell your emulator when to finish.
    return;
  default: // Instruction: data, Reserve a memory location, initialized to the value specified.
    printf("Error: Unrecognised opcode 0x%02X, the emulator is trying to apply data as an instruction.\n", current->opcode);
    exit(1);
    return;
  }
  return;
}

// Check to  see whether the Program Counter is within the valid region of 0 to 0xFFF.
int ProgramCounterCheck(int memory[MEMORYLIMIT], int registers[MAX])
{
  if(registers[PC] > MEMORYLIMIT || registers[PC] < 0) {
    printf("SEGFAULT. Program Counter is at address %03X, outside of valid region.\n", registers[PC]);
    exit(1);
  }
  return 0;
}

// Check to see whether the program is trying to access a valid memory location.
int MemoryAccessCheck(CODE *current, int memory[MEMORYLIMIT], int a)
{
  if((a + current->offset) > MEMORYLIMIT || (a + current->offset) < 0) {
    printf("SEGFAULT. Tried to access memory address %03X.\n", a + current->offset);
    exit(1);
  }
  return 0;
}

// Checks needed for the branch commands brz, brlz and br.
int BranchChecks(CODE *current, int memory[MEMORYLIMIT], int registers[MAX])
{
  if(current->offset == -1) {
    printf("INFINITE LOOP. Infinite loop has been detected in address %04X.\n", registers[PC] + current->offset);
    exit(1);
  }
  registers[PC] += current->offset;
  if(registers[PC] > MEMORYLIMIT || registers[PC] < 0) {
    printf("SEGFAULT. Program Counter is at address %03X, outside of valid region.\n", registers[PC]);
    exit(1);
  }
  return 0;
}

// Finds the next instruction by matching the Program Counter to the instructions address.
CODE *FindNextInstruction(int a, CODE *current)
{
  while(current != NULL) {
    if(a == current->address) {
      return current;
    }
    current = current->next;
  }
  // Breaks the program if the Program Counter is trying to access and apply an instruction from a memory location that does not contain object code from the .o file..
  if(current == NULL) {
    printf("SEGFAULT. Program Counter is at %03X and has exceeded the amount of object code entered.\n", a);
    exit(1);
  }
  return NULL;
}

// Creates the memory log file depeneding on whether it is the intial or final.
int CreateMemoryLog(int a, char *argv[], int memory[MEMORYLIMIT], int registers[MAX])
{
  FILE *file;
  int n = strlen(argv[1]);
  char *filename = calloc(n+20, sizeof(char));
  strncpy(filename, argv[1], n-2);
  if(a == INITIAL) {
    strncpy(filename + n-2, "_emulator_initial.log", 21);
    file = fopen(filename, "w");
    if (file == NULL) {
      fprintf(stderr, "Can't open output file %s!\n","filename");
      exit(1);
    }
    PrintMemoryLog(file,  memory, registers); 
  }
  else {
    strncpy(filename + n-2, "_emulator_final.log  ", 21);
    file = fopen(filename, "w");
    if (file == NULL) {
      fprintf(stderr, "Can't open output file %s!\n","filename");
      exit(1);
    }  
    PrintMemoryLog(file,  memory, registers); 
  }
  return 0;
}

// Prints the memory log output file.
void PrintMemoryLog(FILE *file, int memory[MEMORYLIMIT], int registers[MAX])
{
  int i;
  fprintf(file, "A = %08X, B = %08X, PC = %03X, SP = %03X.\n", registers[A], registers[B], registers[PC], registers[SP]);
  fprintf(file, "Memory:\n");
  for(i = MEMORYLIMIT; i >= 0; i--) {
    fprintf(file, "%03X: %08X ", i, memory[i]);
    if(i == registers[SP]) {
      fprintf(file, "<-SP  "); 
    }
    if(i == registers[PC]) {
      fprintf(file, "<-PC  "); 
    }
    fprintf(file, "\n");
  }
  fclose(file);
}

// Runs the emulator program until HALT or an error is found, unless an undetected infinite loop occurs.
void RunProgram(CODE *current, int memory[MEMORYLIMIT], int registers[MAX], CODE *start) 
{
  while(1) {
    if(current->opcode == 18) {
      ApplyInstruction(current, memory, registers);
      break;
    }
    ApplyInstruction(current, memory, registers);
    printf("A = %08X, B = %08X, PC = %03X, SP = %03X.\n", registers[A], registers[B], registers[PC], registers[SP]);
    current = FindNextInstruction(registers[PC], start);
  }
}
