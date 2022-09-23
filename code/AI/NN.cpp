#include "NN.h"


namespace AI {

	GeneticNeuralNetwork::GeneticNeuralNetwork() 
	: nLayers_(0){}
	
	void GeneticNeuralNetwork::init(const std::vector<unsigned int>& sizeLayer, float rangeMin, float rangeMax) 
	{
	
		nLayers_ = sizeLayer.size();
		weights_ = std::vector<af::array>(nLayers_ - 1);

		// Initialize weights randomly.
		for (int i = 0; i < nLayers_ - 1; i++)
			weights_[i] = (rangeMax - rangeMin) * af::randu(sizeLayer[i] + 1, sizeLayer[i + 1], f32) + rangeMin;
	
	}

	GeneticNeuralNetwork& GeneticNeuralNetwork::operator=(const GeneticNeuralNetwork& source) {
	
		nLayers_ = source.nLayers_;

		weights_ = source.weights_;

		return *this;
	
	}

	af::array GeneticNeuralNetwork::addBias(const af::array& A) {
	
		// Add +1 as bias.
		return af::join(1, af::constant(A.dims(0), 1, f32), A);
	
	}

	af::array GeneticNeuralNetwork::forwardPropagation(const af::array& input) {

		// Process input through each layer and return the values at the output layer.
		af::array values;

		values = input;
		for (int i = 0; i < nLayers_ - 1; i++)
			values = af::sigmoid(af::matmul(addBias(values), weights_[i]));

		return values;

	}

	unsigned int GeneticNeuralNetwork::forwardPropagationMax(const af::array& input) {

		// Process input through each layer and return the values at the output layer.
		af::array values,
				  indices;

		values = input;
		for (int i = 0; i < nLayers_ - 1; i++)
			values = af::sigmoid(af::matmul(addBias(values), weights_[i]));

		(af::max)(values, indices, values); // (af::max) avoids confusing said function with the 'max' macro defined by minwindef.h.

		return indices.scalar<unsigned int>();

	}

	unsigned int GeneticNeuralNetwork::forwardPropagationMin(const af::array& input) {

		// Process input through each layer and return the values at the output layer.
		af::array values,
			indices;

		values = input;
		for (int i = 0; i < nLayers_ - 1; i++)
			values = af::sigmoid(af::matmul(addBias(values), weights_[i]));

		(af::min)(values, indices, values); // (af::min) avoids confusing said function with the 'min' macro defined by minwindef.h.

		return indices.scalar<unsigned int>();

	}

}