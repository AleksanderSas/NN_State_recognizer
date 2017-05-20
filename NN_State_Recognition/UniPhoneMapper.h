#pragma once
#include <map>
#include <string>
#include <fstream>
#include "AbstructPhoneMapper.h"

class UniPhoneMapper : public AbstructPhoneMapper
{
public:
   UniPhoneMapper();
   virtual ~UniPhoneMapper();

   //statre srange: [1,2,3]
   virtual int getStatePosition(std::string phone, int state);
   virtual int getStateNumber();
   virtual std::string getPhone(std::string &name, WordStyle wordStyle);
   virtual void saveRepoer(std::string fileName);

private:
   int UniPhoneMapper::getPhonePositionInJuliusFile(std::string phone);
};

