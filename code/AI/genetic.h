/**
* @file genetic.h
* @version 1.0
* @date 20/04/2023
* @author Abdon Crespo Alvarez
* @title Genetic algorithms.
* @brief Contains the implementation of the genetic algorithms used
* in the example AI game along with the training, testing and recording
* processes and auxiliary data structures and functions.
*/
#ifndef _AIEXAMPLE_GENETIC_
#define _AIEXAMPLE_GENETIC_

#include <vector>
#include <string>
#include <cstddef>
#include <arrayfire.h>
#include "NN.h"
#include "../threadPool.h"


namespace AIExample {

	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class miningAIGame;


	/**
	* @brief Derives from 'job' class at threadPool.h. Used
	* to represent jobs related to the heavy processing parts
	* of the training process that will be completed
	* by worker threads. In this particular case, this class is created
	* for processing a fitness function applied to the individuals.
	*/
	class geneticJob : public VoxelEng::job {

	public:

		/**
		* @brief Class constructor.
		* @param The beginning of the range of individuals to process.
		* @param The end of the range of individuals to process.
		* @param The start of the array that holds the fitness value of the individuals.
		* @param The start of the array that holds the individuals indices for accessing their neural networks.
		*/
		geneticJob(std::size_t rangeStart, std::size_t rangeEnd, float* fitness, unsigned int* indices,
				   float (*evaluationFunction)(unsigned int individualID));

		/**
		* @brief Instead of creating and deleting geneticJob objects, this method allows to reassing
		* the object's attributes in order to reuse objects.
		* @brief Class constructor.
		* @param The beginning of the range of individuals to process.
		* @param The end of the range of individuals to process.
		* @param The start of the array that holds the fitness value of the individuals.
		* @param The start of the array that holds the individuals indices for accessing their neural networks.
		*/
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


	/**
	* @brief Derives from 'job' class at threadPool.h. Used
	* to represent jobs related to the heavy processing parts
	* of the training process that will be completed
	* by worker threads. In this particular case, this class is created
	* for processing the copy of individuals.
	*/
	class copyJob : public VoxelEng::job {

	public:

		/**
		* @brief Class constructor.
		* @param The beginning of the range of individuals to process.
		* @param The end of the range of individuals to process.
		* @param The start of the array that holds the fitness value of the individuals.
		* @param The start of the array that holds the individuals indices for accessing their neural networks.
		*/
		copyJob(std::size_t rangeStart, std::size_t rangeEnd, const GeneticNeuralNetwork* parent,
				std::vector<GeneticNeuralNetwork>* individuals, unsigned int* newbornInds);

		/**
		* @brief Instead of creating and deleting copyJob objects, this method allows to reassing
		* the object's attributes in order to reuse objects.
		* @param The beginning of the range of individuals to process.
		* @param The end of the range of individuals to process.
		* @param The start of the array that holds the fitness value of the individuals.
		* @param The start of the array that holds the individuals indices for accessing their neural networks.
		*/
		void setAttributes(std::size_t rangeStart, std::size_t rangeEnd, const GeneticNeuralNetwork* parent,
						   std::vector<GeneticNeuralNetwork>* individuals, unsigned int* newbornInds);

	private:

		/*
		Attributes.
		*/

		std::size_t rangeStart_,
					rangeEnd_;
		const GeneticNeuralNetwork* parent_;
		std::vector<GeneticNeuralNetwork>* individuals_;
		unsigned int* newbornInds_;


		/*
		Methods.
		*/

		void process();

	};


	/**
	* @brief Manages everything related to the genetic algorithms part of the
	* example AI game, including the training, testing and record generation
	* processes as well as the implementation of the genetic operators and the
	* modification of the neural networks that are associated with the individuals.
	*/
	class genetic {

	public:

		// Constructors.

		/**
		* @brief Class constructor.
		*/
		genetic();


		// Observers.

		/**
		* @brief Returns true if a simulation (training, testing or recording process)
		* is currently in progress or false otherwise.
		* WARNING. Not thread-safe.
		*/
		bool simInProgress() const;

		/**
		* @brief Returns the number of individuals.
		* WARNING. Not thread-safe.
		*/
		unsigned int nIndividuals() const;


		// Modifiers.

		/**
		* @brief Generate an initial population of 'nIndividuals' individuals with random weights.
		* Said weights will be randomly generated in range [rangeMin, rangeMax].
		* If 'training' is true, then the restrictions to the paremeters in case of AI training will be applied (nIndividuals must be
		* divisible by 2 for example). If false, then they will not be applied.
		*/
		void genInitPop(unsigned int nIndividuals, float rangeMin, float rangeMax, bool training);

		/**
		* @brief Sets the loaded AI game as the set game for the 'genetic' object.
		* Said loaded AI game must be loaded and selected using the 'aiGame' class
		* and said game must be represented by an 'miningAIGame' object.
		*/
		void setGame();

		/**
		* @brief Sets the network topology that all the individuals will follow for the
		* simulation.
		*/
		void setNetworkTaxonomy(const std::initializer_list<unsigned int>& sizeLayer);

		/**
		* @brief Sets the fitness function that will be used when training AI agents.
		*/
		void setFitnessFunction(float (*fitnessFunction)(unsigned int individualID));

		/**
		* @brief Set the crossover operator's split point.
		* It is unused in implementation 1 of the crossover operator.
		* Example: if we have a neural network with 5 hidden layers and the split point is set at point 2, the weights that connect the input layer (layer 0)
		* to the first hidden layer (layer 1), the ones that connect the first hidden layer to the second hidden layer (layer 2) and the ones that connect the second hidden layer
		* to the third hidden layer (layer 3) will be exchanged between the parents to create the new offspring.
		* WARNING. Must be called before startSimulation().
		*/
		void setCrossoverSplitPoint(unsigned int point);

		/**
		* @brief Set the mutation operator's parameters
		* WARNING. Must be called before startSimulation().
		*/
		void setMutationParameters(float rate, float mutationVariationMin, float mutationVariationMax);

		/**
		* @brief Set the number of thread that will parallelize the workload of various sections of the learning process
		* that are computationally expensive.
		* If nJobs is set to 0, the number of jobs will be equal to the number returned by std::thread::hardware_concurrency (this is the default value).
		* WARNING. Must be called before startSimulation().
		*/
		void setNThreads(unsigned int nJobs = 0);

		/**
		* @brief Begin the training process.
		* WARNING. There cannot be two simulations running at the same time for the same 'genetic' object.
		* No individual data is saved by this method.
		* A call to genetic::genInitPop() must be made first to generate the initial population.
		*/
		void train(unsigned int nEpochs);

		/**
		* @brief Begin the training process.
		* WARNING. There cannot be two simulations running at the same time for the same 'genetic' object.
		* If 'nEpochsPerSave' is 0, the individuals' data will be saved once all the epochs have been processed.
		* If 'nEpochsForNewWorld' is 0, a new world will be generated per epoch.
		* A call to genetic::genInitPop() must be made first to generate the initial population.
		*/
		void train(unsigned int nEpochs, unsigned int nEpochsPerSave = 0, unsigned int nEpochsForNewWorld = 0);

		/**
		* @brief Begin the testing process.
		* The average fitness of all individuals is returned.
		* The array's size is equal to the number of individuals in this object's population.
		* If 'nEpochsForNewWorld' is 0, a new world will be generated per epoch.
		* WARNING. Heap memory allocated in 'output' must be freed.
		*/
		void test(unsigned int nEpochs, float*& output, unsigned int& outputSize, unsigned int nEpochsForNewWorld = 0);

		/**
		* @brief Begin the testing process.
		* Same as void genetic::test(unsigned int nEpochs, float* output, unsigned int& outputSize) but
		* without returning the average fitness of all individuals.
		*/
		void test(unsigned int nEpochs);

		/**
		* @brief Performs one epoch without training and saves the world used for the epoch in 
		* "saves/recordingsWorlds/" + AI game name + recordName + ".terrain" along with 
		* the recording of said epoch in "records/" + AI game name + recordName + ".rec".
		*/
		void record(const std::string& recordName);

		/**
		* @brief Saves the current individuals stored in this 'genetic' object to disk in the specified directory.
		* The file extension ".aidata" is automatically appended to 'path'.
		*/
		void saveIndividualsData(const std::string& path);

		/**
		* @brief Loads the data directly from the specified file for being used in the simulation.
		* WARNING. Replaces other individual data store on this object.
		* Doesn't load data and returns false if a file in 'savePath' already exists. Loads data and returns true otherwise.
		* Returns 0 and if the file specified in 'savePath' is not found or could not be loaded properly.
		* Otherwise it loads the individuals' data and returns the number of loaded individuals.
		* The file extension ".aidata" is automatically appended to 'path'.
		* It will ask the user to specify a limit to the number of individuals to load from the file,
		* if it is equal to 0 then all the individuals found in the file will be loaded.
		*/
		int loadIndividualsData(const std::string& path);

		GeneticNeuralNetwork& individual(unsigned int individualID);


		// Destructors.

		/**
		* @brief Class destructor.
		*/
		~genetic();

	private:

		/*
		Attributes.
		*/

		bool simInProgress_,
		     saveIndsData_;
		miningAIGame* aiGame_;
		float (*evaluationFunction_)(unsigned int individualID);
		std::vector<unsigned int> sizeLayer_;
		unsigned int nIndividuals_,
					 crossoverSplitPoint_,
					 nJobs_;
		float mutationRate_, // Probability for a single gene (weight) to mutate.
			  mutationVariationMin_,
			  mutationVariationMax_;

		std::vector<GeneticNeuralNetwork> individuals_;
		af::array fitness_,
				  selected_,
			     * popInds_,
				 * newbornInds_;
		float* hostFitness_;
		unsigned int* hostSelected_,
					* hostPopInds_,
					* hostNewbornInds_;

		VoxelEng::threadPool* threadPool_;
		std::vector<geneticJob> geneticJobs_;
		std::vector<copyJob> copyJobs_;


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
		1 = Select the fittest in the population.
		*/
		void selectionOperator(unsigned int implementation);

		/*
		0 = 1-point crossover implementation.
		1 = copy the selected individual in the population.
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

		/*
		If 'nEpochsPerSave' is 0, the individuals' data will be saved once all the epochs have been processed.
		If 'nEpochsForNewWorld' is 0, a new world will be generated per epoch.
		*/
		void trainAgents(unsigned int nEpochs, unsigned int nEpochsPerSave = 0, unsigned int nEpochsForNewWorld = 0);

	};

	inline bool genetic::simInProgress() const {
	
		return simInProgress_;
	
	}

	inline unsigned int genetic::nIndividuals() const {
	
		return nIndividuals_;
	
	}

}

#endif