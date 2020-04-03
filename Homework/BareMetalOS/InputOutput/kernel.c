/*team members: Ganesh Gajavelli*/
#include <stdio.h>

void printString(char *input);
void readString(char *line);
void readSector(char *buffer, int sector);
void handleInterrupt21(int ax, int bx, int cx, int dx);

void main(){  
    char line[80]; // Hint: this line needs to be at the top of main
               // in ansi C declarations must be at the start of a function  
    char* printToLine = "\nDisplay a string to the console using printString!\n";
    char buffer[512];

    /*print "Hello World" using raw memory commands*/
    putInMemory(0xB000, 0x8140, 'H');
    putInMemory(0xB000, 0x8141, 0x7);
    putInMemory(0xB000, 0x8142, 'e');
    putInMemory(0xB000, 0x8143, 0x7);
    putInMemory(0xB000, 0x8144, 'l');
    putInMemory(0xB000, 0x8145, 0x7);
    putInMemory(0xB000, 0x8146, 'l');
    putInMemory(0xB000, 0x8147, 0x7);
    putInMemory(0xB000, 0x8148, 'o');
    putInMemory(0xB000, 0x8149, 0x7);

    putInMemory(0xB000, 0x814A, ' ');
    putInMemory(0xB000, 0x814B, 0x7);

    putInMemory(0xB000, 0x814C, 'W');
    putInMemory(0xB000, 0x814D, 0x7);
    putInMemory(0xB000, 0x814E, 'o');
    putInMemory(0xB000, 0x814F, 0x7);
    putInMemory(0xB000, 0x8150, 'r');
    putInMemory(0xB000, 0x8151, 0x7);
    putInMemory(0xB000, 0x8152, 'l');
    putInMemory(0xB000, 0x8153, 0x7);
    putInMemory(0xB000, 0x8154, 'd');
    putInMemory(0xB000, 0x8155, 0x7);
    
    /*interrupt(0x10, 0xe*256+'Q', 0, 0, 0);*/
    makeInterrupt21();
    interrupt(0x21,0,printToLine,0,0);
    
    printToLine = "Enter a line: \0";
    makeInterrupt21();
    interrupt(0x21,0,printToLine,0,0);
    makeInterrupt21();
    interrupt(0x21,1,line,0,0);
    interrupt(0x21,0,line,0,0);

    makeInterrupt21();
    interrupt(0x21,2,buffer,30,0);
    interrupt(0x21,0,buffer,0,0);

    /*
    makeInterrupt21();
    interrupt(0x21, 0, 0, 0, 0);
    
    char line[80];
    makeInterrupt21();
    interrupt(0x21,1,line,0,0);
    interrupt(0x21,0,line,0,0);
    */
    while(1); /* never forget this */
}  

void printString(char *input){
    int i = 0;
    while (input[i] != '\0'){
        interrupt(0x10, 0xe*256+input[i], 0, 0, 0);
        i++;
    }
}

/*  Reads a line from the console using Interrupt 0x16. */
void readString(char *line){
  int i, lineLength, ax;
  char charRead, backSpace, enter;
  lineLength = 80;
  i = 0;
  ax = 0;
  backSpace = 0x8;
  enter = 0xd;
  charRead = interrupt(0x16, ax, 0, 0, 0);
  while (charRead != enter && i < lineLength-2) {
    if (charRead != backSpace) {
      interrupt(0x10, 0xe*256+charRead, 0, 0, 0);
      line[i] = charRead;
      i++;
    } else {
      i--;
      if (i >= 0) {
	interrupt(0x10, 0xe*256+charRead, 0, 0, 0);
	interrupt(0x10, 0xe*256+'\0', 0, 0, 0);
	interrupt(0x10, 0xe*256+backSpace, 0, 0, 0);
      }
      else {
	i = 0;
      }
    }
    charRead = interrupt(0x16, ax, 0, 0, 0);  
  }
  line[i] = 0xa;
  line[i+1] = 0x0;
  
  /* correctly prints a newline */
  printString("\r\n");

  return;
}

int mod(int a, int b)
{
  int temp;
  temp = a;
  while (temp >= b) {
    temp = temp-b;
  }
  return temp;
}

int div(int a, int b)
{
  int quotient;
  quotient = 0;
  while ((quotient + 1) * b <= a) {
    quotient++;
  }
  return quotient;
}

void readSector(char *buffer, int sector){
    /*
    */
    int ah = 2;
    int al = 1;
    int ax = (ah * 256) + al;
    int bx = buffer;
    int ch = div(sector,36);
    int cl = mod(sector,18)+1;
    int cx = (ch * 256) + cl;
    int dh0 = div(sector,18);
    int dh = mod(dh0,2);
    int dl = 0;
    int dx = (dh * 256) + dl;
    interrupt(0x13,ax,buffer,cx,dx);
    /* interrupt(0x13,513,&buffer,((sector/36)*256)+(mod(sector,18)+1),(mod(div(sector,18),2)*256));*/
}

void handleInterrupt21(int ax, int bx, int cx, int dx){
    /*printString("Hello World");*/
    if (ax == 0){
        char* printer = bx;
        printString(printer);
    }
    else if (ax == 1){
        char* reader = bx;
        readString(reader);
    }
    else if (ax == 2){
        char* buffer = bx;
        int sectorNum = cx;
        readSector(buffer,sectorNum);
    }
    else{
        printString("error message! Invalid first parameter");
    }
}