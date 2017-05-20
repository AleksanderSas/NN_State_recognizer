#include "CNN.h"
#include "string.h"


static float* align_32(float* source)
{
   int offset;
   offset = ((int)source) % 32;
   if (offset != 0)
      offset = 32 - offset;
   return (float*)((int)source + offset);
}

//   allocates memor, fill with zeros and align to 32 bytes
//   sets two pointers:
//          dst is aligned (pointer to use), 
//          _dst is not aligned (just to free allocated memory)
static void align_32(float** dst, float** _dst, unsigned int byte_size)
{
   *_dst = (float*)malloc(byte_size + 64);
   memset(*_dst, 0, byte_size + 64);
   *dst = align_32(*_dst);
}

static int upper_div(int x, int y)
{
   int z = x / y;
   if (z * y != x)
      z++;
   return z;
}

//reverse elements of array
static void reverse(float* begin, float* end)
{
   while (begin < end)
   {
      float f = *begin;
      *begin = *end;
      *end = f;

      begin++;
      end--;
   }
}

//fromat weiths order, so that it's easy to perform filter computations
static void form_filter(Cnn* cnn, Filter* filter, float* weiths)
{
   int i;
   filter->w = weiths;
   for (i = 0; i < cnn->filter_layers_num; i++)
   {
      reverse(weiths + i * cnn->filter_context * cnn->filter_width, weiths + (i + 1) * cnn->filter_context * cnn->filter_width - 1);
   }
   float* tmp = (float*)malloc(sizeof(float) * cnn->filter_context * cnn->filter_width * cnn->filter_layers_num);

   int index = 0;
   for (int k = 0; k < cnn->filter_context; k++)
      for (int j = 0; j < cnn->filter_layers_num; j++)
         for (int h = 0; h < cnn->filter_width; h++)
         {
            tmp[index] = weiths[k * cnn->filter_width + j * cnn->filter_context * cnn->filter_width + h];
            index++;
         }
   memcpy(weiths, tmp, sizeof(float) * cnn->filter_context * cnn->filter_width * cnn->filter_layers_num);
   free(tmp);
}

static BOOL read_filters(Cnn* cnn, FILE* file)
{
   int i = 0;
   //load filter dimensions
   fread(&cnn->filter_num, sizeof(unsigned int), 5, file);
   cnn->filters = (Filter*)malloc(sizeof(Filter) * cnn->filter_num);

   //alocate memory for filter weiths and index them
   cnn->filter_weiths = (float*)malloc(sizeof(float) * cnn->filter_context * cnn->filter_width * cnn->filter_num * cnn->filter_layers_num);
   cnn->filter_bias = (float*)malloc(sizeof(float) * cnn->filter_num);

   fread(cnn->filter_weiths, sizeof(float), cnn->filter_context * cnn->filter_width * cnn->filter_num * cnn->filter_layers_num, file);
   fread(cnn->filter_bias, sizeof(float), cnn->filter_num, file);

   for (i = 0; i < cnn->filter_num; i++)
   {
      cnn->filters[i].bias = cnn->filter_bias[i];
      form_filter(cnn, cnn->filters + i,
         cnn->filter_weiths + (cnn->filter_context * cnn->filter_width * cnn->filter_layers_num) * i);
   }

   return TRUE;
}

static BOOL read_layer(Layer* layer, int layer_input_size, FILE* file)
{
   fread(&(layer->neuron_num), sizeof(unsigned int), 1, file);

   align_32(&layer->weith, &layer->_weith, sizeof(float) * layer_input_size * layer->neuron_num);
   align_32(&layer->bias, &layer->_bias, sizeof(float) * layer->neuron_num);

   fread(layer->weith, sizeof(float), layer_input_size * layer->neuron_num, file);
   fread(layer->bias, sizeof(float), layer->neuron_num, file);

   return TRUE;
}

static float comput_filters(Cnn* cnn, Filter* filter, float* params)
{
   float* weith = filter->w;
   float sum = 0;

   int k, m, i;
   for (i = 0; i < cnn->filter_context; i++)
      for (k = 0; k < cnn->filter_layers_num; k++)
         for (m = 0; m < cnn->filter_width; m++)
         {
            sum += params[i * PARAM_LEN + PARAM_IN_LAYER * k + m] * (*weith);
            weith++;
         }
   sum += filter->bias;

   return sum;
}

/*
PARAMS order: f1t1 f2t1 ... fnt1   f1t2 f2t2 ... f39t2   f1t3 f2t3 ... fnt3
features from one frame may consists of a logical layers
the logicas layers are usually appointed by zero, first and second derivatives:
f1d1 f2d1 f3d1, ...,fnd1, fn+1d2, fn+2d2, ..., f2nd2, f2n+1d3 f2n+2d3, ..., fn3d3
for mfcc one layer contains 13 features, total numer of features is 39

NOTE remember to change PARAM_IN_LAYER and PARAM_LEN id change feature shape
*/
static void comput_filters(float* params, Cnn* cnn, float* output)
{
   int i, j, k, m;
   float sum;
   int featur = 0;
   int output_index = 0;
   for (i = 0; i < cnn->filter_num; i++)
   {
      for (j = 0; j < PARAM_IN_LAYER - cnn->filter_width + 1; j++)
      {
         sum = comput_filters(cnn, cnn->filters + i, params + j);
         output[output_index] = (sum > 0) ? sum : 0;
         output_index++;
      }
   }
}

#ifdef USE_AVX
//in AVX version: outputs, inputs, weiths and biases must be aligned
//inputs and outputs must be lengthen, so that avx window dosn't reach and of memeory block
// 
//    NOTE:   neuron number in feed forward layer must be multiple of 8 !!!!
//
static void comput_feed_forward_layer(float* _input, Layer* layer, float* _output, int input_size)
{
   AVX* input = (AVX*)_input;
   AVX* current_input = input;
   AVX* output = (AVX*)_output;
   AVX* weith = (AVX*)layer->weith;
   AVX* bias = (AVX*)layer->bias;
   int iter_per_neuron = upper_div(input_size, 8);
   int neuron_blocks = upper_div(layer->neuron_num, 8);
   AVX mul_result;
   AVX neuron_sum;
   AVX zeros;

   int i, k;
   float sum = 0;
   zeros.avx = _mm256_setzero_ps();
   for (k = 0; k < layer->neuron_num; k++)
   {
      neuron_sum.avx = _mm256_setzero_ps();
      current_input = input;
      for (i = 0; i < iter_per_neuron; i++)
      {
         mul_result.avx = _mm256_mul_ps(current_input->avx, weith->avx);
         current_input++;
         weith++;
         neuron_sum.avx = _mm256_add_ps(neuron_sum.avx, mul_result.avx);
      }
      neuron_sum.avx = _mm256_hadd_ps(neuron_sum.avx, zeros.avx);
      neuron_sum.avx = _mm256_hadd_ps(neuron_sum.avx, zeros.avx);
      _output[k] = neuron_sum.val[0] + neuron_sum.val[4];
   }
   for (k = 0; k < neuron_blocks; k++)
   {
      output->avx = _mm256_add_ps(output->avx, bias->avx);
      bias++;
      output->avx = _mm256_max_ps(output->avx, zeros.avx);
      output++;
   }

}
#else
static void comput_feed_forward_layer(float* input, Layer* layer, float* output, int input_size)
{
   int i, k;
   float sum = 0;
   for (i = 0; i < layer->neuron_num; i++)
   {
      sum = 0;
      for (k = 0; k < input_size; k++)
         sum += layer->weith[input_size * i + k] * input[k];
      sum += layer->bias[i];
      output[i] = (sum > 0) ? sum : 0;
   }
}
#endif

static void comput_softmax_layer(float* _input, Layer* layer, float* _output, int input_size, float* _apriori)
{
   AVX* apriori = (AVX*)_apriori;
   AVX* input = (AVX*)_input;
   AVX* current_input = input;
   AVX* output = (AVX*)_output;
   AVX* weith = (AVX*)layer->weith;
   AVX* bias = (AVX*)layer->bias;
   int iter_per_neuron = upper_div(input_size, 8);
   int neuron_blocks = upper_div(layer->neuron_num, 8);
   AVX mul_result;
   AVX neuron_sum;
   AVX zeros;

   int i, k;
   float sum = 0;
   zeros.avx = _mm256_setzero_ps();
   for (k = 0; k < layer->neuron_num; k++)
   {
      neuron_sum.avx = _mm256_setzero_ps();
      current_input = input;
      for (i = 0; i < iter_per_neuron; i++)
      {
         mul_result.avx = _mm256_mul_ps(current_input->avx, weith->avx);
         current_input++;
         weith++;
         neuron_sum.avx = _mm256_add_ps(neuron_sum.avx, mul_result.avx);
      }
      neuron_sum.avx = _mm256_hadd_ps(neuron_sum.avx, zeros.avx);
      neuron_sum.avx = _mm256_hadd_ps(neuron_sum.avx, zeros.avx);
      _output[k] = neuron_sum.val[0] + neuron_sum.val[4];
   }
   for (k = 0; k < neuron_blocks; k++)
   {
      output->avx = _mm256_add_ps(output->avx, bias->avx);
      bias++;
      output++;
   }
   for (i = 0; i < layer->neuron_num; i++)
   {
      sum += exp(_output[i]);
   }
   for (i = 0; i < layer->neuron_num; i++)
   {
      _output[i] = exp(_output[i]) / sum;
   }
   output = (AVX*)_output;
   for (k = 0; k < neuron_blocks; k++)
   {
      output->avx = _mm256_mul_ps(output->avx, apriori->avx);
      apriori++;
      output++;
   }
}

Cnn* init_cnn(const char* file_name, const char* apriori_file_name)
{
   int i;
   FILE* file = fopen(file_name, "rb");
   if (file == NULL)
      return NULL;

   Cnn* cnn = (Cnn*)malloc(sizeof(Cnn));

   read_filters(cnn, file);
   read_layer(&cnn->layer[0], (PARAM_IN_LAYER - cnn->filter_width + 1) * cnn->filter_num, file);
   read_layer(&cnn->layer[1], cnn->layer[0].neuron_num, file);
   align_32(&cnn->_apriori, &cnn->apriori, sizeof(float) * 120);

   fclose(file);
   file = fopen(apriori_file_name, "r");
   for (i = 0; i < 120; i++)
   {
      fscanf(file, "%f", cnn->apriori + i);
      cnn->apriori[i] = 1.0 / cnn->apriori[i];
   }

   fclose(file);

   return cnn;
}

Cnn_workspace* init_workspace(Cnn* cnn)
{
   int offset, i;
   Cnn_workspace* cnn_workspace = (Cnn_workspace*)malloc(sizeof(Cnn_workspace));
   align_32(&cnn_workspace->layer_outputs[0], &cnn_workspace->_layer_outputs[0], sizeof(float) * cnn->filter_num * PARAM_IN_LAYER);
   align_32(&cnn_workspace->layer_outputs[1], &cnn_workspace->_layer_outputs[1], sizeof(float) * cnn->layer[0].neuron_num);
   align_32(&cnn_workspace->layer_outputs[2], &cnn_workspace->_layer_outputs[2], sizeof(float) * cnn->layer[1].neuron_num);

   cnn_workspace->input_buffor = (float*)malloc(sizeof(float) * MAX_VECTORS_IN_BUFFOR * PARAM_LEN);
   cnn_workspace->current_input_begin = cnn_workspace->input_buffor - cnn->context * PARAM_LEN;
   cnn_workspace->vectors_in_buffor = 0;
   cnn_workspace->context = cnn->context;

   return cnn_workspace;
}

float* comput(Cnn* cnn, Cnn_workspace* cnn_workspace)
{
   int filters_output = cnn->filter_num * (PARAM_IN_LAYER - cnn->filter_width + 1);
   comput_filters(cnn_workspace->current_input_begin, cnn, cnn_workspace->layer_outputs[0]);
   comput_feed_forward_layer(cnn_workspace->layer_outputs[0], &cnn->layer[0], cnn_workspace->layer_outputs[1], filters_output);
   comput_softmax_layer(cnn_workspace->layer_outputs[1], &cnn->layer[1], cnn_workspace->layer_outputs[2], cnn->layer[0].neuron_num, cnn->apriori);
   return cnn_workspace->layer_outputs[2];
}

void free_cnn(Cnn* cnn)
{
   free(cnn->layer[0]._bias);
   free(cnn->layer[1]._bias);
   free(cnn->layer[0]._weith);
   free(cnn->layer[1]._weith);
   free(cnn->filter_weiths);
   free(cnn->filters);
   free(cnn);
}

void free_cnn_workspace(Cnn_workspace* cnn_workspace)
{
   free(cnn_workspace->_layer_outputs[0]);
   free(cnn_workspace->_layer_outputs[1]);
   free(cnn_workspace->_layer_outputs[2]);
   free(cnn_workspace->input_buffor);
   free(cnn_workspace);
}

void next_frame(float* params, Cnn_workspace* workspace)
{
   memcpy(workspace->current_input_begin + workspace->context * PARAM_LEN, params, sizeof(float) * PARAM_LEN);
   workspace->current_input_begin += PARAM_LEN;
   workspace->vectors_in_buffor++;

   if (workspace->vectors_in_buffor == MAX_VECTORS_IN_BUFFOR)
   {
      memcpy(workspace->input_buffor, workspace->current_input_begin, sizeof(float) * PARAM_LEN * workspace->context);
      workspace->vectors_in_buffor = workspace->context;
      workspace->current_input_begin = workspace->input_buffor;
   }
}

void clean_workspace(Cnn_workspace* workspace)
{
   workspace->vectors_in_buffor = 0;
   workspace->current_input_begin = workspace->input_buffor - workspace->context * PARAM_LEN;
}
