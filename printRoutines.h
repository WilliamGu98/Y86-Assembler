/* This file contains the prototypes and constants needed to use the
   routines defined in printRoutines.c
*/

#ifndef _PRINTROUTINES_H_
#define _PRINTROUTINES_H_

#include <stdio.h>

int samplePrint(FILE *);

void readMachineCode(FILE* out, FILE *machineCode, unsigned long startingOffset); //Maybe shouldnt go here

//Methods to deal with each instruction:
int nopCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr);
int rrmovqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr);
int cmovXXCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr);
int irmovqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr);
int rmmovqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr);
int mrmovqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr);
int opqCase(FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr);
int jxxCase(FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr);
int callCase(FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr);
int pushqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr);
int popqCase (FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned long currAddr);
int quadOrByteCase(FILE* out, FILE *machineCode, unsigned char buffer[1], unsigned char byteData[], int bytesSoFar, unsigned long currAddr);

//Helper methods:
char* bytesToString(unsigned char byteData[], int byteLength);
char* bytesToEncInstrString(unsigned char bytes[], int byteLength);
int checkRegister(unsigned char regVal);
int checkNoRegister(unsigned char regVal);
char* getRegString(unsigned char reg);
char* getOpCodeString(unsigned char reg);
char* getJmpString(unsigned char reg);
char* getCMovString(unsigned char reg);

#endif /* PRINTROUTINES */
