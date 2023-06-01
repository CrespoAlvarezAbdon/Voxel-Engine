#include "NN.h"
#include "AIGameEx1.h"
#include "../AIAPI.h"


namespace AIExample {

	// 'GeneticNeuralNetwork' class.


	void GeneticNeuralNetwork::initNetwork(const std::vector<unsigned int>& sizeLayer, float rangeMin, float rangeMax) {

		nLayers_ = sizeLayer.size();
		weights_ = std::vector<af::array>(nLayers_ - 1);
		AIExample::miningAIGame* aiGame_ = dynamic_cast<AIExample::miningAIGame*>(VoxelEng::AIAPI::aiGame::selectedGame());


		if (!aiGame_)
			VoxelEng::logger::errorLog("The selected AI game is not a 'miningAIGame'");

		// Initialize weights randomly.
		for (unsigned int i = 0; i < nLayers_ - 1; i++)
			weights_[i] = (rangeMax - rangeMin) * af::randu(af::dim4(sizeLayer[i] + 1, sizeLayer[i + 1]), af::dtype::f32, aiGame_->AIrandEng()) + rangeMin;

	}

	GeneticNeuralNetwork::GeneticNeuralNetwork() 
	: nLayers_(0) {}

	GeneticNeuralNetwork& GeneticNeuralNetwork::operator=(const GeneticNeuralNetwork& source) {
	
		nLayers_ = source.nLayers_;

		weights_ = source.weights_;

		return *this;
	
	}

	af::array GeneticNeuralNetwork::addBias(const af::array& A) {
	
		// Add +1 as bias.
		return af::join(1, af::constant(1, 1, A.type()), A);
	
	}

	af::array GeneticNeuralNetwork::forwardPropagation(const af::array& input) {

		// Process input through each layer and return the values at the output layer.
		af::array values;

		values = input;
		for (unsigned int i = 0; i < nLayers_ - 1; i++)
			values = af::sigmoid(af::matmul(addBias(values), weights_[i]));

		return values;

	}

	unsigned int GeneticNeuralNetwork::forwardPropagationMax(const af::array& input) {

		// Process input through each layer and return the values at the output layer.
		af::array values,
				  indices;

		values = input;
		for (unsigned int i = 0; i < nLayers_ - 1; i++)
			values = af::sigmoid(af::matmul(addBias(values), weights_[i]));

		af::max(values, indices, values);

		return indices.scalar<unsigned int>();

	}

	unsigned int GeneticNeuralNetwork::forwardPropagationMin(const af::array& input) {

		// Process input through each layer and return the values at the output layer.
		af::array values,
				  indices;

		values = input;
		for (unsigned int i = 0; i < nLayers_ - 1; i++)
			values = af::sigmoid(af::matmul(addBias(values), weights_[i]));

		af::min(values, indices, values);

		return indices.scalar<unsigned int>();

	}

}