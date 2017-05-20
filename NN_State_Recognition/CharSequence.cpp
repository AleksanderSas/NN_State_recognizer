#include "CharSequence.h"
#include <string>

//fist string from stream willbe added
CharSequence::CharSequence(std::ifstream &file)
{
   std::string buf;
   file >> buf;
   while (buf != "cmscore1:")
   {
      phoneSequence.push_back(buf);
      file >> buf;
   }

   currentIndex = 0;
   currentPhone = "sil";
}


CharSequence::~CharSequence()
{
}

void CharSequence::move(std::string phone)
{
   //reach sp-phone       phone   |    sil
   //                      n-3   n-2   n-1  
   if (phone == "sil" && currentIndex > 1 && currentIndex < phoneSequence.size() - 3)
   {
      return;
   }

   if (currentPhone != phone)
   {
      currentIndex++;
      currentPhone = phone;
      if (phoneSequence.at(currentIndex) == "|")
         currentIndex++;
   }
}

WordStyle CharSequence::getWordStyle(std::string phone)
{
   //return INTERNAL;
   _ASSERT(phoneSequence.at(currentIndex) != "|");

   move(phone);
   if (phoneSequence.at(currentIndex) == "sil")
      return ONE_CHAR;

   if (phoneSequence.at(currentIndex) == "|")
      throw "invalid phone position\n";

   unsigned char mask = 0;

   //don't care about index out of boundary, boundaries are 'sil's
   if (phoneSequence.at(currentIndex + 1) == "|")
      mask |= 2;
   if (phoneSequence.at(currentIndex - 1) == "|")
      mask |= 1;

   switch (mask)
   {
   case 0:
      return INTERNAL;
   case 1:
      return BEGIN;
   case 2:
      return END;
   case 3:
      return ONE_CHAR;
   default:
      return INTERNAL;
   }
}
