#include "Test.h"
#include <fstream>
#include "Swap.h"

Test::Test(AbstructPhoneMapper *_mapper)
{
   mapper = _mapper;
}


Test::~Test()
{
}

void write_vec(std::ofstream &file, float x, int order, float other, int vec_len)
{
   Swap4f value;
   int pos = 0;

   while (pos < vec_len)
   {
      if (pos != order)
      {
         value.value = other;
         //swapBytes4(value.bytes);
         file.write(value.bytes, sizeof(unsigned int));
      }
      else
      {
         value.value = x;
         //swapBytes4(value.bytes);
         file.write(value.bytes, sizeof(unsigned int));
      }
      pos++;
   }
}

void Test::write_phone(std::ofstream &file, std::string phone, float x, float other, int vec_len)
{
   int order = mapper->getStatePosition(phone, 1);
   write_vec(file, x, order, other, vec_len);
   write_vec(file, x, order, other, vec_len);
   write_vec(file, x, order, other, vec_len);

   order = mapper->getStatePosition(phone, 3);
   write_vec(file, x, order, other, vec_len);
   write_vec(file, x, order, other, vec_len);

   order = mapper->getStatePosition(phone, 3);
   write_vec(file, x, order, other, vec_len);
   write_vec(file, x, order, other, vec_len);
}

void Test::save_test_file(std::string outFileName, std::vector<std::string> phones)
{
   int len = 7 * (phones.size() + 2);
   short output_vec_len = 490;
   float x = -5;
   float other = -10;
   std::string phone_name;

   std::ofstream bin_file(outFileName.c_str(), std::ios::binary);

   Swap4i value;
   Swap2i value2;
   long long int _time;

   value.value = len;
   swapBytes4(value.bytes);
   bin_file.write(value.bytes, sizeof(unsigned int));

   value.value = 100000;
   swapBytes4(value.bytes);
   bin_file.write(value.bytes, sizeof(unsigned int));

   value2.value = output_vec_len * 4;
   swapBytes2(value2.bytes);
   bin_file.write(value2.bytes, sizeof(unsigned short));

   value2.value = 9;
   swapBytes2(value2.bytes);
   bin_file.write(value2.bytes, sizeof(unsigned short));

   //begin sil
   write_phone(bin_file, "sil", x, other, output_vec_len);

   //first bi-phone
   phone_name = phones.at(0) + "+" + phones.at(1);
   write_phone(bin_file, phone_name, x, other, output_vec_len);

   //word internal content (tri-phones)
   for (int i = 1; i < phones.size() - 2; i++)
   {
      phone_name = phones.at(i - 1) + "-" + phones.at(i) + "+" + phones.at(i + 1);
      write_phone(bin_file, phone_name, x, other, output_vec_len);
   }

   //last bi-phone
   phone_name = phones.at(phones.size() - 2) + "-" + phones.at(phones.size() - 1);
   write_phone(bin_file, phone_name, x, other, output_vec_len);

   //end sil
   write_phone(bin_file, "sil", x, other, output_vec_len);

}