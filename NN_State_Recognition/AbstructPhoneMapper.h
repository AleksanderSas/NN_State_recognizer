#pragma once
#include "string"
#include "WordStyle.h"
#include <vector>

class AbstructPhoneMapper
{
public:
   AbstructPhoneMapper();
   virtual ~AbstructPhoneMapper();

   //state range: [1,2,3]
   virtual int getStatePosition(std::string phone, int state) = 0;
   virtual int getStateNumber() = 0;
   virtual std::string getPhone(std::string &name, WordStyle wordStyle) = 0;
   virtual void saveRepoer(std::string fileName) = 0;

   //state srange: [1,2,3]
   virtual void upadteStateOccurance(std::string phone, int state, int frameNumber);
   void saveStateFrequence(std::string fileName);

protected:
   void initOccurance();
   void splitPhone(std::string& name);

   int *stateOccurance;
   int totalStateOccurance;
   char phones[3][20];
};

