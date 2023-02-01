#include <stdio.h>
#include <stdlib.h>

struct structure {
  int hex;
  char *value;
  int byte;
}; //Made a structure in order to store the hex value, the opcode and how many bytes an instruction takes.

const struct structure arr_opCode[] = {
    {0x10, "nop", 1},    {0x90, "ret", 1},    {0x60, "addl", 2},
    {0x62, "andl", 2},   {0x61, "subl", 2},   {0x63, "xorl", 2},
    {0x20, "rrmovl", 2}, {0x21, "cmovle"},    {0x23, "cmove", 2},
    {0x22, "cmovl", 2},  {0x24, "cmovne", 2}, {0x26, "cmovg", 2},
    {0x25, "cmovge", 2}, {0xA0, "pushl", 2},  {0xB0, "popl", 2},
    {0x00, "halt", 1},   {0x30, "irmovl", 6}, {0x40, "rmmovl", 6},
    {0x50, "mrmovl", 6}, {0x70, "jmp", 5},    {0x71, "jle", 5},
    {0x72, "jl", 5},     {0x73, "je", 5},     {0x74, "jne", 5},
    {0x75, "jge", 5},    {0x76, "jg", 5},     {0x80, "call", 5}}; //Hardcoded every opcode, it's hex value and its byte size in an array.

char *arr_operands[] = {"%eax", "%ecx", "%edx", "%ebx",
                        "%esp", "%ebp", "%esi", "%edi"}; //registers.

// This is the help function that reads y86 binary code from a file
unsigned int loadBinFromFile(const char *filename, unsigned char memory[],
                             unsigned int memsize);

int main(int argc, char **argv) {
  unsigned int MAX_MEMSIZE = 4096;
  unsigned char
      memory[MAX_MEMSIZE]; // This array represents the 4KB memory space
  unsigned int PC = 0;     // This is the initial program counter address

  if (argc < 2) {
    printf(
        "Usage: please specify a y86 binary program file in the argument.\n");
    return -1;
  }

  unsigned int program_size = loadBinFromFile(argv[1], memory, MAX_MEMSIZE);

  if (program_size == 0)
    return 0;
  while (PC < program_size) {
    int flag = 0; //a variable to check if the hexadecimal instruction given matched the predefined hex values for the opcodes 
    for (int i = 0; i < 27; i++) {
      if (memory[PC] == arr_opCode[i].hex) {
        printf("%s", arr_opCode[i].value); //prints opcode
        switch (arr_opCode[i].byte) {  //checks the byte the function takes so it can check if the rest of the bytes should be used or not
        case 2: {
          int operandA = memory[PC + 1] / 0x10;
          int operandB = memory[PC + 1] % 0x10;
          if (memory[PC] == 0xA0 || memory[PC] == 0xB0) { //checks if the opcode matches pushl or popl as they do not use the second half of the second byte
            printf(" %s", arr_operands[operandA]);
          } else { //if the opcode is not pushl or popl it prints out both the operands 
            printf(" %s, %s", arr_operands[operandA], arr_operands[operandB]);
          }
          break;
        }
        case 5: {
          int dest = memory[PC + 1] + (memory[PC + 2] << 8) +
                     (memory[PC + 3] << 16) + (memory[PC + 4] << 24); //In opcodes that take in 5 bytes of memory the last 4 bytes act as a pointer towards a destination in the memory
          printf(" %u", dest); //%u is used because the destination cannot be negative
          break;
        }
        case 6: {
          int value = memory[PC + 2] + (memory[PC + 3] << 8) +
                      (memory[PC + 4] << 16) + (memory[PC + 5] << 24); //these 6 byte sized instructions contain the opcode in the first byte, registers in the second, and a signed 32 bit integer in the last 4 bytes.
          switch (arr_opCode[i].hex) { //checking through the 3 6-byte sized instructions; as all three of them are arranged in a different way.
          case 0x30: {
            int operandB = memory[PC + 1] % 0x10;
            printf(" $%d, %s", value, arr_operands[operandB]);
            break;
          }
          case 0x40: {
            int operandA = memory[PC + 1] / 0x10;
            int operandB = memory[PC + 1] % 0x10;
            printf(" %s, %d(%s)", arr_operands[operandA], value,
                   arr_operands[operandB]);
            break;
          }
          case 0x50: {
            int operandA = memory[PC + 1] / 0x10;
            int operandB = memory[PC + 1] % 0x10;
            printf(" %d(%s), %s", value, arr_operands[operandB],
                   arr_operands[operandA]);
            break;
          }
          }
          break;
        }
        }
        printf("\n");
        PC += arr_opCode[i].byte; //this increments the program counter to the amount of bytes an opcode uses. e.g, if the opcode was nop the program counter will be incremented by 1.
        flag++; // an increment in the flag variable that ensures that the byte recieved represents an opcode
        break;
      }
    }
    if(flag == 0) PC++; // if the byte recieved didn't match an opcode then this line ensures that the byte would be skipped so the program counter would jump to next byte.
  }

  return 0;
}


unsigned int loadBinFromFile(const char *filename, unsigned char memory[],
                             unsigned int memsize) {
  unsigned int bytes_read = 0;

  unsigned int file_size = 0;

  FILE *pfile = fopen(filename, "rb");

  if (!pfile) {
    printf("Unable to load file %s, please check if the path and name are "
           "correct.\n",
           filename);
    return 0;
  }

  fseek(pfile, 0, SEEK_END);
  file_size = ftell(pfile);
  rewind(pfile);

  if (file_size > memsize) {
    printf("Program size exceeds memory size of %d.\n", memsize);
    return 0;
  }

  bytes_read = fread(memory, 1, file_size, pfile);

  if (bytes_read != file_size) {
    printf("Bytes read does not match the file size.\n");
    return 0;
  }

  fclose(pfile);

  return bytes_read;
}
