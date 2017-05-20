#include "PhraseReader.h"
#include <cstdlib>
#include "time.h"
#include <iostream>
#include "WordStyle.h"
#include "CharSequence.h"
#pragma comment(lib, "Shlwapi.lib")

PhraseReader::PhraseReader(std::wstring _fileBaseName, AbstructPhoneMapper *_phoneMapper, bool parseOutFile, bool compressed, int &frame_num)
{
   phoneMapper = _phoneMapper;
   fileBaseName = _fileBaseName;
   rejectedFrames = 0;

   std::string str(fileBaseName.begin(), fileBaseName.end());
   str += FILE_EFX_BASE;
   char buffor[512];
   strcpy(buffor, str.c_str());

   parseBinary(buffor, compressed);
   frame_num = textData.size();

   if (parseOutFile)
   {
      std::string fileBaseName_std(fileBaseName.begin(), fileBaseName.end());
      parseOutTextFile(fileBaseName_std + ".out");
   }
}


PhraseReader::~PhraseReader(void)
{
	for (std::vector<Frame*>::iterator it = textData.begin() ; it != textData.end(); ++it)
		delete *it;
}

void PhraseReader::parseBinary(char* file_name, bool compressed)
{
   FILE* file = fopen(file_name, "rb");
   int samples = parseBinaryHeader(file, compressed);

   if (compressed)
      for (int i = 0; i < samples; i++)
      {
         parseBinaryContent(file);
      }
   else
      for (int i = 0; i < samples; i++)
      {
         parseBinaryContent_not_compreed(file);
      }
   fclose(file);

   computeNormalization();
}

//-------------------------------------- methods to parsing text data -------------------------------


/*
*  Main function to parse file with recognition
*/
void PhraseReader::parseOutTextFile(std::string fileName)
{
   std::vector<std::string>phoneSequence;
	std::ifstream file = std::ifstream(fileName);

	//just move throuth header
   //each state entry start with '['
	std::string buf = "x";

   while (buf != "phseq1:")
   {
      file >> buf;
   }

   //file >> buf;
   CharSequence charSequence(file);
   /*while (buf != "cmscore1:")
   {
      phoneSequence.push_back(buf);
      file >> buf;
   }*/

   while (buf[0] != '[')
   {
      file >> buf;
      if (!file.good())
         throw std::string("error: header corrupted");
   }
	
	while(parseOneState(file, charSequence));

	file.close();
}

/*
*  The function determine if reach word-begining, word-ending or one-char-word,
*/
WordStyle getWordStyle(std::vector<std::string> &phoneSequence, int phoneNo)
{
   _ASSERT(phoneSequence.at(phoneNo) != "|");

   if (phoneSequence.at(phoneNo) == "sil")
      return INTERNAL;

   if (phoneSequence.at(phoneNo) == "|")
      throw "invalid phone position\n";

   unsigned char mask = 0;

   //don't care about index out of boundary, boundaries are 'sil's
   if (phoneSequence.at(phoneNo + 1) == "|")
      mask &= 2;
   if (phoneSequence.at(phoneNo - 1) == "|")
      mask &= 1;

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

/*
*  The function parse one state entry,
*  returns false is reach footer
*/
bool PhraseReader::parseOneState(std::ifstream &stream, CharSequence &charSequence)
{
	std::string buf;
	std::string phone;
	int begin = 0;
	int end = 0;
	float score = 0.0;
	int stateNo;
   char tmp;
   int frameNumber = textData.size();

	stream >> begin ;
	if(!stream.good())
		return false;
		
	stream >> end >> buf >> score >> phone >> buf; 
	sscanf(buf.c_str(), "#%d", &stateNo);

   if (stateNo > 3)
   {
	   stream >> buf;
		phone = "sil";
		stateNo -= 3;
	}

   if (buf[0] == '[' && isdigit(buf[2]))
   {
      //stream.
   }
   bool foundPhone = true;
   try
   {
      phone = phoneMapper->getPhone(phone, charSequence.getWordStyle(phone));
      phoneMapper->upadteStateOccurance(phone, stateNo, end - begin + 1);
   }
   catch (std::string error)
   {
      rejectedFrames += end - begin + 1;
      foundPhone = false;
      //setlocale(LC_ALL, "");
      //fprintf(stderr, "unknown phone: %s  ", error.c_str());
   }

   //append data to feature vectors
	for(int i = begin; i <= end; i++)
	{
      if (frameNumber > i)
      {
         Frame* frame = textData.at(i);
         frame->foundPhone = foundPhone;
         frame->stateNo = stateNo;
         frame->name = phone;
      }
      else
      {
         throw std::string("error: out is too big");
      }
	}

   tmp = buf[0];
   //move to next entry, each entry starts with '['
   while (tmp != '[' && tmp != 'r')
      stream >> tmp;
   if (tmp == 'r')
   {
      if (end != frameNumber - 1 && end != frameNumber && end != frameNumber + 1)
         throw std::string("error: out is too small");
   }
	return true;
}

//-------------------------------------- methods to binary text data -------------------------------

void PhraseReader::readAndSwapVector(FILE* file, float* buffor, int size)
{
   Swap4f value;
   for (int i = 0; i < size; i++)
   {
      fread(&value.value, sizeof(int), 1, file);
      swapBytes4(value.bytes);
      buffor[i] = value.value;
   }
}


void PhraseReader::readAndSwapVector(FILE* file, short* buffor, int size)
{
   Swap2i value;
   for (int i = 0; i < size; i++)
   {
      fread(&value.value, sizeof(short), 1, file);
      swapBytes2(value.bytes);
      buffor[i] = value.value;
   }
}

int PhraseReader::parseBinaryHeader(FILE* file, bool compressed)
{
	Swap4i value;
	Swap2i value2;

	fread(&value.value, sizeof(int), 1, file);
	swapBytes4(value.bytes);
   if (compressed)
      sampleNum = value.value - 4;
   else
      sampleNum = value.value;

	fread(&value.value, sizeof(int), 1, file);
	swapBytes4(value.bytes);

	fread(&value2.value, sizeof(short), 1, file);
	swapBytes2(value2.bytes);

	fread(&value2.value, sizeof(short), 1, file);
	swapBytes2(value2.bytes);

   if (compressed)
   {
      readAndSwapVector(file, A, PARAM_LEN);
      readAndSwapVector(file, B, PARAM_LEN);
   }

   return sampleNum;
}
void PhraseReader::parseBinaryContent_not_compreed(FILE* file)
{
   float data_tmp[PARAM_LEN];
   Frame* sample = new Frame;
   readAndSwapVector(file, sample->param, PARAM_LEN);

   textData.push_back(sample);
}

void PhraseReader::parseBinaryContent(FILE* file)
{
   short data_tmp[PARAM_LEN];
   Frame* sample = new Frame;
   readAndSwapVector(file, data_tmp, PARAM_LEN);
   for (int i = 0; i < PARAM_LEN; i++)
   {
      sample->param[i] = (data_tmp[i] + B[i]) / A[i];
   }
   textData.push_back(sample);
}

int PhraseReader::size()
{
	return textData.size() - rejectedFrames;
}
Frame* PhraseReader::at(int i)
{
	return textData.at(i);
}

bool PhraseReader::save_theano_input_base(std::ofstream &stream)
{
   int value2writre = size();
   stream.write((const char*)&value2writre, sizeof(int));

   if (SAVE_LAYER_ORDER)
   {
      for (int layer = 0; layer < 3; layer++)
         for (int i = 0; i < textData.size(); i++)
            if (textData.at(i)->foundPhone)
               saveOneVector_binary(stream, i, layer);
   }
   else
   {
      for (int i = 0; i < textData.size(); i++)
         if(textData.at(i)->foundPhone)
            saveOneVector_binary(stream, i);
   }

   for (std::vector<Frame*>::iterator it = textData.begin(); it != textData.end(); ++it)
   {
      if ((*it)->foundPhone)
      {
         value2writre = phoneMapper->getStatePosition((*it)->name, (*it)->stateNo);
         stream.write((const char*)&value2writre, sizeof(int));
      }
   }
   return true;
}

void PhraseReader::saveOneVector(std::ofstream &stream, int index)
{
   Frame* frame = textData.at(index);
   for (int i = 0; i < PARAM_LEN; i++)
      stream << std::setprecision(7) << frame->param[i] << " ";
}

void PhraseReader::saveOneVector_binary(std::ofstream &stream, int index)
{
   Frame* frame = textData.at(index);
   stream.write((const char*)frame->param, sizeof(float) * PARAM_LEN);
}

void PhraseReader::saveOneVector_binary(std::ofstream &stream, int index, int layer)
{
   Frame* frame = textData.at(index);
   stream.write((const char*)(frame->param + layer * PARAM_IN_LAYER), sizeof(float) * PARAM_IN_LAYER);
}


void PhraseReader::computeNormalization()
{
   float vars[PARAM_LEN];
   float std[PARAM_LEN];
   float means[PARAM_LEN];

   for (int i = 0; i < PARAM_LEN; i++)
   {
      vars[i] = 0.0;
      means[i] = 0.0;
   }

   int samples = 0;
   computeMena(means, samples);

   for (int i = 0; i < PARAM_LEN; i++)
   {
      means[i] /= samples;
   }

   computeVar(vars, means);

   for (int i = 0; i < PARAM_LEN; i++)
   {
      std[i] = sqrt(vars[i] / samples);
   }

   for (std::vector<Frame*>::iterator it = textData.begin(); it != textData.end(); ++it)
   {
      for (int i = 0; i < PARAM_LEN; i++)
      {
         (*it)->param[i] -= means[i];
         (*it)->param[i] /= std[i];
      }
   }
}

void PhraseReader::computeMena(float *vals, int &n)
{
   for (std::vector<Frame*>::iterator it = textData.begin(); it != textData.end(); ++it)
   {
      for (int i = 0; i < PARAM_LEN; i++)
         vals[i] += (*it)->param[i];
   }
   n = textData.size();
}

void PhraseReader::computeVar(float *vals, float *means)
{
   for (std::vector<Frame*>::iterator it = textData.begin(); it != textData.end(); ++it)
   {
      for (int i = 0; i < PARAM_LEN; i++)
      {
         float x = (*it)->param[i] - means[i];
         vals[i] += x * x;
      }
   }
}

//-------------------- Recognize with cnn implemented in CNN.cpp and write to julius bin files -----------------

void PhraseReader::write_bin_content(float* values, std::ofstream &bin_file)
{
   int i = 0;
   Swap4f log_value;
   for (i = 0; i < 120; i++)
   {
      log_value.value = log(values[i]);
      swapBytes4(log_value.bytes);
      bin_file.write(log_value.bytes, sizeof(float));
   }
}


//NOT SUPPORTED !!!!
void PhraseReader::recognize(Cnn* cnn, Cnn_workspace* workspace)
{
   std::string str(fileBaseName.begin(), fileBaseName.end());
   str += ".bin";
   std::ofstream bin_file(str.c_str(), std::ios::binary);

   Swap4i value;
   Swap2i value2;
   long long int _time;

   value.value = size() - cnn->filter_context + 1;
   swapBytes4(value.bytes);
   bin_file.write(value.bytes, sizeof(unsigned int));

   value.value = 100000;
   swapBytes4(value.bytes);
   bin_file.write(value.bytes, sizeof(unsigned int));

   value2.value = 120 * 4;
   swapBytes2(value2.bytes);
   bin_file.write(value2.bytes, sizeof(unsigned short));

   value2.value = 9;
   swapBytes2(value2.bytes);
   bin_file.write(value2.bytes, sizeof(unsigned short));

   clean_workspace(workspace);
   //fill context
   std::vector<Frame*>::iterator it = textData.begin();
   for (int i = 0; i < cnn->context - 1; i++, it++)
   {
      next_frame((*it)->param, workspace);
   }

   _time = clock();

   for (std::vector<Frame*>::iterator it = textData.begin() + cnn->context - 1; it != textData.end(); ++it)
   {
      next_frame((*it)->param, workspace);
      comput(cnn, workspace);
      write_bin_content(workspace->layer_outputs[2], bin_file);
   }

   printf("TIME: \t%lld ms \twav time: %d ms\n", (clock() - _time) * 1000 / CLOCKS_PER_SEC, textData.size() * 10);

   bin_file.close();
}

int PhraseReader::getMisFound()
{
   return rejectedFrames;
}

//--------------------------------------------------------------------------------------------------------------