
#include <stdio.h>
#include <unistd.h>
#include "printRoutines.h"

/* Perform read on machine code and write Y86 interpretation to corresponding file */
void readMachineCode(FILE* out, FILE *machineCode, unsigned long startingOffset) {
  //Set address:
  unsigned long currAddr; //"program counter"
  unsigned char buffer[1]; //Currently reads 1 bytes at a time

  //Move currAddr to offset value
  for (currAddr = 0; currAddr < startingOffset; currAddr++) {
    fread(buffer, 1, 1, machineCode); //each element to be read is 1 byte, buffer has 1 element, machineCode is the input stream
  }

  //int counter = 0; //Counter for keeping track of rows for now (remove later)
  int increment; //How much to increment the curr addr value by after each switch statement
  int skipHalt = 1; //If 1, skip printing halt statement
  while (fread(buffer, 1, 1, machineCode) == 1) { //Progresses fread by one byte
    //This switch statement assumes that there is no "expected" instruction yet
    char* encodString;
    char* instrString;
    switch(buffer[0]) {
      case 0x00: //HALT
        if (skipHalt == 0){
          encodString = "00";
          instrString = "halt";
          fprintf(out, "%016lx: %-22s%-8s\n", currAddr, encodString, instrString);
        }
        skipHalt = 1;
        increment = 1;
        break;

      case 0x10: //NOP
        encodString = "10";
        instrString = "nop";
        fprintf(out, "%016lx: %-22s%-8s\n", currAddr, encodString, instrString);
        skipHalt = 0;
        increment = 1;
        break;

      case 0x20: //RRMOVQ
        skipHalt = 0;
        increment = rrmovqCase(out, machineCode, buffer, currAddr);
        break;

      case 0x21: //Add additional cases to CMOVXX
      case 0x22:
      case 0x23:
      case 0x24:
      case 0x25:
      case 0x26:
        skipHalt = 0;
        increment = cmovXXCase(out, machineCode, buffer, currAddr);
        break;

      case 0x30: //IRMOVQ
        skipHalt = 0;
        increment = irmovqCase(out, machineCode, buffer, currAddr);
        break;

      case 0x40: //RRMOVQ
        skipHalt = 0;
        increment = rmmovqCase(out, machineCode, buffer, currAddr);
        break;

      case 0x50: //MRMOVQ
        skipHalt = 0;
        increment = mrmovqCase(out, machineCode, buffer, currAddr);
        break;

      case 0x60: //Add additional cases to OPQ
      case 0x61:
      case 0x62:
      case 0x63:
      case 0x64:
      case 0x65:
      case 0x66:
        skipHalt = 0;
        increment = opqCase(out, machineCode, buffer, currAddr);
        break;

      case 0x70: //Add additional cases to JXX
      case 0x71:
      case 0x72:
      case 0x73:
      case 0x74:
      case 0x75:
      case 0x76:
        skipHalt = 0;
        increment = jxxCase(out, machineCode, buffer, currAddr);
        break;

      case 0x80: //CALL DEST
        skipHalt = 0;
        increment = callCase(out, machineCode, buffer, currAddr);
        break;

      case 0x90: //RET
        instrString = "ret";
        encodString = "90";
            fprintf(out, "%016lx: %-22s%-8s\n", currAddr, encodString, instrString);
        skipHalt = 0;
        increment = 1;
        break;

      case 0xA0: //PUSHQ
        skipHalt = 0;
        increment = pushqCase(out, machineCode, buffer, currAddr);
        break;

      case 0xB0: //POPQ
        skipHalt = 0;
        increment = popqCase(out, machineCode, buffer, currAddr);
        break;

      default:  //Quad/Byte
        skipHalt = 0;
        unsigned char byteData[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
        byteData[0] = buffer[0];
        increment = quadOrByteCase(out, machineCode, buffer, byteData, 1, currAddr);
        break;
    }
    currAddr+=increment; //Increment determined from the switch statement
  }
}

/* This is a function to handle a "rrmovq rA rB" instruction.
 * It will write the corresponding y86 info to the output file
 * Returns the number of bytes of the nop instruction or data (if quad/byte)
 */
int rrmovqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr) {
  //currAddr used in print statement handled by this function

  unsigned char byteData[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Use in case it is quad/byte
  byteData[0] = buffer[0];   //Pre-emptively put first buffered byte into bytedata

  fread(buffer, 1, 1, machineCode); //Read next byte
  byteData[1] = buffer[0]; //Fill in second bit of data

  unsigned char rA = buffer[0]>>4;   //rA is upper 4 bits
  unsigned char rB = buffer[0]&0x0F; //rB is lower 4 bits

  if (checkRegister(rA)==1 && checkRegister(rB)==1) { //If registers are valid ...
    char* encodString = bytesToEncInstrString(byteData, 2);
    fprintf(out, "%016lx: %-22s%-8s%s, %s\n", currAddr, encodString, "rrmovq", getRegString(rA), getRegString(rB));
    return 2; //rrmovqCase instruction is 2 bytes
  }
  else { //Must be data...
    return quadOrByteCase(out, machineCode, buffer, byteData, 2, currAddr); //Quad or byte case deals with print and returns number of bytes
  }
}

/* This is a function to handle a "cmovXX rA, rB" instruction.
 * It will write the corresponding y86 info to the output file
 * Returns the number of bytes of the nop instruction or data (if quad/byte)
 */
int cmovXXCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr) {
  //currAddr used in print statement handled by this function
  unsigned char byteData[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Use in case it is quad/byte
  byteData[0] = buffer[0];   //Pre-emptively put first buffered byte into bytedata
  char* cMovCode = getCMovString(buffer[0]&0xF); //Retrieve cmov condition from lower half of first byte

  fread(buffer, 1, 1, machineCode); //Read next byte
  byteData[1] = buffer[0]; //Fill in second bit of data

  unsigned char rA = buffer[0]>>4;   //rA is upper 4 bits
  unsigned char rB = buffer[0]&0x0F; //rB is lower 4 bits
  if (checkRegister(rA)==1 && checkRegister(rB)==1) { //If registers are valid ...
    char* encodString = bytesToEncInstrString(byteData, 2);
    fprintf(out, "%016lx: %-22s%-8s%s, %s\n", currAddr, encodString, cMovCode, getRegString(rA), getRegString(rB));
    return 2; //cmovXXCase instruction is 2 bytes
  }
  else { //Must be data...
    return quadOrByteCase(out, machineCode, buffer, byteData, 2, currAddr); //Quad or byte case deals with print and returns number of bytes
  }
}

/* This is a function to handle a "irmovq V, rB" instruction.
 * It will write the corresponding y86 info to the output file
 * Returns the number of bytes of the nop instruction or data (if quad/byte)
 */
int irmovqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr) {
    unsigned char byteData[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Use in case it is quad/byte
    byteData[0] = buffer[0];   //Pre-emptively put first buffered byte into bytedata

    fread(buffer, 1, 1, machineCode); //Read next byte
    byteData[1] = buffer[0];
    unsigned char rA = buffer[0]>>4;   //rA is upper 4 bits
    unsigned char rB = buffer[0]&0x0F; //rB is lower 4 bits

    if (checkNoRegister(rA)==1 && checkRegister(rB)==1) { //If registers are valid, next 8? bytes are V
        unsigned char V[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Holds imm
        for (int i=0; i<8; i++){ //Fills imm
          if (fread(buffer, 1, 1, machineCode)){
            byteData[i+2] = buffer[0];
            V[i] = buffer[0];
          }
          else{
            return quadOrByteCase(out, machineCode, buffer, byteData, i+2, currAddr);
          }
        }
        char* encodString = bytesToEncInstrString(byteData, 10);
        fprintf(out, "%016lx: %-22s%-8s$%s, %s\n", currAddr, encodString, "irmovq", bytesToString(V, 8), getRegString(rB));
        return 10; //irmovqCase instruction is 10 bytes
    }
    else { //Must be data...
        return quadOrByteCase(out, machineCode, buffer, byteData, 2, currAddr); //Quad or byte case deals with print and returns number of bytes
    }
}

/* This is a function to handle a "rmmovq rA, D (rB)" instruction.
 * It will write the corresponding y86 info to the output file
 * Returns the number of bytes of the nop instruction or data (if quad/byte)
 */
int rmmovqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr) {
    unsigned char byteData[10] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Use in case it is quad/byte
    byteData[0] = buffer[0];   //Pre-emptively put first buffered byte into bytedata

    fread(buffer, 1, 1, machineCode); //Read next byte
    byteData[1] = buffer[0];
    unsigned char rA = buffer[0]>>4;   //rA is upper 4 bits
    unsigned char rB = buffer[0]&0x0F; //rB is lower 4 bits

    if (checkRegister(rA)==1 && checkRegister(rB)==1) { //If registers are valid, next 8? bytes are V
        unsigned char D[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Holds adr base
        for (int i=0; i<8; i++){ //Fills adr base
          if (fread(buffer, 1, 1, machineCode)){
            byteData[i+2] = buffer[0];
            D[i] = buffer[0];
          }
          else{
            return quadOrByteCase(out, machineCode, buffer, byteData, i+2, currAddr);
          }
        }
        char* encodString = bytesToEncInstrString(byteData, 10);
        fprintf(out, "%016lx: %-22s%-8s%s, %s(%s)\n", currAddr, encodString, "rrmovq", getRegString(rA), bytesToString(D, 8), getRegString(rB));
        return 10; //irmovqCase instruction is 10 bytes
    }
    else { //Must be data...
        return quadOrByteCase(out, machineCode, buffer, byteData, 2, currAddr); //Quad or byte case deals with print and returns number of bytes
    }
}

/* This is a function to handle a "mrmovq D (rB), rA" instruction.
 * It will write the corresponding y86 info to the output file
 * Returns the number of bytes of the nop instruction or data (if quad/byte)
 */
int mrmovqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr) {
    unsigned char byteData[10] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Use in case it is quad/byte
    byteData[0] = buffer[0];   //Pre-emptively put first buffered byte into bytedata
    fread(buffer, 1, 1, machineCode); //Read next byte
    byteData[1] = buffer[0];
    unsigned char rA = buffer[0]>>4;   //rA is upper 4 bits
    unsigned char rB = buffer[0]&0x0F; //rB is lower 4 bits
    if (checkRegister(rA)==1 && checkRegister(rB)==1) { //If registers are valid, next 8? bytes are V
        unsigned char D[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Holds offset
        for (int i=0; i<8; i++){ //Fills offset
          if (fread(buffer, 1, 1, machineCode)){
            D[i] = buffer[0];
            byteData[i+2] = buffer[0];
          }
          else{
            return quadOrByteCase(out, machineCode, buffer, byteData, i+2, currAddr);
          }
        }
        char* encodString = bytesToEncInstrString(byteData, 10);
        fprintf(out, "%016lx: %-22s%-8s%s(%s), %s\n", currAddr, encodString, "mrmovq", bytesToString(D, 8), getRegString(rB), getRegString(rA));
        return 10; //irmovqCase instruction is 10 bytes
    }
    else { //Must be data...
        return quadOrByteCase(out, machineCode, buffer, byteData, 2, currAddr); //Quad or byte case deals with print and returns number of bytes
    }
}

/* This is a function to handle a "OPq rA, rB" instruction.
 * It will write the corresponding y86 info to the output file
 * Returns the number of bytes of the nop instruction or data (if quad/byte)
 */
int opqCase(FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr){
    unsigned char byteData[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Use in case it is quad/byte
    byteData[0] = buffer[0];   //Pre-emptively put first buffered byte into bytedata
    char* opCode = getOpCodeString(buffer[0]&0xF); //Retrieve opcode from lower half of first byte

    fread(buffer, 1, 1, machineCode); //Read next byte
    byteData[1] = buffer[0]; //Fill in second bit of data
    unsigned char rA = buffer[0]>>4;   //rA is upper 4 bits
    unsigned char rB = buffer[0]&0x0F; //rB is lower 4 bits
    if (checkRegister(rA)==1 && checkRegister(rB)==1) { //If registers are valid ...
      char* encodString = bytesToEncInstrString(byteData, 2);
      fprintf(out, "%016lx: %-22s%-8s%s, %s\n", currAddr, encodString, opCode, getRegString(rA), getRegString(rB));
      return 2; //opqCase instruction is 2 bytes
    }
    else { //Must be data...
      return quadOrByteCase(out, machineCode, buffer, byteData, 2, currAddr); //Quad or byte case deals with print and returns number of bytes
    }
}

/* This is a function to handle a "jXX Dest" instruction.
 * It will write the corresponding y86 info to the output file
 * Returns the number of bytes of the nop instruction or data (if quad/byte)
 * Note that in this instruction, it is impossible for it to be a quad (unless we run out of space?)
 */
int jxxCase(FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr){
    unsigned char byteData[9] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
    byteData[0] = buffer[0];
    char* jmpCode = getJmpString(buffer[0]&0xF); //Retrieve opcode from lower half of first byte
    unsigned char dest[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Holds offset
    for (int i=0; i<8; i++){ //Fills offset
      if (fread(buffer, 1, 1, machineCode)){
        dest[i] = buffer[0];
        byteData[i+1] = buffer[0];
      }
      else{
        return quadOrByteCase(out, machineCode, buffer, byteData, i+1, currAddr);
      }
    }
    char* encodString = bytesToEncInstrString(byteData, 9);
    fprintf(out, "%016lx: %-22s%-8s%s\n", currAddr, encodString, jmpCode, bytesToString(dest, 8));
    return 9; //jxxCase instruction is 9 bytes
}

/* This is a function to handle a "call Dest" instruction.
 * It will write the corresponding y86 info to the output file
 * Returns the number of bytes of the nop instruction or data (if quad/byte)
 * Note that in this instruction, it is impossible for it to be a quad (unless we run out of space?)
 */
int callCase(FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr){
    unsigned char byteData[9] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
    byteData[0] = buffer[0];
    unsigned char dest[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Holds offset
    for (int i=0; i<8; i++){ //Fills offset
      if (fread(buffer, 1, 1, machineCode)){
        dest[i] = buffer[0];
        byteData[i+1] = buffer[0];
      }
      else {
        return quadOrByteCase(out, machineCode, buffer, byteData, i+1, currAddr);
      }
    }
    char* encodString = bytesToEncInstrString(byteData, 9);
    fprintf(out, "%016lx: %-22s%-8s%s\n", currAddr, encodString, "call", bytesToString(dest, 8));
    return 9; //call instruction is 9 bytes
}

/* This is a function to handle a "pushq rA" instruction.
 * It will write the corresponding y86 info to the output file
 * Returns the number of bytes of the nop instruction or data (if quad/byte)
 */
int pushqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr) {
  unsigned char byteData[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Use in case it is quad/byte
  byteData[0] = buffer[0];   //Pre-emptively put first buffered byte into bytedata

  fread(buffer, 1, 1, machineCode); //Read next byte
  byteData[1] = buffer[0]; //Fill in second bit of data
  unsigned char rA = buffer[0]>>4;   //rA is upper 4 bits
  unsigned char rB = buffer[0]&0x0F; //rB is lower 4 bits
  if (checkRegister(rA)==1 && checkNoRegister(rB)==1) { //If registers are valid ...
    char* encodString = bytesToEncInstrString(byteData,2);
    fprintf(out, "%016lx: %-22s%-8s%s\n", currAddr, encodString, "pushq", getRegString(rA));
    return 2; //pushQ instruction is 2 bytes
  }
  else { //Must be data...
    return quadOrByteCase(out, machineCode, buffer, byteData, 2, currAddr); //Quad or byte case deals with print and returns number of bytes
  }
}

/* This is a function to handle a "popq rA" instruction.
 * It will write the corresponding y86 info to the output file
 * Returns the number of bytes of the nop instruction or data (if quad/byte)
 */
int popqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr) {
  //currAddr used in print statement handled by this function

  unsigned char byteData[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; //Use in case it is quad/byte
  byteData[0] = buffer[0];   //Pre-emptively put first buffered byte into bytedata

  fread(buffer, 1, 1, machineCode); //Read next byte
  byteData[1] = buffer[0]; //Fill in second bit of data
  unsigned char rA = buffer[0]>>4;   //rA is upper 4 bits
  unsigned char rB = buffer[0]&0x0F; //rB is lower 4 bits
  if (checkRegister(rA)==1 && checkNoRegister(rB)==1) { //If registers are valid ...
    char* encodString = bytesToEncInstrString(byteData,2);
    fprintf(out, "%016lx: %-22s%-8s%s\n", currAddr, encodString, "popq", getRegString(rA));
    return 2; //pushQ instruction is 2 bytes
  }
  else { //Must be data...
    return quadOrByteCase(out, machineCode, buffer, byteData, 2, currAddr); //Quad or byte case deals with print and returns number of bytes
  }
}

/* Attempt to read a quad or byte
 * Note that it reads from the machine code file so it progresses fread
 * The byteData array holds the bytes of the expected quad/byte
 * bytesSoFar represents how many valid bits already exist in byteData
 * Returns the length of the byte data and manipulates the byteData array to contain the bytes read
 * Therefore returns 8 if it is a quad, and <8 if it is a byte
 */
int quadOrByteCase(FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned char byteData[], int bytesSoFar, unsigned long currAddr) {
  int currBytes;
  char* encodString;
  for (currBytes=bytesSoFar; currBytes<8; currBytes++){
    if (fread(buffer, 1, 1, machineCode)==1){ //Take next byte
      byteData[currBytes] = buffer[0];
    }
    else {
      break;
    }
  }
  //Quad print procedure
  if (currBytes==8){
    encodString = bytesToEncInstrString(byteData,8);
    fprintf(out, "%016lx: %-22s%-8s%s\n", currAddr, encodString, ".quad", bytesToString(byteData, 8));
    return currBytes;
  }
  else if (currBytes<8){ //Byte print procedure
    for (int i=0; i<currBytes; i++){
      unsigned char oneByteData[] = {byteData[i]};
      encodString = bytesToEncInstrString(oneByteData,1);
      fprintf(out, "%016lx: %-22s%-8s%s\n", currAddr + i, encodString, ".byte", bytesToString(oneByteData, 1));
    }
    return currBytes;
  }
  else{//Quad+byte print procedure
    encodString = bytesToEncInstrString(byteData,8);
    fprintf(out, "%016lx: %-22s%-8s%s\n", currAddr, encodString, ".quad", bytesToString(byteData, 8));

    for (int i=8; i<currBytes; i++){ //Print extra bytes
      unsigned char oneByteData[] = {byteData[i]};
      encodString = bytesToEncInstrString(oneByteData,1);
      fprintf(out, "%016lx: %-22s%-8s%s\n", currAddr + i, encodString, ".byte", bytesToString(oneByteData, 1));
    }
    return currBytes;
  }
}

char* getOpCodeString(unsigned char reg) {
  switch(reg){
    case 0x0:
      return "addq";
      break;
    case 0x1:
      return "subq";
      break;
    case 0x2:
      return "andq";
      break;
    case 0x3:
      return "xorq";
      break;
    case 0x4:
      return "mulq";
      break;
    case 0x5:
      return "divq";
      break;
    case 0x6:
      return "modq";
      break;
    default:
      return NULL;
      break;
  }
}

char* getJmpString(unsigned char reg) {
  switch(reg){
    case 0x0:
      return "jmp";
      break;
    case 0x1:
      return "jle";
      break;
    case 0x2:
      return "jl";
      break;
    case 0x3:
      return "je";
      break;
    case 0x4:
      return "jne";
      break;
    case 0x5:
      return "jge";
      break;
    case 0x6:
      return "jg";
      break;
    default:
      return NULL;
      break;
  }
}

char* getCMovString(unsigned char reg) {
  switch(reg){
    case 0x1:
      return "cmovle";
      break;
    case 0x2:
      return "cmovl";
      break;
    case 0x3:
      return "cmove";
      break;
    case 0x4:
      return "cmovne";
      break;
    case 0x5:
      return "cmovge";
      break;
    case 0x6:
      return "cmovg";
      break;
    default:
      return NULL;
      break;
  }
}

 /* Converts reg value to string
  */
char* getRegString(unsigned char reg) {
    switch(reg){
      case 0x0:
        return "%rax";
        break;
      case 0x1:
        return "%rcx";
        break;
      case 0x2:
        return "%rdx";
        break;
      case 0x3:
        return "%rbx";
        break;
      case 0x4:
        return "%rsp";
        break;
      case 0x5:
        return "%rbp";
        break;
      case 0x6:
        return "%rsi";
        break;
      case 0x7:
        return "%rdi";
        break;
      case 0x8:
        return "%r8";
        break;
      case 0x9:
        return "%r9";
        break;
      case 0xA:
        return "%r10";
        break;
      case 0xB:
        return "%r11";
        break;
      case 0xC:
        return "%r12";
        break;
      case 0xD:
        return "%r13";
        break;
      case 0xE:
        return "%r14";
        break;
      default:
        return NULL;
        break;
    }
}

/* Converts byte data into a corresponding string representation from
 * byteData to byteString, in hexdecimal
 * Note that the byteString is in reverse order as displayed b/c little endian
 * so least significant bytes are read first
 * Bytelength specifies the number of bytes in the byteData array
 */
char* bytesToString(unsigned char byteData[], int byteLength) {
  int i;
  char hex[17] = "0123456789abcdef";
  static char byteString[19];
  byteString[0] = '0';
  byteString[1] = 'x';

  for (i=0; i<byteLength; i++){
    byteString[2*i+2] = hex[(byteData[byteLength-1-i]>>4)&0xF];
    byteString[2*i+3] = hex[(byteData[byteLength-1-i])&0xF];
  }
  byteString[2*i+2] = '\0';

  // Remove leading 0's:
  i=2;
  int numLeadZero = 0;
  while (byteString[i]=='0' && i<byteLength*2+2){
    numLeadZero ++;
    i++;
  }
  if (numLeadZero == byteLength*2){ //If all 0's, return 0x0
    return "0x0";
  }
  //Shift left to remove leading zeros
  for (i=2; i<(byteLength*2-numLeadZero+3); i++){
    byteString[i] = byteString[i+numLeadZero];
  }
  return byteString;
}

/* Converts bytes into a corresponding encoded instruction format
 * byteLength = number of bytes to take from array
 */
char* bytesToEncInstrString(unsigned char bytes[], int byteLength) {
  char hex[17] = "0123456789ABCDEF";
  static char instrString[21];

  for (int i=0; i<byteLength; i++){
    instrString[2*i] = hex[(bytes[i]>>4)&0xF];
    instrString[2*i+1] = hex[bytes[i]&0xF];
  }
  instrString[2*byteLength] = '\0';
  return instrString;
}

/* Given half a byte, determine if it matches that of a valid register (rax-r14 or 0x0-0xE)
 * Returns 1 if it does and 0 if not
 */
int checkRegister(unsigned char regVal){
  if (regVal==0x0 || regVal==0x1 || regVal==0x2 || regVal==0x3 || regVal==0x4 ||
    regVal==0x5 || regVal==0x6 || regVal==0x7 || regVal==0x8 || regVal==0x9 ||
    regVal==0xA || regVal==0xB || regVal==0xC || regVal==0xD || regVal==0xE) {
      return 1;
    }
  else {
    return 0;
  }
}

/* Given half a byte, determine if it matches the code of no register (0xF)
 * Returns 1 if it does and 0 if not
 */
int checkNoRegister(unsigned char regVal){
  if (regVal==0xF){
    return 1;
  }
  else {
    return 0;
  }
}
