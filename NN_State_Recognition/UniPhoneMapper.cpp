#include "UniPhoneMapper.h"


UniPhoneMapper::UniPhoneMapper()
{
   initOccurance();
}

UniPhoneMapper::~UniPhoneMapper()
{
}

//statre srange: [1,2,3]
int UniPhoneMapper::getStatePosition(std::string phone, int state)
{
   return getPhonePositionInJuliusFile(phone) * 3 + state - 1;
}

int UniPhoneMapper::getPhonePositionInJuliusFile(std::string phone)
{
   int phone_len = phone.size();
   wchar_t letter = phone[0];
   //there are no letters 'q', 'w', 'x'
   int basic_letter_num = 'z' - 'a' - 3;
   if (phone_len == 1)
   {
      //a-p
      if (letter >= 'a' && letter <= 'p')
         return letter - 'a';

      //r-v, no 'q' in julius dictioanry
      if (letter >= 'r' && letter <= 'v')
         return letter - 'a' - 1;

      //no 'w', 'x'
      if (letter >= 'y' && letter <= 'z')
         return letter - 'a' - 3;
   }

   if (phone =="dŸ")
      return basic_letter_num + 1;

   if (phone =="d¿")
      return basic_letter_num + 2;

   if (phone =="cz")
      return basic_letter_num + 3;

   if (phone =="dz")
      return basic_letter_num + 4;

   if (phone =="e~")
      return basic_letter_num + 5;

   if (phone =="g^")
      return basic_letter_num + 6;

   if (phone =="k^")
      return basic_letter_num + 7;

   if (phone =="nn")
      return basic_letter_num + 8;

   if (phone =="o~")
      return basic_letter_num + 9;

   if (phone =="sz")
      return basic_letter_num + 10;

   if (phone =="œ")
      return basic_letter_num + 11;

   if (phone =="Ÿ")
      return basic_letter_num + 12;

   if (phone =="³")
      return basic_letter_num + 13;

   if (phone =="¿")
      return basic_letter_num + 14;

   if (phone =="æ")
      return basic_letter_num + 15;

   if (phone =="ñ")
      return basic_letter_num + 16;

   if (phone =="sil")
      return basic_letter_num + 17;

}

int UniPhoneMapper::getStateNumber()
{
   //there are no letters 'q', 'w', 'x' 
   //there are 18 special phones
   //see function getPhonePositionInJuliusFile
   return ('z' - 'a' - 3 + 17 + 1) * 3;
}

/*
*  The function read centre phone of uni/bi/tri-phone
*/
std::string UniPhoneMapper::getPhone(std::string &name, WordStyle wordStyle)
{
   splitPhone(name);
   return std::string(phones[1]);
}

void UniPhoneMapper::saveRepoer(std::string fileName)
{

}

