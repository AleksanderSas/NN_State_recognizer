#include "DataReader.h"
#include "UniPhoneMapper.h"
#include "TriPhoneMapper.h"
#include "Test.h"


using namespace std;

//commnad line arguments
// B:\NN_State_Recognizer_jurek_data\train B:\NN_State_Recognizer_jurek_data\valid B : \NN_State_Recognizer_jurek_data\test B:\NN_data\THEANO_DATA 1.0
//B:\NN_data B:\NN_data B:\NN_data B:\NN_data 1.0

void print(int numerator, int denumerator)
{
   cout << numerator << "\\" << denumerator << "   " << ((denumerator > 0) ? 100.0 * numerator / denumerator : 0.0) << "%   ";
}

int main(int argc, char *argv[])

{
   //D:\studia\Praca_magisterska\MS_research\Speakers\5707\model/hmm_3/hmmdefs3_utf.bin
   if (argc != 7 && argc != 9)
   {
      printf("PARAMETRY:\n");
      printf("1: Katalog zawierajacy pliki mfc i out do treningu\n");
      printf("2: Katalog zawierajacy pliki mfc i out do walidacji\n");
      printf("3: Katalog zawierajacy pliki mfc i out do testu\n");
      printf("4: Katalog do zapisania danych\n");
      printf("5: Ilosc danych (liczba od 0.0 do 1.0)\n");
      printf("6: Maksymalny rozmiar paczki w MB, -1: brak ograniczenia\n");
      printf("7: [opcjonalny] Trifonowy model akustyczny, plik hmmdefs3_utf.bin (wlacza tryb trifonowy)\n");
      printf("8: [opcjonalny] Trifonowy model akustyczny, plik tiedlist_utf (wymagany z param 6)\n");
      exit(1);
   }

   int batchSize = atoi(argv[6]);
   if (batchSize == 0)
   {
      printf("ERROR: %s is invalid batch size\n", argv[6]);
      exit(1);
   }

   AbstructPhoneMapper* phoneMapper = NULL;
   try
   {
      if (argc == 9)
      //if (TRUE)
      {
         //phoneMapper = new TriPhoneMapper("D:\\studia\\Praca_magisterska\\siec_trifon\\model_180000\\hmm_3\\hmmdefs3_utf.bin",
          //  "D:\\studia\\Praca_magisterska\\siec_trifon\\model_180000\\hmm_3\\tiedlist_utf");

         phoneMapper = new TriPhoneMapper(argv[7], argv[8]);
      }
      else
      {
         phoneMapper = new UniPhoneMapper();
      }

      if (FALSE)
      {
         std::string testFileName = "D:\\test_mode.bin";
         vector<string> charSequence;
         charSequence.push_back("a");
         charSequence.push_back("l");
         charSequence.push_back("t");
         charSequence.push_back("y");
         charSequence.push_back("r");
         charSequence.push_back("n");
         charSequence.push_back("a");
         charSequence.push_back("t");
         charSequence.push_back("y");
         charSequence.push_back("v");
         charSequence.push_back("n");
         charSequence.push_back("e");

         charSequence.push_back("a");
         charSequence.push_back("l");
         charSequence.push_back("t");
         charSequence.push_back("y");
         charSequence.push_back("r");
         charSequence.push_back("n");
         charSequence.push_back("a");
         charSequence.push_back("t");
         charSequence.push_back("y");
         charSequence.push_back("v");
         charSequence.push_back("n");

         charSequence.push_back("p");
         charSequence.push_back("o");
         charSequence.push_back("d");
         charSequence.push_back("e");
         charSequence.push_back("j");
         charSequence.push_back("œ");
         charSequence.push_back("æ");
         charSequence.push_back("e");

         charSequence.push_back("j");
         charSequence.push_back("e");
         charSequence.push_back("s");
         charSequence.push_back("t");

         charSequence.push_back("ñ");
         charSequence.push_back("e");
         charSequence.push_back("u");
         charSequence.push_back("z");
         charSequence.push_back("a");
         charSequence.push_back("s");
         charSequence.push_back("a");
         charSequence.push_back("d");
         charSequence.push_back("ñ");
         charSequence.push_back("o");
         charSequence.push_back("n");
         charSequence.push_back("e");

         //j + e j - e + s e - s + t s - t
         //p+o p-o+d o-d+e d-e+j e-j+\234 j-\234+\346 \234-\346+a \346-a
         //\361+e \361-e+u e-u+z u-z+a z-a+s a-s+a s-a+d a-d+\361 d-\361+o \361-o+n o-n+e n-e
         //"a+l" "a-l+t" "l-t+e" "t-e+r" "e-r+n" "r-n+a" "n-a+t" "a-t+y" "t-y+v" "y-v+n" "v-n+e" "n-e"
         //cholecystektomia h+o h-o+l o-l+e l-e+c e-c+y c-y+s y-s+t s-t+e t-e+k e-k+t k-t+o t-o+m o-m+j m-j+a j-a
         Test(phoneMapper).save_test_file(testFileName, charSequence);
         return 0;
      }

      string _rainDirectory = argv[1];
      string _testDirectory = argv[3];
      string _validDirectory = argv[2];

      wstring trainDirectory(_rainDirectory.begin(), _rainDirectory.end());
      wstring testDirectory(_testDirectory.begin(), _testDirectory.end());
      wstring validDirectory(_validDirectory.begin(), _validDirectory.end());



      string outputDirectory = argv[4];

      float learningFraction = 1.0;
      sscanf_s(argv[5], "%f", &learningFraction);
      bool saveAsJuliusFiles = false;
      bool parseMfcFiles = true;
      //max files size in MB
      int max_size = 700;
      std::string testInfo;
      std::string validInfo;
      std::string trainInfo;

      //dataRedaers are cleaned at the end of the blocks
      {
         DataReader test(testDirectory, phoneMapper, learningFraction, parseMfcFiles);
         testInfo = test.getInfo();
#ifdef MFC
         test.save_theano_train_base(outputDirectory + "\\test_b3_mfcc", -1);
#else
         test.save_theano_train_base(outputDirectory + "\\test_b3_mfsc", -1);
#endif
      }
      {
         DataReader valid(validDirectory, phoneMapper, learningFraction, parseMfcFiles);
         validInfo = valid.getInfo();
#ifdef MFC
         valid.save_theano_train_base(outputDirectory + "\\valid_b3_mfcc", -1);
#else
         valid.save_theano_train_base(outputDirectory + "\\valid_b3_mfsc", -1);
#endif
      }
      {
         DataReader train(trainDirectory, phoneMapper, learningFraction, parseMfcFiles);
         trainInfo = train.getInfo();
#ifdef MFC
         train.save_theano_train_base(outputDirectory + "\\train_b3_mfcc", -1);
#else
         train.save_theano_train_base(outputDirectory + "\\train_b3_mfsc", batchSize);
#endif
      }

      cout << "TRAIN " << trainInfo << endl << endl;
      cout << "VALID " << validInfo << endl << endl;
      cout << "TEST  " << testInfo << endl << endl;

      phoneMapper->saveStateFrequence(outputDirectory + "\\frequence.txt");
      phoneMapper->saveRepoer(outputDirectory + "\\report.txt");

      delete phoneMapper;
   }
   catch (std::string excpetion)
   {
      cout << std::endl << excpetion << std::endl;
   }
   return 0;
}