#pragma once
#include "AbstructPhoneMapper.h"
#include "sent\htk_hmm.h"
#include "sent\stddefs.h"
#include <map>

class TriPhoneMapper : public AbstructPhoneMapper
{
public:
   TriPhoneMapper(std::string AM_filename, std::string tiedlist_filename);
   virtual ~TriPhoneMapper();

   //statre srange: [1,2,3]
   virtual int getStatePosition(std::string phone, int state);
   virtual int getStateNumber();
   virtual std::string getPhone(std::string &name, WordStyle wordStyle);
   virtual void saveRepoer(std::string fileName);
   virtual void upadteStateOccurance(std::string phone, int state, int frameNumber);

private:
   bool load_AM_model(std::string hmmdefs_filename, std::string tiedlist_filename);
   HTK_HMM_Data* findPhoneDef(std::string phone);
   void write_phone(FILE* file, std::string phone, float x, float other, int vec_len);

   std::map<std::string, HTK_HMM_Data*> phoneMap;
   //std::map<std::string, HMM_Logical*> virtualPhoneMap;
   HTK_HMM_INFO *hmminfo;
   int total_state_number;
   std::string oryginal_triphone;
   int inaccuratePhoneMatch;
};
