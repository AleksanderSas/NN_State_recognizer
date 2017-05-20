#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif

#pragma ones

#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include <xmmintrin.h>
#include <immintrin.h>
#include "FeatureType.h"

#define USE_AVX
#define MAX_VECTORS_IN_BUFFOR 3900

typedef union _avx
{
   __m256 avx;
   float val[8];
} AVX;

typedef struct layer
{
   unsigned int neuron_num;
   float* _weith;          //do not use, this is not part of api
   float* _bias;           //do not use, this is not part of api
   float* weith;           //[neuron_index][weigh]
   float* bias;            //[neuron_index]
}Layer;

typedef struct filter
{
   float* w;
   float bias;
}Filter;

typedef struct _cnn
{
   //metadata order is omportant while initing, dont change
   unsigned int filter_num;
   unsigned int context;
   unsigned int filter_layers_num;
   unsigned int filter_context;
   unsigned int filter_width;

   Filter* filters;
   float* filter_weiths;   //do not use, this is not part of api
   float* filter_bias;     //[filter_index]

   float* _apriori;        //do not use, this is not part of api
   float* apriori;

   Layer layer[2];         //[layer_index]
}Cnn;

typedef struct cnn_workspace
{
   float* input_buffor;
   float* current_input_begin;
   int vectors_in_buffor;
   int context;

   float* _layer_outputs[3]; //do not use, this is not part of api
   float* layer_outputs[3];  //[layer_index][output_index]
}Cnn_workspace;

Cnn* init_cnn(const char* file_name, const char* apriori_file_name);
Cnn_workspace* init_workspace(Cnn* cnn);
float* comput(Cnn* cnn, Cnn_workspace* cnn_workspace);
void free_cnn(Cnn* cnn);
void free_cnn_workspace(Cnn_workspace* cnn_workspace);
void next_frame(float* params, Cnn_workspace* workspace);
void clean_workspace(Cnn_workspace* workspace);