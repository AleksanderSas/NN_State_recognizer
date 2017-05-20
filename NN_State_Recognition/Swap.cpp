#include "Swap.h"

void swapBytes4(char* bytes)
{
   char c1;

   c1 = bytes[0];
   bytes[0] = bytes[3];
   bytes[3] = c1;

   c1 = bytes[1];
   bytes[1] = bytes[2];
   bytes[2] = c1;
}


void swapBytes2(char* bytes)
{
   char c1;

   c1 = bytes[0];
   bytes[0] = bytes[1];
   bytes[1] = c1;
}

int swap(int x)
{
   
   Swap4i val;
   val.value = x;
   swapBytes4(val.bytes);
   return val.value;
}

short swap(short x)
{
   Swap2i val;
   val.value = x;
   swapBytes2(val.bytes);
   return val.value;
}

float swap(float x)
{
   Swap4f val;
   val.value = x;
   swapBytes4(val.bytes);
   return val.value;
}