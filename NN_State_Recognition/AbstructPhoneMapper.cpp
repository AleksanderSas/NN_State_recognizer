#include "AbstructPhoneMapper.h"
#include <fstream>

AbstructPhoneMapper::AbstructPhoneMapper()
{
}


AbstructPhoneMapper::~AbstructPhoneMapper()
{
}

void AbstructPhoneMapper::initOccurance()
{
   int stateNumber = getStateNumber();
   stateOccurance = new int[stateNumber];

   memset(stateOccurance, 0, sizeof(int) * stateNumber);
   totalStateOccurance = 0;
}

//statre srange: [1,2,3]
void AbstructPhoneMapper::upadteStateOccurance(std::string phone, int state, int frameNumber)
{
   int statePosition = getStatePosition(phone, state);
   stateOccurance[statePosition] += frameNumber;
   totalStateOccurance += frameNumber;
}

void AbstructPhoneMapper::saveStateFrequence(std::string fileName)
{
   std::ofstream file(fileName);
   int totalPhoneNumber = getStateNumber();
   for (int i = 0; i < totalPhoneNumber; i++)
      file << 1.0 * stateOccurance[i] / totalStateOccurance << std::endl;
   file.close();
}

void AbstructPhoneMapper::splitPhone(std::string& name)
{
   int begin = 0, end = 0;
   int plus_l = 0, minus_l = 0, plus_r = 0, minus_r = 0;
   int size = name.size();

   if (name[0] == '{')
      begin = 1;

   for (; end < size; end++)
   {
      char c = name[end];
      if (c == '+')
      {
         plus_l = end;
         plus_r = end + 1;
      }
      if (c == '-')
      {
         minus_l = end;
         minus_r = end + 1;
      }

      if (c == '[' || c == '}' || c == '\0')
      {        
         break;
      }
   }
   if (plus_l == 0)
   {
      plus_l = end;
      plus_r = end;
   }
   phones[0][0] = '\0';
   phones[1][0] = '\0';
   phones[2][0] = '\0';
   
   strcpy(phones[0], name.substr(begin, minus_l - begin).c_str());
   strcpy(phones[1], name.substr(minus_r, plus_l - minus_r).c_str());
   strcpy(phones[2], name.substr(plus_r, end - plus_r).c_str());
}