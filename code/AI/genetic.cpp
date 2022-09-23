#include "genetic.h"
#include <thread>
#include <ctime>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstddef>
#include "af/algorithm.h"
#include "../logger.h"
#include "../timer.h"
#include "../AIAPI.h"
#include "AIGameEx1.h"


namespace AIExample {

	///////////////////////////////////
	// Implementation file variables.//
	///////////////////////////////////

	AIExample::miningAIGame* aiGame_ = nullptr; // Selected aiGame's pointer cache.


	/////////////
	//Classses.//
	/////////////

	// 'geneticJob' class.

	geneticJob::geneticJob(size_t rangeStart, size_t rangeEnd, float* fitness, unsigned int* indices,
						   float (*evaluationFunction)(unsigned int individualID))
		: rangeStart_(rangeStart), rangeEnd_(rangeEnd), fitness_(fitness), indices_(indices), evaluationFunction_(evaluationFunction) {}

	void geneticJob::setAttributes(size_t rangeStart, size_t rangeEnd, float* fitness, unsigned int* indices,
								   float (*evaluationFunction)(unsigned int individualID)) {

		rangeStart_ = rangeStart;
		rangeEnd_ = rangeEnd;
		fitness_ = fitness;
		indices_ = indices;
		evaluationFunction_ = evaluationFunction;

	}

	void geneticJob::process() {

		for (size_t i = rangeStart_; i <= rangeEnd_; i++)
			*(fitness_ + *(indices_ + i)) = evaluationFunction_(i);
	
	}


	// 'genetic' class.

	genetic::genetic()
		: simInProgress_(false), saveIndsData_(false), crossoverSplitPoint_(0), nLayers_(0),
		nThreads_(std::thread::hardware_concurrency()), nJobs_(0), mutationRate_(0.0f), mutationVariationMin_(0.0f),
		mutationVariationMax_(0.0f), popInds_(nullptr), newbornInds_(nullptr), hostFitness_(nullptr), hostSelected_(nullptr), 
		hostPopInds_(nullptr), hostNewbornInds_(nullptr), threadPool_(nullptr) {}

	void genetic::trainAgents(unsigned int nEpochs, unsigned int nEpochsPerSave) {
	
		if (aiGame_ = dynamic_cast<AIExample::miningAIGame*>(VoxelEng::AIAPI::aiGame::selectedGame()))
			VoxelEng::logger::debugLog("Number of threads: " + std::to_string(nThreads_));
		else
			VoxelEng::logger::errorLog("The selected AI game is not a 'miningAIGame'");

		simInProgress_ = true;
		unsigned int epochSaveCounter = 0; // Used to decide when to save individuals' data.
		std::string epochString;
		VoxelEng::timer t;
		for (unsigned int epoch = 0; epoch < nEpochs; epoch++) {

			/*
			Begin epoch.
			*/
			t.start();
			aiGame_->generateAIWorld();
			aiGame_->spawnAgents();

			calculateFitness(); // Calculate fitness of the actual population.

			selectionOperator(0);

			crossoverOperator(0);

			mutationOperator(0);

			replacementOperator(1);

			t.clean();
			epochString = std::to_string(epoch);
			VoxelEng::logger::debugLog("EPOCH " + epochString + " finished");
			VoxelEng::logger::debugLog("Time: " + std::to_string(t.getDurationMs()) + " ms");

			/*
			Epoch's end.
			*/

			if (++epochSaveCounter == nEpochsPerSave && saveIndsData_) {

				saveIndividualsData("AIData/" + aiGame_->name() + "_EPOCH_" + epochString);

				epochSaveCounter = 0;

			}

		}

		// If 'nEpochsPerSave' is 0, the individuals' data will be saved once all the epochs have been processed.
		if (saveIndsData_ && nEpochsPerSave == 0)
			saveIndividualsData("AIData/" + aiGame_->name() + "_EPOCH_" + epochString);

	}

	void genetic::train(unsigned int nEpochs) {

		checkFlagsTrainOrTest();
		saveIndsData_ = false;
		trainAgents(nEpochs);

	}

	void genetic::train(unsigned int nEpochs, unsigned int nEpochsPerSave = 0) {

		checkFlagsTrainOrTest();
		saveIndsData_ = true;
		trainAgents(nEpochs, nEpochsPerSave);

	}

	void genetic::test(unsigned int nEpochs, float* output, unsigned int& outputSize) {
	
		checkFlagsTrainOrTest();

		if (aiGame_ = dynamic_cast<AIExample::miningAIGame*>(VoxelEng::AIAPI::aiGame::selectedGame()))
			VoxelEng::logger::debugLog("Number of threads: " + std::to_string(nThreads_));
		else
			VoxelEng::logger::errorLog("The selected AI game is not a 'miningAIGame'");


		af::array results = af::constant(0, nIndividuals_);


		simInProgress_ = true;
		std::string epochString;
		VoxelEng::timer t;
		VoxelEng::duration totalDuration = 0,
						   duration;
		for (unsigned int epoch = 0; epoch < nEpochs; epoch++) {

			/*
			Epoch's beginning.
			*/
			t.start();
			aiGame_->generateAIWorld();
			aiGame_->spawnAgents();

			calculateFitness(); // Calculate fitness of the actual population.

			t.clean();
			duration = t.getDurationMs();
			totalDuration += duration;
			epochString = std::to_string(epoch);
			VoxelEng::logger::debugLog("EPOCH " + epochString + " finished");
			VoxelEng::logger::debugLog("Time: " + std::to_string(duration) + " ms");

			/*
			Epoch's end.
			*/
			VoxelEng::logger::debugLog("Results' size: " + std::to_string(results.elements()));
			results += fitness_(*popInds_);
			VoxelEng::logger::debugLog("Results' size now: " + std::to_string(results.elements()));

		}

		results /= nEpochs;
		output = results.host<float>();
		outputSize = results.elements();

	}

	void genetic::test(unsigned int nEpochs) {
	
		if (aiGame_ = dynamic_cast<AIExample::miningAIGame*>(VoxelEng::AIAPI::aiGame::selectedGame()))
			VoxelEng::logger::debugLog("Number of threads: " + std::to_string(nThreads_));
		else
			VoxelEng::logger::errorLog("The selected AI game is not a 'miningAIGame'");

		checkFlagsTrainOrTest();

		simInProgress_ = true;
		
		for (unsigned int i = 0; i < nEpochs; i++) {
		
			/*
			Epoch's beginning.
			*/
			aiGame_->generateAIWorld();
			aiGame_->spawnAgents();

			calculateFitness(); // Calculate fitness of the actual population.

			/*
			Epoch's end.
			*/
		
		}

	}

	void genetic::saveIndividualsData(const std::string& path) {
	
		std::string truePath = path + ".aidata",
			        saveData;
		std::ofstream saveFile(truePath);
		
		float* weightsData = nullptr;


		for (unsigned int i = 0; i < nIndividuals_; i++) { // For each individual (population or newborn).
		
			std::vector<af::array>& weights = individuals_[*(hostPopInds_ + i)].weights();

			for (unsigned int layer = 0; layer < weights.size(); layer++) { // Save for every individual's layer.
			
				weightsData = weights[layer].host<float>();

				for (unsigned int w = 0; w < weights[layer].elements(); w++) { // Every layer's weight.
				
					saveData += std::to_string(*(weightsData + w));
					saveData += '|';
				
				}

				saveData += '#';

				delete weightsData;
			
			}

			saveData += '-';
		
		}

		saveFile << saveData;
	
	}

	bool genetic::loadIndividualsData(const std::string& path) {
	
		std::string truePath = path + ".aidata";

		if (std::filesystem::exists(truePath)) {

			std::ifstream saveFile(truePath);
			std::string saveData,
					    word;


			individuals_.clear();

			// Read from disk into main memory at once.
			saveFile.seekg(0, std::ios::end);
			saveData.resize(saveFile.tellg());
			saveFile.seekg(0);
			saveFile.read(saveData.data(), saveData.size());
			saveFile.close();

			individuals_.emplace_back();
			std::vector<af::array>& weights = individuals_[0].weights();
			std::vector<unsigned int> sizeLayer;
			std::vector<float> hostLayer;
			bool readSizeLayer = false;
			unsigned int i = 0,
				layer = 0;
			float number = 0.0f;
			while (i < saveData.size()) {

				switch (saveData[i]) {

				case '-': // New individual

					individuals_.back().updateNLayers();
					individuals_.emplace_back();
					weights = individuals_.back().weights();
					layer = 0;
					readSizeLayer = true;

					break;

				case '#': // New individual's layer

					weights[layer].write(hostLayer.data(), hostLayer.size());

					if (!readSizeLayer)
						sizeLayer.push_back(hostLayer.size());

					layer++;

					hostLayer.clear();

					break;

				case '|': // New weight.

					number = std::stof(word);
					hostLayer.push_back(number);
					word = "";

					break;

				default: // Any part of a weight.

					word += saveData[i];

					break;

				}

				i++;

			}

			nIndividuals_ = individuals_.size();

			nLayers_ = individuals_[0].nLayers(); // All individuals have the same number of layers (at the moment).
			fitness_ = af::constant(0.0f, nIndividuals_ * 2, f32);
			selected_ = af::constant(0, nIndividuals_, u32);

			if (popInds_)
				delete popInds_;
			popInds_ = new af::array(af::dim4(nIndividuals_));

			if (newbornInds_)
				delete newbornInds_;
			newbornInds_ = new af::array(af::dim4(nIndividuals_)) + nIndividuals_;

			for (int i = nIndividuals_; i < nIndividuals_ * 2; i++)
				individuals_[i].init(sizeLayer, 0.0f, 0.0f);

			return true;

		}
		else
			return false;
	
	}

	void genetic::genInitPop(unsigned int nIndividuals, const std::vector<unsigned>& sizeLayer, float rangeMin, float rangeMax) {
	
		if (nIndividuals % 2 != 0)
			VoxelEng::logger::errorLog("The number of individuals must be divisible by 2");

		nIndividuals_ = nIndividuals;
		nLayers_ = sizeLayer.size();
		individuals_ = std::vector<AI::GeneticNeuralNetwork>(nIndividuals_ * 2);
		fitness_ = af::constant(0.0f, nIndividuals_ * 2, f32);
		selected_ = af::constant(0, nIndividuals_, u32);

		if (popInds_)
			delete popInds_;
		popInds_ = new af::array(af::dim4(nIndividuals_));

		if (newbornInds_)
			delete newbornInds_;
		newbornInds_ = new af::array(af::dim4(nIndividuals_)) + nIndividuals_;

		for (int i = 0; i < nIndividuals_ * 2; i++)
			individuals_[i].init(sizeLayer, rangeMin, rangeMax);
	
	}

	void genetic::setFitnessFunction(float (*fitnessFunction)(unsigned int individualID)) {
	
		if (simInProgress_)
			VoxelEng::logger::errorLog("Cannot change fitness function during a simulation");
		else
			evaluationFunction_ = fitnessFunction;
	
	}

	void genetic::setCrossoverSplitPoint(unsigned int point) {

		if (simInProgress_)
			VoxelEng::logger::errorLog("Cannot change crossover split point during a simulation");
		else {

			if (point >= nLayers_)
				VoxelEng::logger::errorLog("The crossover split point was defined in a non-existent layer");
			else
				crossoverSplitPoint_ = point;

		}

	}

	void genetic::setMutationParameters(float rate, float mutationVariationMin, float mutationVariationMax) {
	
		if (simInProgress_)
			VoxelEng::logger::errorLog("Cannot change mutation parameters during a simulation");
		else {

			if (rate < 0.0f || rate > 1.0f)
				VoxelEng::logger::errorLog("Mutation rate must be in the range [0.0f, 1.0f]");
			else
			{
		
				mutationRate_ = rate;
				mutationVariationMin_ = mutationVariationMin;
				mutationVariationMax_ = mutationVariationMax;

			}

		}

	}

	void genetic::setNJobs(unsigned int nJobs) {

		if (simInProgress_)
			VoxelEng::logger::errorLog("Cannot change the number of parallel threadpool jobs during a simulation");
		else {
		
			if (nJobs)
				nJobs_ = nJobs;
			else
				nJobs_ = nThreads_;
		
		}

	}

	void genetic::checkFlagsTrainOrTest() {
	
		if (simInProgress_)
			VoxelEng::logger::errorLog("A training session is already in progress with this 'genetic' object");

		if (!evaluationFunction_)
			VoxelEng::logger::errorLog("No evaluation function was assigned to this object");

		if (!aiGame_)
			VoxelEng::logger::errorLog("No game was assigned to the simulation");

		if (!nIndividuals_)
			VoxelEng::logger::errorLog("There is no initial population to begin training");
	
	}

	void genetic::calculateFitness(bool useNewborn) {

		// Manage the jobs to send to the thread pool.
		if (nJobs_ > jobs_.capacity())
			jobs_.reserve(nJobs_);

		// Arrayfire's gfor cannot be used since we have a std::vector of neural networks
		// because each neural network could be different in future implementations
		// an accessing an element in an af::array to index a std::vector breaks
		// the gfor GPU paralellism since the std::vector is allocated in CPU's main memory.
		// Therefore, to optimise the execution of the evaluation function on all the individuals,
		// CPU-threads will be used.
		unsigned int* hostIndices = nullptr;
		if (useNewborn) {

			newbornInds_->host(hostNewbornInds_);
			hostIndices = hostNewbornInds_;
			
		}
		else {

			popInds_->host(hostPopInds_);
			hostIndices = hostPopInds_;

		}

		size_t rangeStart = 0,
			   rangeEnd = 0,
			   nConstructedJobs = jobs_.size();
		fitness_.host(hostFitness_);
		for (size_t i = 0; i < nJobs_; i++) {

			rangeStart = nIndividuals_ / nJobs_ * i;
			if (i == nJobs_ - 1)
				rangeEnd = nIndividuals_ - 1;
			else
				rangeEnd = rangeStart + nIndividuals_ / nJobs_ - 1;

			if (i >= nConstructedJobs)
				jobs_.emplace_back(rangeStart, rangeEnd, hostFitness_, hostIndices, evaluationFunction_); // Avoid unnecesary copy from push_back().
			else
				jobs_[i].setAttributes(rangeStart, rangeEnd, hostFitness_, hostIndices, evaluationFunction_); // Reuse 'geneticJob' objects to avoid dynamic memory overhead.
			threadPool_->submitJob(&jobs_[i]);

		}

		VoxelEng::logger::debugLog("Waiting for all jobs to end");
		threadPool_->awaitNoJobs();
		VoxelEng::logger::debugLog("All jobs ended");


		// DEBUG.
		VoxelEng::logger::debugLog("Update fitness for population:");
		for (size_t i = 0; i <= nIndividuals_; i++)
			VoxelEng::logger::debugLog("Individual " + std::to_string(i) + ": " +  std::to_string(*(hostFitness_ + *(hostIndices + i))));


		// Free copied memory from device (GPU) to host (CPU).
		delete hostIndices;
		hostIndices = nullptr;

	}

	AI::GeneticNeuralNetwork& genetic::individual(unsigned int individualID) {

		if (individualID < individuals_.size())
			return individuals_[individualID];
		else
			VoxelEng::logger::errorLog("The specified individual ID is out of bounds");

	}

	void genetic::selectionOperator(unsigned int implementation) {
	
		// Clean up the selected_ array  from an earlier execution of this method
		// because it stores this method's output instead of creating an af::array
		// each time it is called.
		selected_ = af::constant(0, nIndividuals_);

		if (implementation == 0) { // Roulette-wheel.
		
			// Get the sum of all fitness functions.
			af::array popFitness = fitness_(*popInds_),
					  fitnessSum = af::sum(popFitness, 0);

			VoxelEng::logger::debugLog("fitnessSum's nElements: " + fitnessSum.elements());
			VoxelEng::logger::debugLog("Is fitnessSum's scalar: " + fitnessSum.isscalar());
			VoxelEng::logger::debugLog("fitnessMax's value: " + fitnessSum.scalar<unsigned>());

			// Calculate the cumulative sum of probability of being selected for each individual.
			af::array prob = af::accum(popFitness / fitnessSum);

			// Select the parents using said random numbers.
			af::array rand = af::randu(nIndividuals_);
			selected_(af::where(rand <= prob(0))) = 0;
			gfor(af::seq i, 1, nIndividuals_-1)
				selected_(af::where(rand <= prob(i) && rand > prob(i.operator-(1)))) = i;

		}
		else
			VoxelEng::logger::errorLog("The specified implementation for the selection operator does not exist");

	}

	void genetic::crossoverOperator(unsigned int implementation) {

		if (implementation == 0) { // 1-point crossover.

			selected_.host(hostSelected_);
			popInds_->host(hostPopInds_);
			newbornInds_->host(hostNewbornInds_);

			for(int i = 0; i < nIndividuals_; i+=2)
			{

				std::vector<af::array>& newborn1Weights = individuals_[hostPopInds_[hostSelected_[i]]].weights(),
										newborn2Weights = individuals_[hostPopInds_[hostSelected_[i+1]]].weights(),
										parent1Weights = individuals_[hostNewbornInds_[i]].weights(),
										parent2Weights = individuals_[hostNewbornInds_[i+1]].weights();
				for (int i = crossoverSplitPoint_; i >= 0; i--) {

					newborn1Weights[i] = parent2Weights[i];
					newborn2Weights[i] = parent1Weights[i];

				}

				for (int i = crossoverSplitPoint_ + 1; i < nLayers_; i++) {

					newborn1Weights[i] = parent1Weights[i];
					newborn2Weights[i] = parent2Weights[i];

				}

			}


			// Free copied memory from device (GPU) to host (CPU).
			delete hostSelected_;
			hostSelected_ = nullptr;
			delete hostPopInds_;
			hostPopInds_ = nullptr;
			delete hostNewbornInds_;
			hostNewbornInds_ = nullptr;
			
		}
		else
			VoxelEng::logger::errorLog("The specified implementation for the crossover operator does not exist");
	
	}

	void genetic::mutationOperator(unsigned int implementation) {
	
		if (implementation == 0) { // Modify slighly some random newborn's genes.

			newbornInds_->host(hostNewbornInds_);

			unsigned int sizeX = 0,
						 sizeY = 0;
			for (int i = 0; i < nIndividuals_; i++) {
			
				std::vector<af::array>& weights = individuals_[hostNewbornInds_[i]].weights();

				for (int j = 0; j < weights.size(); j++) {
				
					// Calculate genes unaffected by mutation.
					sizeX = weights[j].dims(0);
					sizeY = weights[j].dims(1);
					af::array variation = (mutationVariationMax_ - mutationVariationMin_) * af::randu(sizeX, sizeY) + mutationVariationMin_;

					// Apply mutation variation.
					weights[j] += variation * (af::randu(sizeX, sizeY) < mutationRate_);
				
				}
			
			}

			delete hostNewbornInds_;
			hostNewbornInds_ = nullptr;

		}
		else
			VoxelEng::logger::errorLog("The specified implementation for the mutation operator does not exist");
	
	}

	void genetic::replacementOperator(unsigned int implementation) {
	
		if (implementation == 0) // Generational.
		{

			af::array* aux = popInds_;
			popInds_ = newbornInds_;
			newbornInds_ = aux;

		}
		else if (implementation == 1) { // Elitist.
		
			// Calculate the fitness value for the newborn.
			calculateFitness(true);
			
			// Get the first 'nIndividuals' individuals among which the actual population and the newborn that have the highest fitness.
			fitness_.write<float>(hostFitness_, sizeof(float) * nIndividuals_*2); // Pass updated fitness data from the CPU to the GPU.
			af::array fitnessIndices;
			af::sort(fitness_, fitnessIndices, fitness_, 0, false);

			// Create the new generation by changing the indices in 'popInds_' and in 'newbornInds_'.
			*popInds_ = fitnessIndices(af::seq(nIndividuals_));
			*newbornInds_ = fitnessIndices(af::seq(nIndividuals_, nIndividuals_*2 - 1));
		
		}
		else
			VoxelEng::logger::errorLog("The specified implementation for the replacement operator does not exist");

	}

	genetic::~genetic() {
	
		threadPool_->shutdown();
		threadPool_->awaitTermination();
		delete threadPool_;

		if (popInds_)
			delete popInds_;

		if (newbornInds_)
			delete newbornInds_;

		if (hostFitness_)
			delete[] hostFitness_;

		if (hostSelected_)
			delete[] hostSelected_;

		if (hostPopInds_)
			delete[] hostPopInds_;

		if (hostNewbornInds_)
			delete[] hostNewbornInds_;
	
	}

}