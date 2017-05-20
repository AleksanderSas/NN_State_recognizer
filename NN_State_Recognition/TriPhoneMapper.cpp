#include "TriPhoneMapper.h"
#include "stdio.h"
#include "Swap.h"
#include <fstream>
#include <iostream>
#include <cstdlib>


TriPhoneMapper::TriPhoneMapper(std::string hmmdefs_filename, std::string tiedlist_filename)
{
   FILE* state_order = fopen("D:\\state_order.txt", "w");
   hmminfo = hmminfo_new();
   if (!load_AM_model(hmmdefs_filename, tiedlist_filename))
   {
      //hmminfo and value* para should be cleaned
      throw std::string("Failed to load acoustic model\n");
   }

   for (HTK_HMM_Data* hmm = hmminfo->start; hmm != NULL; hmm = hmm->next)
   {
      for (int k = 0; k < hmm->state_num; k++)
      {
         if (hmm->s[k] != NULL)
         {
            hmm->s[k]->order = -1;
            hmm->s[k]->occurance = 0;
            hmm->s[k]->printed = 0;
         }
      }
   } 

   total_state_number = 0;
   int repeats = 0;
   for (HTK_HMM_Data* hmm = hmminfo->start; hmm != NULL; hmm = hmm->next)
   {
      if (strcmp(hmm->name, "sil") == 0)
      {
         int k = 0;
      }
      char *str = "s_s21";
      if (strcmp(hmm->s[1]->name, str) == 0 || strcmp(hmm->s[2]->name, str) == 0 || strcmp(hmm->s[3]->name, str) == 0)
      {
         int k = 0;
      }
      phoneMap.insert(std::pair<std::string, HTK_HMM_Data*>(hmm->name, hmm));
      for (int k = 0; k < hmm->state_num; k++)
      {
         if (hmm->s[k] != NULL)
         {
            if (hmm->s[k]->order == -1)
            {
               hmm->s[k]->order = total_state_number;
               fprintf(state_order, "%s \t%d\n", hmm->s[k]->name, hmm->s[k]->order);
               total_state_number++;
            }
            else
            {
               repeats++;
            }
         }
      }
   }

   fclose(state_order);

   //add virtula phones to the phone map
   for (HMM_Logical* hmm_logical = hmminfo->lgstart; hmm_logical != NULL; hmm_logical = hmm_logical->next)
   {
      if (hmm_logical->is_pseudo == FALSE)
      {
         if (findPhoneDef(hmm_logical->body.defined->name) == NULL)
            throw std::string("Try to load virtual phone without definition: ") + hmm_logical->name + " map to: " + hmm_logical->body.defined->name;

         phoneMap.insert(std::pair<std::string, HTK_HMM_Data*>(hmm_logical->name, hmm_logical->body.defined));
      }
   }

   /*HTK_HMM_Data* hmm = phoneMap["a"];
   printf("%s \t:= %d %d %d\n", hmm->name, hmm->s[1]->order, hmm->s[2]->order, hmm->s[3]->order);
   hmm = phoneMap["a-r+b"];
   printf("%s \t:= %d %d %d\n", hmm->name, hmm->s[1]->order, hmm->s[2]->order, hmm->s[3]->order);
   hmm = phoneMap["sil"];
   printf("%s \t:= %d %d %d\n", hmm->name, hmm->s[1]->order, hmm->s[2]->order, hmm->s[3]->order);
   */
   initOccurance();
   inaccuratePhoneMatch = 0;
}


TriPhoneMapper::~TriPhoneMapper()
{
   //hmminfo and value* para should be cleaned
}

//state range: [1,2,3]
int TriPhoneMapper::getStatePosition(std::string phone, int state)
{
   HTK_HMM_Data* hmm = NULL;
   if ((hmm = findPhoneDef(phone)) == NULL)
   {
      throw std::string("unknow phone: ") + phone + " in: " + oryginal_triphone;
   }
   _ASSERT(state == 1 || state == 2 || state == 3);
   return hmm->s[state]->order;
}

int TriPhoneMapper::getStateNumber()
{
   return total_state_number;
}

//adopted for init_phmm.c [Julius]
bool load_hmmdefs(char* hmmfilename, HTK_HMM_INFO *hmminfo)
{
   FILE *fp;
   Value* para = (Value*)malloc(sizeof(Value));
   undef_para(para);

   if ((fp = fopen_readfile(hmmfilename)) == NULL) {
      jlog("Error: init_phmm: failed to open %s\n", hmmfilename);
      return FALSE;
   }
   if (read_binhmm(fp, hmminfo, TRUE, para) == TRUE) {
   }
   if (fclose_readfile(fp) < 0) {
      jlog("Error: init_phmm: failed to close %s\n", hmmfilename);
      return FALSE;
   }
   return TRUE;
}

//adopted for init_phmm.c [Julius]
bool load_tiedlist(char* tiedlist_filename, HTK_HMM_INFO* hmminfo)
{
   FILE *fp;
   if ((fp = fopen_readfile(tiedlist_filename)) == NULL) {
      jlog("Error: init_phmm: failed to open %s\n", tiedlist_filename);
      return FALSE;
   }
   /* detect binary / ascii by the first 4 bytes */
    {
       int x;
       if (myfread(&x, sizeof(int), 1, fp) < 1) {
          jlog("Error: init_phmm: failed to read %s\n", tiedlist_filename);
          return FALSE;
       }
       if (x == 0) {
          jlog("Error: init_phmm: expected text file %s\n", tiedlist_filename);
          return FALSE;
       }
       else {
          myfrewind(fp);
       }
    }
    /* ascii format */
    jlog("Stat: init_phmm: loading ascii hmmlist\n");
    if (rdhmmlist(fp, hmminfo) == FALSE) {
       jlog("Error: init_phmm: HMMList \"%s\" read error\n", tiedlist_filename);
       return FALSE;
    }

    if (fclose_readfile(fp) < 0) {
       jlog("Error: init_phmm: failed to close %s\n", tiedlist_filename);
       return FALSE;
    }
    jlog("Stat: init_phmm: logical names: %5d in HMMList\n", hmminfo->totallogicalnum);
}

bool TriPhoneMapper::load_AM_model(std::string hmmdefs_filename, std::string tiedlist_filename)
{
   char hmmfilename[1024];
   strcpy(hmmfilename, hmmdefs_filename.c_str());

   if(load_hmmdefs(hmmfilename, hmminfo) == FALSE)
      return FALSE;

   strcpy(hmmfilename, tiedlist_filename.c_str());

   if (load_tiedlist(hmmfilename, hmminfo) == FALSE)
      return FALSE;

   return TRUE;
}

std::string TriPhoneMapper::getPhone(std::string &name, WordStyle wordStyle)
{
   oryginal_triphone = name;
   splitPhone(name);

   // try triphone c1-c2+c3
   if (phones[0][0] != '\0' && phones[1][0] != '\0' && phones[2][0] != '\0')
   {
      std::string result = std::string(phones[0]) + "-" + phones[1] + "+" + phones[2];
      if (findPhoneDef(result) != NULL)
         return result;
   }

   //within word only triphones are accepted
   if (WordStyle::INTERNAL == wordStyle)
   {
      inaccuratePhoneMatch++;
      throw name;
   }
   //try left biphone c1-c2
   if (phones[0][0] != '\0' && wordStyle == END)
   {
      std::string result = std::string(phones[0]) + "-" + phones[1];
      if (findPhoneDef(result) != NULL)
         return result;
   }
   //try right biphone c2+c3
   if (phones[2][0] != '\0' && wordStyle == BEGIN)
   {
      std::string result = std::string(phones[1]) + "+" + phones[2];
      if (findPhoneDef(result) != NULL)
         return result;
   }
   //use uniphone
   if (findPhoneDef(phones[1]) != NULL && wordStyle == ONE_CHAR)
      return std::string(phones[1]);

   inaccuratePhoneMatch++;
   throw name;
}

void TriPhoneMapper::saveRepoer(std::string fileName)
{
   FILE* file = fopen(fileName.c_str(), "w");
   fprintf(file, "inaccurate rate: %f\n", 1.0*inaccuratePhoneMatch/totalStateOccurance);

   for (HTK_HMM_Data* hmm = hmminfo->start; hmm != NULL; hmm = hmm->next)
   {
      for (int k = 0; k < hmm->state_num; k++)
      {
         if (hmm->s[k] != NULL && !hmm->s[k]->printed)
         {
            fprintf(file, "%s    \t%d  \t%f\n", hmm->s[k]->name, hmm->s[k]->occurance, 1.0 * hmm->s[k]->occurance / totalStateOccurance);
         }
      }
   }
   fprintf(file, "\n\n");
   for (std::map<std::string, HTK_HMM_Data*>::iterator it = phoneMap.begin(); it != phoneMap.end(); ++it)
   {
      std::string s = it->first.c_str();
      HTK_HMM_Data* f = it->second;
      fprintf(file, "%s [1]  \t%s   \t%d\n", it->first.c_str(), it->second->s[1]->name, stateOccurance[it->second->s[1]->order]);
      fprintf(file, "%s [2]  \t%s   \t%d\n", it->first.c_str(), it->second->s[2]->name, stateOccurance[it->second->s[2]->order]);
      fprintf(file, "%s [3]  \t%s   \t%d\n", it->first.c_str(), it->second->s[3]->name, stateOccurance[it->second->s[3]->order]);
   }

   fclose(file);
}

HTK_HMM_Data* TriPhoneMapper::findPhoneDef(std::string phone)
{
   std::map<std::string, HTK_HMM_Data*>::iterator it = phoneMap.find(phone);
   return (it != phoneMap.end()) ? it->second : NULL;
}

void TriPhoneMapper::upadteStateOccurance(std::string phone, int state, int frameNumber)
{
   AbstructPhoneMapper::upadteStateOccurance(phone, state, frameNumber);
   findPhoneDef(phone)->s[state]->occurance += frameNumber;
}
