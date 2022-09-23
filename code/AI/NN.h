#ifndef _AI_NN_
#define _AI_NN_
#include <vector>
#include <arrayfire.h>


namespace AI {

	/*
	This neural network implementation is meant to be used alongside
	with genetic algorithms operators such as fitness function, selection, crossover,
	mutation and replacement to modify the network's weights.
	Implementations of some of these operators can be found in Deep dive open/code/AI/genetic.h and
	Deep dive open/code/AI/genetic.cpp
	*/
	class GeneticNeuralNetwork {

	public:

		// Constructors.

		/*
		Create an empty neural network(zero neurons).
		*/
		GeneticNeuralNetwork();

		/*
		Initialize the neural network with the topology described in layers (number
		of neurons per layer) and with the respective weights initialized with
		random values between rangeMin and rangeMax.
		*/
		void init(const std::vector<unsigned int>& sizeLayer, float rangeMin, float rangeMax);


		// Observers.

		const std::vector<af::array>& weights() const;

		unsigned int nLayers() const;


		// Modifiers.

		/*
		Do not forget to call GeneticNeuralNetwork::updateNLayers() on the object you called this method
		on to update the internal counted number of the network's layers if you want to reflect any
		changes made to said number of layers.
		WARNING. Not thread-safe.
		*/
		std::vector<af::array>& weights();

		/*
		WARNING. Not thread-safe.
		*/
		void updateNLayers();

		/*
		Overwrites this neural network's topology and weights with the ones
		from source.
		*/
		GeneticNeuralNetwork& operator= (const GeneticNeuralNetwork& source);

		af::array forwardPropagation(const af::array& input);

		/*
		Returns the index corresponding to the output neuron that has
		the biggest value of the entire output layer.
		*/
		unsigned int forwardPropagationMax(const af::array& input);

		/*
		Returns the index corresponding to the output neuron that has
		the biggest value of the entire output layer.
		*/
		unsigned int forwardPropagationMin(const af::array& input);

		/*
		WARNING. Do not forget to free the heap memory used to allocate the results in 'output'
		when it is no longer needed.
		*/
		template <typename TInput, typename TOutput>
		void forwardPropagation(const std::vector<TInput>& input, TOutput* output, unsigned int& outputSize);

		/*
		Returns the index corresponding to the output neuron that has
		the biggest value of the entire output layer.
		*/
		template <typename TInput>
		unsigned int forwardPropagationMax(const std::vector<TInput>& input);

		/*
		Returns the index corresponding to the output neuron that has
		the biggest value of the entire output layer.
		*/
		template <typename TInput>
		unsigned int forwardPropagationMin(const std::vector<TInput>& input);

	private:

		// Attributes.

		unsigned int nLayers_;
		std::vector<af::array> weights_;


		// Methods.

		af::array addBias(const af::array& A);

		// NOTE. Backward propagation won't be implemented here due to the fact that 
		// this neural networks will be trained with genetic algorithms. Because
		// of this, the backpropagation from the output layer error through the
		// network is useless in this case.

	};

	inline const std::vector<af::array>& GeneticNeuralNetwork::weights() const {
	
		return weights_;
	
	}

	inline std::vector<af::array>& GeneticNeuralNetwork::weights() {
	
		return weights_;
	
	}

	inline unsigned int GeneticNeuralNetwork::nLayers() const {
	
		return nLayers_;
	
	}

	inline void GeneticNeuralNetwork::updateNLayers() {
	
		nLayers_ = weights_.size();
	
	}

	template <typename TInput, typename TOutput>
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
	unsigned int GeneticNeuralNetwork::forwardPropagationMax(const std::vector<TInput>& input) {

		// Process input through each layer and return the values at the output layer.
		af::array values,
				  indices;

		values.write<TInput>(input.data(), input.size());
		for (int i = 0; i < nLayers_ - 1; i++)
			values = af::sigmoid(af::matmul(addBias(values), weights_[i]));

		(af::max)(values, indices, values); // (af::max) avoids confusing said function with the 'max' macro defined by minwindef.h.

		return indices.scalar<unsigned int>();

	}

	template <typename TInput>
	unsigned int GeneticNeuralNetwork::forwardPropagationMin(const std::vector<TInput>& input) {

		// Process input through each layer and return the values at the output layer.
		af::array values,
			indices;

		values.write<TInput>(input.data(), input.size());
		for (int i = 0; i < nLayers_ - 1; i++)
			values = af::sigmoid(af::matmul(addBias(values), weights_[i]));

		(af::min)(values, indices, values); // (af::min) avoids confusing said function with the 'min' macro defined by minwindef.h.

		return indices.scalar<unsigned int>();

	}

}

#endif