#pragma once
#include "fann.h"

typedef struct fann* FANN;

class NeuronNetwork
{
public:
	NeuronNetwork(void);
	~NeuronNetwork(void);
private:
	FANN network;
};

