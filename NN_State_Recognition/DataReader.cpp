#include "DataReader.h"


DataReader::DataReader(std::wstring directory, AbstructPhoneMapper *_phoneMapper, float fraction, bool parseOutFile)
{
   WIN32_FIND_DATA ffd;
   HANDLE hFind = INVALID_HANDLE_VALUE;
   std::wstring findString = directory + FILE_EFX;
   totalSize = 0;
   total_files = 0;
   rejected_files = 0;
   total_frames = 0;
   rejected_frames = 0;
   phoneMapper = _phoneMapper;
   float random;


   hFind = FindFirstFile(findString.c_str(), &ffd);

   if (INVALID_HANDLE_VALUE != hFind) 
   {
	   do
	   {
         //skipp directories
		   if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			   continue;

		   std::wstring baseName = ffd.cFileName;
		   int dotPos = baseName.rfind('.');
		   baseName = baseName.substr(0, dotPos);
         PhraseReader* phrase = NULL;
         int frame_num = 0;

         if ((random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) <= fraction)
         {
            try
            {
               //read data from each pair .mfc/mfsc .out
               total_files++;
               phrase = new PhraseReader(directory + L"\\" + baseName, _phoneMapper, parseOutFile, COMPRESSED, frame_num);
               phrases.push_back(phrase);
               std::wcout << ffd.cFileName << "\t";
               if (totalSize % 7 == 0)
                  std::wcout << std::endl;
               totalSize++;
            }
            catch (std::string str)
            {
               delete phrase;
               rejected_frames += frame_num;
               std::wstring message(str.begin(), str.end());
               std::wcerr<< std::endl << L"error on file: " << (directory + L"\\" + baseName) << std::wstring(L" :") << message << std::endl;
               rejected_files++;
            }
            total_frames += frame_num;
         }
	   }
	   while (FindNextFile(hFind, &ffd) != 0);

	   FindClose(hFind);
   }
}


DataReader::~DataReader(void)
{
	for (std::vector<PhraseReader*>::iterator it = phrases.begin() ; it != phrases.end(); ++it)
		delete *it;
}


//end is not computed
void DataReader::save_theano_train_base(std::string dir, std::vector<PhraseReader*>::iterator begin, std::vector<PhraseReader*>::iterator end, int samples_num, int phrase_num)
{
   std::ofstream file(dir, std::ofstream::binary);

   int output_size = phoneMapper->getStateNumber();
   file.write((const char*)&samples_num, sizeof(int));
   int value2write = phrase_num;
   file.write((const char*)&value2write, sizeof(int));
   value2write = PARAM_LEN;
   file.write((const char*)&value2write, sizeof(int));
   file.write((const char*)&output_size, sizeof(int));

   for (std::vector<PhraseReader*>::iterator it = begin; it != end; ++it)
      (*it)->save_theano_input_base(file);

   printf("saving....\n");
   file.close();
}


void DataReader::save_theano_train_base(std::string dir)
{
   int totalSampleNumber = 0;
   //comput total number of samples
   for (std::vector<PhraseReader*>::iterator it = phrases.begin(); it != phrases.end(); ++it)
   {
      int phrazeSampleNum = (*it)->size();
      totalSampleNumber += phrazeSampleNum;
   }
   std::ofstream file(dir, std::ofstream::binary);
   bool process = true;

   int output_size = phoneMapper->getStateNumber();
   file.write((const char*)&totalSampleNumber, sizeof(int));
   int value2write = phrases.size();
   file.write((const char*)&value2write, sizeof(int));
   value2write = PARAM_LEN;
   file.write((const char*)&value2write, sizeof(int));
   file.write((const char*)&output_size, sizeof(int));


   for (std::vector<PhraseReader*>::iterator it = phrases.begin(); it != phrases.end(); ++it)
      (*it)->save_theano_input_base(file);

   printf("saving....\n");
   file.close();
}

// max_size: max file sieze in MB, -1 if not set
void DataReader::save_theano_train_base(std::string dir, int max_size)
{
   if (max_size == -1)
   {
      save_theano_train_base(dir + "_0");
      return;
   }
   //compute max number of verctors
   max_size = (max_size * 1000000) / (PARAM_LEN * 4 + 4);
   int totalSampleNumber = 0;
   char buffor[20];
   int index = 0;
   int phrase_counter = 0;
   //comput total number of samples
   std::vector<PhraseReader*>::iterator begin = phrases.begin();
   for (std::vector<PhraseReader*>::iterator it = phrases.begin(); it != phrases.end(); ++it)
   {
      int phrazeSampleNum = (*it)->size();
      totalSampleNumber += phrazeSampleNum;
      phrase_counter++;
      if (totalSampleNumber > max_size)
      {
         sprintf(buffor, "_%d", index++);
         save_theano_train_base(dir + buffor, begin, it, totalSampleNumber - phrazeSampleNum, phrase_counter - 1);
         totalSampleNumber = phrazeSampleNum;
         phrase_counter = 1;
         begin = it;
      }
   }
   sprintf(buffor, "_%d", index++);
   save_theano_train_base(dir + buffor, begin, phrases.end(), totalSampleNumber, phrase_counter);
}

void DataReader::recognize(Cnn* cnn, Cnn_workspace* workspce)
{
   for (std::vector<PhraseReader*>::iterator it = phrases.begin(); it != phrases.end(); it++)
      (*it)->recognize(cnn, workspce);
}

std::string print(int numerator, int denumerator)
{
   char buffor[512];
   sprintf(buffor, "%d\\%d %f %%", numerator, denumerator, (denumerator > 0) ? 100.0 * numerator / denumerator : -1);
   return std::string(buffor);
}

std::string DataReader::getInfo()
{
   int tota_mifound = 0;
   for (std::vector<PhraseReader*>::iterator it = phrases.begin(); it != phrases.end(); it++)
      tota_mifound += (*it)->getMisFound();
   std::string info("rejected: ");
   info += print(rejected_files, total_files) + std::string(" \tFRAMES: ") + print(rejected_frames, total_frames) + 
           std::string(" \tMISFOUND: ") + print(tota_mifound, total_frames);
   return info;
}