#pragma once
#include <string>
#include "stdio.h"
#include <vector>
#include "AbstructPhoneMapper.h"
#include <fstream>
#include <iostream>
#include "windows.h"
#include "Shlwapi.h"
#include "ctype.h"
#include <iomanip>
#include <math.h>
#include "CNN.h"
#include "CharSequence.h"
#include "Swap.h"

typedef struct frame
{
   float param[PARAM_LEN];
   std::string name;
   char stateNo;
   bool foundPhone;
} Frame;

class PhraseReader
{
public:
   PhraseReader(std::wstring fileBaseName, AbstructPhoneMapper *_phoneMapper, bool parseOutFile, bool compressed, int &frame_num);
	~PhraseReader(void);

	int size();
	Frame* at(int i);
	void test();
   bool save_theano_input_base(std::ofstream &stream);
   bool save_theano_output(std::ofstream &stream, int context);
   void computeMena(float *vals, int &n);
   void computeVar(float *vals, float *means);
   void computeNormalization();
   void recognize(Cnn* cnn, Cnn_workspace* workspce);
   int getMisFound();

   std::wstring fileBaseName;        //<< this is file name with path without extension
private:
   void saveOneVector(std::ofstream &stream, int index);
   void saveOneVector_binary(std::ofstream &stream, int index);
   void saveOneVector_binary(std::ofstream &stream, int index, int layer);
	void parseOutTextFile(std::string fileName);
   bool parseOneState(std::ifstream &stream, CharSequence &charSequence);
   void readAndSwapVector(FILE* file, float* buffor, int size);
   void readAndSwapVector(FILE* file, short* buffor, int size);
   void write_bin_content(float* values, std::ofstream &bin_file);

   void parseBinary(char* file_name, bool compressed);
	int parseBinaryHeader(FILE* file, bool compressed);
	void parseBinaryContent(FILE* file);
   void parseBinaryContent_not_compreed(FILE* file);

	int sampleNum;
	short* data;
	float A[PARAM_LEN], B[PARAM_LEN];

   AbstructPhoneMapper *phoneMapper;
	std::vector<Frame*> textData;
   int rejectedFrames;
};

