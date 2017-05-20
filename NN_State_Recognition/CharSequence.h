#pragma once

#include <fstream>
#include <vector>
#include "WordStyle.h"
#include <string>

class CharSequence
{
public:
   CharSequence(std::ifstream &file);
   ~CharSequence();

   void move(std::string phone);
   WordStyle getWordStyle(std::string phone);

private:
   std::vector<std::string>phoneSequence;
   int currentIndex;
   std::string currentPhone;
};

