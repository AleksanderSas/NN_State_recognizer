#pragma once
#include <string>
#include "stdio.h"
#include <vector>
#include <fstream>
#include <iostream>
#include "windows.h"
#include "PhraseReader.h"
#include "AbstructPhoneMapper.h"
#include <cstdlib>
#include "FeatureType.h"

class DataReader
{
public:
   DataReader(std::wstring directory, AbstructPhoneMapper *_phoneMapper, float fraction, bool parseOutFile);
	~DataReader(void);
   void save(std::string file_name, int context, bool F_plus_S_mode);
   void save_julius_inputs(int context, bool F_plus_S_mode, struct fann* network, std::string dir);
   void save_theano_train_base(std::string dir, int max_size);
   void save_theano_train_base(std::string dir);
   void save_theano_train_base(std::string dir, std::vector<PhraseReader*>::iterator begin, std::vector<PhraseReader*>::iterator end, int samples_num, int phrase_num);
   void recognize(Cnn* cnn, Cnn_workspace* workspce);
   std::string getInfo();
   //void computNormalization();
   //void computNormalization2();

private:
	std::vector<PhraseReader*> phrases;
   AbstructPhoneMapper* phoneMapper;
	int totalSize;
   int total_files;
   int rejected_files; 
   int total_frames;
   int rejected_frames;
};

