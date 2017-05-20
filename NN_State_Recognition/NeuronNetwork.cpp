#include "NeuronNetwork.h"


NeuronNetwork::NeuronNetwork(void)
{
	network = fann_create_standard(4,2,8,9,1);
}


NeuronNetwork::~NeuronNetwork(void)
{
   fann_destroy(network);
}
