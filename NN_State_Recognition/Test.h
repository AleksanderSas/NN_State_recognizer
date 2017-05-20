#pragma once
#include "AbstructPhoneMapper.h"

class Test
{
public:
   Test(AbstructPhoneMapper *_mapper);
   ~Test();
   void save_test_file(std::string outFileName, std::vector<std::string> phones);

private:
   void write_phone(std::ofstream &file, std::string phone, float x, float other, int vec_len);

   AbstructPhoneMapper* mapper;
};

