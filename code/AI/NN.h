/**
* @file NN.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Example AI game's neural networks.
* @brief Contains the declaration of the neural networks
* used in the example AI game that is to be used alongside
* genetic algorithms.
*/
#ifndef _AI_NN_
#define _AI_NN_

#include <vector>
#include <concepts>
#include <typeinfo>
#include <random>
#include <limits>
#include <arrayfire.h>
#include <Utilities/Logger/logger.h>


namespace AIExample {

	/**
	* @brief This neural network implementation is meant to be used alongside
	* with genetic algorithms operators such as fitness function, selection, crossover,
	* mutation and replacement to modify the network's weights.
	* Implementations of some of these operators can be found in Deep dive open/code/AI/genetic.h and
	* Deep dive open/code/AI/genetic.cpp
	*/
	class GeneticNeuralNetwork {

	public:

		// Initialisers.

		/**
		* @brief Initialise the neural network with the topology described in layers (number
		* of neurons per layer) and with the respective weights initialized with
		* random values between rangeMin and rangeMax.
		*/
		void initNetwork(const std::vector<unsigned int>& sizeLayer, float rangeMin, float rangeMax);


		// Constructors.

		/**
		* @brief Create an empty neural network(zero neurons).
		*/
		GeneticNeuralNetwork();


		// Observers.

		/**
		* @brief Returns the network's weights.
		* WARNING. Not thread-safe.
		*/
		const std::vector<af::array>* weights() const;

		/**
		* @brief Returns the number of neuron layers in the network.
		* WARNING. Not thread-safe.
		*/
		unsigned int nLayers() const;

		/**
		* @brief Returns the values corresponding to the output neurons of
		* the network after propagating the 'input' values through it.
		*/
		af::array forwardPropagation(const af::array& input);

		/**
		* @brief Returns the index corresponding to the output neuron that has
		* the biggest value of the entire output layer after propagating the 'input' values through it.
		*/
		unsigned int forwardPropagationMax(const af::array& input);

		/**
		* @brief Returns the index corresponding to the output neuron that has
		* the smallest value of the entire output layer after propagating the 'input' values through it.
		*/
		unsigned int forwardPropagationMin(const af::array& input);

		/**
		* @brief Returns in 'output' the values corresponding to the output neurons of
		* the network after propagating the 'input' values through it.
		* WARNING. Do not forget to free the heap memory used to allocate the results in 'output'
		* when it is no longer needed.
		*/
		template <typename TInput, typename TOutput>
		requires std::is_arithmetic<TInput>::value && std::is_arithmetic<TOutput>::value
		void forwardPropagation(const std::vector<TInput>& input, TOutput* output, unsigned int& outputSize);

		/**
		* @brief Returns the index corresponding to the output neuron that has
		* the biggest value of the entire output layer after propagating the 'input' values through it.
		*/
		template <typename TInput>
		requires std::is_arithmetic<TInput>::value
		unsigned int forwardPropagationMax(const std::vector<TInput>& input);

		/**
		* @brief Returns the index corresponding to the output neuron that has
		* the smallest value of the entire output layer after propagating the 'input' values through it.
		*/
		template <typename TInput>
		requires std::is_arithmetic<TInput>::value
		unsigned int forwardPropagationMin(const std::vector<TInput>& input);


		// Modifiers.

		/**
		* @brief Returns the network's weights.
		* Do not forget to call GeneticNeuralNetwork::updateNLayers() on the object you called this method
		* on to update the internal counted number of the network's layers if you want to reflect any
		* changes made to said number of layers.
		* WARNING. Not thread-safe.
		*/
		std::vector<af::array>* weights();

		/**
		* @brief Update the network's number of layers once all modifications to the network's layers are finished.
		* WARNING. Not thread-safe. 
		*/
		void updateNLayers();

		/**
		* @brief Overwrites this neural network's topology and weights with the ones
		* from source.
		*/
		GeneticNeuralNetwork& operator= (const GeneticNeuralNetwork& source);

	private:

		// Attributes.
		
		unsigned int nLayers_;
		std::vector<af::array> weights_;


		// Methods.

		af::array addBias(const af::array& A);

		// NOTE. Backward propagation won't be implemented here due to the fact that 
		// this neural networks will be trained with genetic algorithms. Because
		// of this, the backpropagation from the output layer error through the
		// network is not needed in this case.

	};

	inline const std::vector<af::array>* GeneticNeuralNetwork::weights() const {
	
		return &weights_;
	
	}

	inline std::vector<af::array>* GeneticNeuralNetwork::weights() {
	
		return &weights_;
	
	}

	inline unsigned int GeneticNeuralNetwork::nLayers() const {
	
		return nLayers_;
	
	}

	inline void GeneticNeuralNetwork::updateNLayers() {
	
		nLayers_ = weights_.size() + 1;
	
	}

	template <typename TInput, typename TOutput>
	requires std::is_arithmetic<TInput>::value && std::is_arithmetic<TOutput>::value
	void GeneticNeuralNetwork::forwardPropagation(const std::vector<TInput>& input, TOutput* output, unsigned int& outputSize) {

		// Process input through each layer and return the values at the output layer.
		af::array values;

		values.write<TInput>(input.data(), input.size());
		for (int i = 0; i < nLayers_ - 1; i++)
			values = af::sigmoid(af::matmul(addBias(values), weights_[i]));

		output = values.host<TOutput>();
		outputSize = values.elements();

	}

	template <typename TInput>
	requires std::is_arithmetic<TInput>::value
	unsigned int GeneticNeuralNetwork::forwardPropagationMax(const std::vector<TInput>& input) {

		af::dtype type;
		if (typeid(TInput) == typeid(int))
			type = af::dtype::s32;
		else if (typeid(TInput) == typeid(unsigned int))
			type = af::dtype::u32;
		else if (typeid(TInput) == typeid(float))
			type = af::dtype::f32;
		else
			VoxelEng::logger::errorLog("Unknown type used!");


		// Process input through each layer and return the values at the output layer.
		af::array values(1, input.size(), type),
				  indices;

		values.write<TInput>(input.data(), input.size() * sizeof(TInput));

		for (unsigned int i = 0; i < nLayers_ - 1; i++)
			values = af::sigmoid(af::matmul(addBias(values).as(af::dtype::f32), weights_[i]));

		af::max(values, indices, values);

		return indices.scalar<unsigned int>();

	}

	template <typename TInput>
	requires std::is_arithmetic<TInput>::value
	unsigned int GeneticNeuralNetwork::forwardPropagationMin(const std::vector<TInput>& input) {

		// Process input through each layer and return the values at the output layer.
		af::array values,
				  indices;

		values.write<TInput>(input.data(), input.size());
		for (int i = 0; i < nLayers_ - 1; i++)
			values = af::sigmoid(af::matmul(addBias(values), weights_[i]));

		af::min(values, indices, values);

		return indices.scalar<unsigned int>();

	}

}

#endif