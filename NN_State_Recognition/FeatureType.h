//#define MFC

//if read files from file, compressions is set permanently
#ifdef MFC
   #define PARAM_LEN 39
   #define FILE_EFX L"\\*.mfc"
   #define FILE_EFX_BASE ".mfc"
   #define COMPRESSED true
   #define PARAM_IN_LAYER 13
   //if true layers are saved separetely in files
   //data loading in python is faster
   #define SAVE_LAYER_ORDER false
#else
   #define PARAM_LEN 123
   #define FILE_EFX L"\\*.mfsc"
   #define FILE_EFX_BASE ".mfsc"
   #define COMPRESSED false
   #define PARAM_IN_LAYER 41
   //if true layers are saved separetely in files
   //data loading in python is faster
   #define SAVE_LAYER_ORDER true
#endif

