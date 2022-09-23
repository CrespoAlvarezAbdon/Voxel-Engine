#ifndef _AIEXAMPLE_GENETIC_
#define _AIEXAMPLE_GENETIC_
#include <vector>
#include <string>
#include <cstddef>
#include <arrayfire.h>
#include "NN.h"
#include "threadPool.h"


namespace AIExample {

	/* 
	Derives from 'job' class at AI / threadPool.
	h*/
	class geneticJob : public job {

	public:

		geneticJob(std::size_t rangeStart, std::size_t rangeEnd, float* fitness, unsigned int* indices,
				   float (*evaluationFunction)(unsigned int individualID));

		void setAttributes(std::size_t rangeStart, std::size_t rangeEnd, float* fitness, unsigned int* indices,
						   float (*evaluationFunction)(unsigned int individualID));

	private:

		/*
		Attributes.
		*/

		std::size_t rangeStart_,
					rangeEnd_;
		float* fitness_;
		unsigned int * indices_;
		float (*evaluationFunction_)(unsigned int individualID);


		/*
		Methods.
		*/

		/*
		Evaluate the fitness of the assigned individuals executing
		the provided evaluation function and corresponding parameters.
		*/
		void process();


	};


	class genetic {

	public:

		// Constructors.

		genetic();


		// Observers.

		/*
		WARNING. Not thread-safe.
		*/
		bool simInProgress() const;

		/*
		WARNING. Not thread-safe.
		*/
		unsigned int nIndividuals() const;


		// Modifiers.

		/*
		Generate an initial population of 'nIndividuals' individuals with random weights.
		Said weights will be randomly generated in range [rangeMin, rangeMax].
		*/
		void genInitPop(unsigned int nIndividuals, const std::vector<unsigned int>& sizeLayer, float rangeMin, float rangeMax);

		void setFitnessFunction(float (*fitnessFunction)(unsigned int individualID));

		/*
		WARNING. Must be called before startSimulation().
		This hyperparameter is unused in implementation 1 of the crossover operator.
		Example: if we have a neural network with 5 hidden layers and the split point is set at point 2, the weights that connect the input layer (layer 0)
		to the first hidden layer (layer 1), the ones that connect the first hidden layer to the second hidden layer (layer 2) and the ones that connect the second hidden layer
		to the third hidden layer (layer 3) will be exchanged between the parents to create the new offspring.
		*/
		void setCrossoverSplitPoint(unsigned int point);

		/*
		WARNING. Must be called before startSimulation().
		This settings are used in mutation operator 0.
		*/
		void setMutationParameters(float rate, float mutationVariationMin, float mutationVariationMax);

		/*
		WARNING. Must be called before startSimulation().
		Set the number of thread jobs that will parallelize the workload of various sections of the learning process
		that are computationally expensive.
		If nJobs is set to zero, the number of jobs will be equal to the number of total CPU cores available (this is the default value).
		*/
		void setNJobs(unsigned int nJobs);

		/*
		WARNING. There cannot be two simulations running at the same time for the same 'genetic' object.
		WARNING. No individual data is saved by this method.
		A call to genetic::genInitPop() must be made first to generate the initial population.
		*/
		void train(unsigned int nEpochs);

		/*
		WARNING. There cannot be two simulations running at the same time for the same 'genetic' object.
		If 'nEpochsPerSave' is 0, the individuals' data will be saved once all the epochs have been processed.
		A call to genetic::genInitPop() must be made first to generate the initial population.
		*/
		void train(unsigned int nEpochs, unsigned int nEpochsPerSave);

		/*
		The average fitness of all individuals is returned.
		The array's size is equal to the number of individuals in this object's population.
		WARNING. Heap memory allocated in 'output' must be freed.
		*/
		void test(unsigned int nEpochs, float* output, unsigned int& outputSize);

		/*
		Same as void genetic::test(unsigned int nEpochs, float* output, unsigned int& outputSize) but
		without returning the average fitnees of all individuals.
		*/
		void test(unsigned int nEpochs);

		/*
		Saves the current individuals stored in this 'genetic' object to disk in the specified directory.
		The file extension ".aidata" is automatically appended to 'path'.
		*/
		void saveIndividualsData(const std::string& path);

		/*
		Loads the data directly for being used in training/testing on this 'genetic' object.
		WARNING. Replaces other individual data store on this object.
		Doesn't load data and returns false if a file in 'savePath' already exists. Loads data and returns true otherwise.
		Returns false and does not load any data if the file specified in 'savePath' is not found.
		Otherwise it loads the individuals' data and returns true.
		The file extension ".aidata" is automatically appended to 'path'.
		*/
		bool loadIndividualsData(const std::string& path);

		AI::GeneticNeuralNetwork& individual(unsigned int individualID);


		// Destructors.

		~genetic();

	private:

		/*
		Attributes.
		*/

		bool simInProgress_,
		     saveIndsData_;
		float (*evaluationFunction_)(unsigned int individualID);
		unsigned int nIndividuals_,
					 crossoverSplitPoint_,
					 nLayers_,
					 nThreads_,
					 nJobs_;
		float mutationRate_, // Probability for a single gene (weight) to mutate.
			  mutationVariationMin_,
			  mutationVariationMax_;

		std::vector<AI::GeneticNeuralNetwork> individuals_;
		af::array fitness_,
				  selected_,
			     * popInds_,
				 * newbornInds_;
		float* hostFitness_;
		unsigned int* hostSelected_,
					* hostPopInds_,
					* hostNewbornInds_;

		threadPool* threadPool_;
		std::vector<geneticJob> jobs_;


		/*
		Methods.
		*/

		// Observers.

		/*
		WARNING. Throws an exception if at least one of the conditions that signals that a training/testing
		session cannot be started safetly is met. Otherwise it does nothing.
		*/
		void checkFlagsTrainOrTest();


		// Modifiers.

		/*
		Calculate the fitness function assigned to this 'genetic' object
		to all the individuals in the population and store the values.
		It uses the evaluation function provided in genInitPop().
		If useNewborn is equal to true, then the individuals that belong to the last
		generated offspring will be the ones that will have their fitness calculated.

		NOTES ON USAGE.
		hostFitness_ will have the correct fitness of the actual or the newborn population
		(depending on the value of the parameter 'useNewborn'

		*/
		void calculateFitness(bool useNewborn = false);

		/*
		0 = Roulette - wheel implementation. 
		*/
		void selectionOperator(unsigned int implementation);

		/*
		0 = 1-point crossover implementation.
		1 = copy implementation.
		*/
		void crossoverOperator(unsigned int implementation);

		/*
		0 = Random implementation. Each gene can be altered slightly independently.
		*/
		void mutationOperator(unsigned int implementation);

		/*
		0 = generational implementation. 
		1 = elitist implementation.
		*/
		void replacementOperator(unsigned int implementation);

		void trainAgents(unsigned int nEpochs, unsigned int nEpochsPerSave = 0);

	};

	inline bool genetic::simInProgress() const {
	
		return simInProgress_;
	
	}

	inline unsigned int genetic::nIndividuals() const {
	
		return nIndividuals_;
	
	}

}

#endif