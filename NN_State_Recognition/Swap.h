#ifndef __swap_bytes__
#define __swap_bytes__

typedef union swap4i
{
   int value;
   char bytes[4];
} Swap4i;

typedef union swap4f
{
   float value;
   char bytes[4];
} Swap4f;

typedef union swap2i
{
   short value;
   char bytes[2];
} Swap2i;

void swapBytes4(char* bytes);
void swapBytes2(char* bytes);

int swap(int x);
short swap(short x);
float swap(float x);


#endif