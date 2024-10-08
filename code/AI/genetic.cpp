#include "genetic.h"
#include <ctime>
#include <cstddef>
#include <thread>
#include <initializer_list>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <af/algorithm.h>
#include <af/defines.h>
#include <timer.h>
#include <AIAPI.h>
#include <worldGen.h>
#include <chunk.h>
#include <utilities.h>
#include <Utilities/Logger/logger.h>
#include "AIGameEx1.h"


namespace AIExample {


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

		VoxelEng::logger::debugLog("Genetic job started");

		for (std::size_t i = rangeStart_; i <= rangeEnd_; i++)
			fitness_[indices_[i]] = evaluationFunction_(indices_[i]);
	
	}


	// 'copyJob' class.

	copyJob::copyJob(std::size_t rangeStart, std::size_t rangeEnd, const GeneticNeuralNetwork* parent,
					std::vector<GeneticNeuralNetwork>* individuals, unsigned int* newbornInds)
		: rangeStart_(rangeStart), rangeEnd_(rangeEnd), parent_(parent), individuals_(individuals), newbornInds_(newbornInds) {}

	void copyJob::setAttributes(std::size_t rangeStart, std::size_t rangeEnd, const GeneticNeuralNetwork* parent,
								std::vector<GeneticNeuralNetwork>* individuals, unsigned int* newbornInds) {

		rangeStart_ = rangeStart;
		rangeEnd_ = rangeEnd;
		parent_ = parent;
		individuals_ = individuals;
		newbornInds_ = newbornInds;

	}

	void copyJob::process() {

		VoxelEng::logger::debugLog("Copy job started");

		for (std::size_t i = rangeStart_; i <= rangeEnd_; i++)
			individuals_->operator[](newbornInds_[i]) = *parent_;

	}


	// 'genetic' class.

	genetic::genetic()
		: simInProgress_(false), saveIndsData_(false), crossoverSplitPoint_(0),
		nJobs_(0), mutationRate_(0.0f), mutationVariationMin_(0.0f),
		mutationVariationMax_(0.0f), popInds_(nullptr), newbornInds_(nullptr), hostFitness_(nullptr), hostSelected_(nullptr), 
		hostPopInds_(nullptr), hostNewbornInds_(nullptr), threadPool_(nullptr) {}

	void genetic::trainAgents(unsigned int nEpochs, unsigned int nEpochsPerSave, unsigned int nEpochsForNewWorld) {
	
		if (aiGame_ = dynamic_cast<AIExample::miningAIGame*>(VoxelEng::AIAPI::aiGame::selectedGame())) {

			simInProgress_ = true;
			unsigned int epochSaveCounter = 0, // Used to decide when to save individuals' data.
						 epochNewWorldCounter = nEpochsForNewWorld;
			std::string epochString;
			VoxelEng::timer t;
			for (unsigned int epoch = 0; epoch < nEpochs; epoch++) {

				/*
				Begin epoch.
				*/
				t.start();
				VoxelEng::worldGen::setSeed();
				VoxelEng::chunkManager::resetAIChunks();
				if (epochNewWorldCounter >= nEpochsForNewWorld) {

					aiGame_->generateAIWorld();
					epochNewWorldCounter = 0;

				}
				aiGame_->spawnAgents();

				calculateFitness(); // Calculate fitness of the actual population.

				selectionOperator(0);

				crossoverOperator(0);

				mutationOperator(0);

				replacementOperator(1);

				t.finish();
				epochString = std::to_string(epoch);
				VoxelEng::logger::debugLog("EPOCH " + epochString + " finished");
				VoxelEng::logger::debugLog("Time: " + std::to_string(t.getDurationMs()) + " ms");

				/*
				Epoch's end.
				*/

				epochSaveCounter++;
				if (epochSaveCounter == nEpochsPerSave && saveIndsData_) {

					saveIndividualsData("AIData/" + aiGame_->name() + '/' + aiGame_->name() + "_EPOCH_" + epochString);

					epochSaveCounter = 0;

				}				

				epochNewWorldCounter++;

			}

			// If 'nEpochsPerSave' is 0, the individuals' data will be saved once all the epochs have been processed.
			if (saveIndsData_ && nEpochsPerSave == 0)
				saveIndividualsData("AIData/" + aiGame_->name() + '/' + aiGame_->name() + "_FINAL_EPOCH");

			simInProgress_ = false;
		
		}
		else
			VoxelEng::logger::errorLog("The selected AI game is not a 'miningAIGame'");

	}

	void genetic::train(unsigned int nEpochs) {

		checkFlagsTrainOrTest();
		saveIndsData_ = false;
		trainAgents(nEpochs);

	}

	void genetic::train(unsigned int nEpochs, unsigned int nEpochsPerSave, unsigned int nEpochsForNewWorld) {

		checkFlagsTrainOrTest();
		saveIndsData_ = true;
		trainAgents(nEpochs, nEpochsPerSave, nEpochsForNewWorld);

	}

	void genetic::test(unsigned int nEpochs, float*& output, unsigned int& outputSize, unsigned int nEpochsForNewWorld) {
	
		checkFlagsTrainOrTest();

		if (!(aiGame_ = dynamic_cast<AIExample::miningAIGame*>(VoxelEng::AIAPI::aiGame::selectedGame())))
			VoxelEng::logger::errorLog("The selected AI game is not a 'miningAIGame'");


		simInProgress_ = true;
		unsigned int epochNewWorldCounter = nEpochsForNewWorld;
		std::string epochString;
		VoxelEng::timer t;
		VoxelEng::duration totalDuration = 0,
						   duration;
		af::array results = af::constant(0, nIndividuals_, af::dtype::f32);
		for (unsigned int epoch = 0; epoch < nEpochs; epoch++) {

			/*
			Epoch's beginning.
			*/
			t.start();

			VoxelEng::chunkManager::resetAIChunks();
			if (epochNewWorldCounter == nEpochsForNewWorld) {

				aiGame_->generateAIWorld();
				epochNewWorldCounter = 0;

			}
			aiGame_->spawnAgents();

			calculateFitness(); // Calculate fitness of the actual population.

			/*
			Epoch's end.
			*/
			t.finish();
			duration = t.getDurationMs();
			totalDuration += duration;
			epochString = std::to_string(epoch);

			VoxelEng::logger::debugLog("EPOCH " + epochString + " finished");
			VoxelEng::logger::debugLog("Time: " + std::to_string(duration) + " ms");

			VoxelEng::logger::debugLog("Results' size: " + std::to_string(results.elements()));
			results += fitness_(*popInds_);
			VoxelEng::logger::debugLog("Results' size now: " + std::to_string(results.elements()));

		}

		results /= nEpochs;
		output = results.host<float>();
		outputSize = results.elements();

		simInProgress_ = false;

	}

	void genetic::test(unsigned int nEpochs) {
	
		if (!(aiGame_ = dynamic_cast<AIExample::miningAIGame*>(VoxelEng::AIAPI::aiGame::selectedGame())))
			VoxelEng::logger::errorLog("The selected AI game is not a 'miningAIGame'");

		checkFlagsTrainOrTest();

		simInProgress_ = true;
		
		for (unsigned int i = 0; i < nEpochs; i++) {
		
			VoxelEng::chunkManager::resetAIChunks();

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

		simInProgress_ = false;

	}

	void genetic::record(const std::string& recordName) {
	
		if (!(aiGame_ = dynamic_cast<AIExample::miningAIGame*>(VoxelEng::AIAPI::aiGame::selectedGame())))
			VoxelEng::logger::errorLog("The selected AI game is not a 'miningAIGame'");

		checkFlagsTrainOrTest();

		simInProgress_ = true;

		VoxelEng::chunkManager::resetAIChunks();

		/*
		Epoch's beginning.
		*/

		aiGame_->generateAIWorld();
		aiGame_->spawnAgents();

		calculateFitness(); // Calculate fitness of the actual population.

		/*
		Epoch's end.
		*/

		// Save the record's world.
		//VoxelEng::chunkManager::saveAllChunks("saves/recordingWorlds/" + aiGame_->name() + "/" + recordName);

		simInProgress_ = false;

	
	}

	void genetic::saveIndividualsData(const std::string& path) {
	
		VoxelEng::timer t;
		t.start();

		std::string truePath = path + ".aidata",
			        saveData;
		std::ofstream saveFile(truePath);
		float* weightsData = nullptr;


		// Store the individuals' network layout.
		for (std::size_t i = 0; i < sizeLayer_.size(); i++)
			saveData += std::to_string(sizeLayer_[i]) + '|';
		saveData += '/';

		// Store individuals' data.
		hostPopInds_ = popInds_->host<unsigned int>();
		VoxelEng::logger::say("Saving AI data...");
		for (unsigned int i = 0; i < nIndividuals_; i++) { // For each individual (population or newborn).
		
			std::vector<af::array>* weights = individuals_[*(hostPopInds_ + i)].weights();

			for (unsigned int layer = 0; layer < weights->size(); layer++) { // Save for every individual's connection between layers.
			
				weightsData = weights->operator[](layer).host<float>();

				for (unsigned int w = 0; w < weights->operator[](layer).elements(); w++) { // Every connection's weight.
				
					saveData += std::to_string(*(weightsData + w));
					saveData += '|';
				
				}

				saveData += '#';

				af::freeHost(weightsData);
			
			}

			if (i != nIndividuals_ - 1)
				saveData += '@';
		
		}

		saveFile << saveData;

		af::freeHost(hostPopInds_);
		hostPopInds_ = nullptr;

		t.finish();

		VoxelEng::logger::say("Finished saving AI data. TIME: " + std::to_string(t.getDurationMs()));
	
	}

	int genetic::loadIndividualsData(const std::string& path) {
	
		VoxelEng::timer t;
		t.start();
		std::string truePath = path + ".aidata";

		if (std::filesystem::exists(truePath)) {

			std::ifstream saveFile(truePath);
			std::string word;
			float number = 0.0f;
			char character;
			unsigned int nIndividualsToLoad = 0;


			VoxelEng::logger::say("Type the number of individuals to load from the file or 0 to load them all.");
			while (!VoxelEng::validatedCinInput<unsigned int>(nIndividualsToLoad))
				VoxelEng::logger::say("Invalid number. Try again.");


			// Clean up data structures when necessary.
			individuals_.clear();
			sizeLayer_.clear();

			// Create the data for the first read individual.
			individuals_.emplace_back();
			std::vector<af::array>* weights = individuals_[0].weights();

			// Read the individuals' data.
			std::vector<float> hostLayer;
			bool readingLayout = true,
			     continueReading = true;
			unsigned int layer = 0,
						 nConnections = 0;
			GeneticNeuralNetwork* previousInd = nullptr;
			while (saveFile.get(character) && continueReading) {

				switch (character) {

					case '/': // Finished reading the individuals' network layout.
						readingLayout = false;
						nConnections = sizeLayer_.size();
						break;

					case '@': // New individual.

						// Reshape the previous read individual to match the read network's layout.
						previousInd = &individuals_.back();
						for (unsigned int i = 0; i < nConnections - 1; i++)
							previousInd->weights()->operator[](i) = af::moddims(previousInd->weights()->operator[](i), sizeLayer_[i] + 1, sizeLayer_[i + 1]);
						previousInd->updateNLayers();

						if (nIndividualsToLoad && individuals_.size() >= nIndividualsToLoad)
							continueReading = false;
						else {
						
							// Create data for the new read individual.
							individuals_.emplace_back();
							weights = individuals_.back().weights();
							layer = 0;
						
						}
						break;

					case '#': // New individual's layer.

						weights->emplace_back(hostLayer.size(), af::dtype::f32);
						weights->operator[](layer).write(hostLayer.data(), hostLayer.size() * sizeof(float));

						layer++;

						hostLayer.clear();

						break;

					case '|': // New weight.

						number = std::stof(word);
						word = "";

						if (readingLayout)
							sizeLayer_.push_back(number);
						else
							hostLayer.push_back(number);

						break;

					default: // Any part of a weight.

						word += character;

						break;

				}

			}

			// Reshape the last read individual to match the read network's layout if there is more than 1.
			if (individuals_.size() > 1) {
			
				previousInd = &individuals_.back();
				for (unsigned int i = 0; i < nConnections - 1; i++)
					previousInd->weights()->operator[](i) = af::moddims(previousInd->weights()->operator[](i), sizeLayer_[i] + 1, sizeLayer_[i + 1]);
				previousInd->updateNLayers();

			}
			
			// Adjust the other data structures that need first the individuals' data to be loaded.
			nIndividuals_ = individuals_.size();

			fitness_ = af::constant(0.0f, nIndividuals_ * 2, af::dtype::f32);
			selected_ = af::constant(0, nIndividuals_, af::dtype::u32);

			if (popInds_)
				delete popInds_;
			popInds_ = new af::array(nIndividuals_, af::dtype::u32); // Allocate space.
			*popInds_ = af::seq(nIndividuals_).operator af::array().as(af::dtype::u32); // Assign values by creating an af::seq and then converting it to an af::array.

			if (newbornInds_)
				delete newbornInds_;
			newbornInds_ = new af::array(nIndividuals_, af::dtype::u32);
			*newbornInds_ = af::seq(nIndividuals_).operator af::array().as(af::dtype::u32) + nIndividuals_;

			for (unsigned int i = nIndividuals_; i < nIndividuals_ * 2; i++) {
			
				individuals_.emplace_back();
				individuals_[i].initNetwork(sizeLayer_, 0.0f, 0.0f);

			}

			t.finish();
			VoxelEng::logger::debugLog("Finished loading AI data on " + std::to_string(t.getDurationMs()) + " ms");

			return nIndividuals_;

		}
		else
			return 0;

	}

	void genetic::genInitPop(unsigned int nIndividuals, float rangeMin, float rangeMax, bool training) {
	
		if (training && nIndividuals % 2 != 0)
			VoxelEng::logger::errorLog("The number of individuals must be divisible by 2.");

		unsigned int totalNIndividuals = (aiGame_->recording()) ? nIndividuals : nIndividuals * 2;
		nIndividuals_ = nIndividuals;
		individuals_ = std::vector<GeneticNeuralNetwork>(totalNIndividuals);
		fitness_ = af::constant(0.0f, totalNIndividuals, af::dtype::f32);
		selected_ = af::constant(0, nIndividuals_, af::dtype::u32);

		if (popInds_)
			delete popInds_;
		popInds_ = new af::array(nIndividuals_, af::dtype::u32); // Allocate space.
		*popInds_ = af::seq(nIndividuals_).operator af::array().as(af::dtype::u32); // Assign values by creating an af::seq and then converting it to an af::array.

		if (newbornInds_)
			delete newbornInds_;
		newbornInds_ = new af::array(nIndividuals_, af::dtype::u32);
		*newbornInds_ = af::seq(nIndividuals_).operator af::array().as(af::dtype::u32) + nIndividuals_;

		for (unsigned int i = 0; i < totalNIndividuals; i++)
			individuals_[i].initNetwork(sizeLayer_, rangeMin, rangeMax);
	
	}

	void genetic::setGame() {

		if (simInProgress_)
			VoxelEng::logger::errorLog("Cannot set AI game during a simulation.");
		else
			aiGame_ = dynamic_cast<miningAIGame*>(VoxelEng::AIAPI::aiGame::selectedGame());

	}

	void genetic::setNetworkTaxonomy(const std::initializer_list<unsigned int>& sizeLayer) {

		if (simInProgress_)
			VoxelEng::logger::errorLog("Cannot set network taxonomy during a simulation.");
		else
			sizeLayer_ = std::vector<unsigned int>(sizeLayer);

	}

	void genetic::setFitnessFunction(float (*fitnessFunction)(unsigned int individualID)) {
	
		if (simInProgress_)
			VoxelEng::logger::errorLog("Cannot change fitness function during a simulation.");
		else
			evaluationFunction_ = fitnessFunction;
	
	}

	void genetic::setCrossoverSplitPoint(unsigned int point) {

		if (simInProgress_)
			VoxelEng::logger::errorLog("Cannot change crossover split point during a simulation");
		else {

			if (point >= sizeLayer_.size() - 1)
				VoxelEng::logger::errorLog("The crossover split point was defined in a non-existent weight connection between layers");
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

	void genetic::setNThreads(unsigned int nJobs) {

		if (simInProgress_)
			VoxelEng::logger::errorLog("Cannot change the number of parallel threadpool jobs during a simulation");
		else {
		
			if (nJobs)
				nJobs_ = nJobs;
			else
				nJobs_ = std::thread::hardware_concurrency();
		
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

		// Arrayfire's gfor cannot be used since we have a std::vector of neural networks
		// because each neural network could be different in future implementations
		// an accessing an element in an af::array to index a std::vector breaks
		// the gfor GPU paralellism since the std::vector is allocated in CPU's main memory.
		// Therefore, to optimise the execution of the evaluation function on all the individuals,
		// CPU-threads will be used.

		unsigned int* hostIndices = nullptr;
		if (useNewborn)
			hostIndices = newbornInds_->host<unsigned int>();
		else
			hostIndices = popInds_->host<unsigned int>();


		// Manage the threadPool objects and the jobs to send to the it.
		if (nJobs_ > geneticJobs_.capacity())
			geneticJobs_.reserve(nJobs_);

		if (!threadPool_)
			threadPool_ = new VoxelEng::threadPool(nJobs_);

		// Send jobs to thread pool and wait until they are done.
		hostFitness_ = fitness_.host<float>(); // Do not free this as it is used later by other genetic operators.

		if (nIndividuals_ < nJobs_) {
		
			if (geneticJobs_.empty())
				geneticJobs_.emplace_back(0, nIndividuals_ - 1, hostFitness_, hostIndices, evaluationFunction_); // Avoid unnecesary copy from push_back().
			else
				geneticJobs_[0].setAttributes(0, nIndividuals_ - 1, hostFitness_, hostIndices, evaluationFunction_); // Reuse 'geneticJob' objects to avoid dynamic memory overhead.
			threadPool_->submitJob(&geneticJobs_[0]);
		
		}
		else {
		
			std::size_t rangeStart = 0,
				rangeEnd = 0,
				nConstructedJobs = geneticJobs_.size();
			for (std::size_t i = 0; i < nJobs_; i++) {

				rangeStart = nIndividuals_ / nJobs_ * i;
				rangeEnd = (i == nJobs_ - 1) ? nIndividuals_ - 1 : rangeStart + nIndividuals_ / nJobs_ - 1;

				if (i >= nConstructedJobs)
					geneticJobs_.emplace_back(rangeStart, rangeEnd, hostFitness_, hostIndices, evaluationFunction_); // Avoid unnecesary copy from push_back().
				else
					geneticJobs_[i].setAttributes(rangeStart, rangeEnd, hostFitness_, hostIndices, evaluationFunction_); // Reuse 'geneticJob' objects to avoid dynamic memory overhead.
				threadPool_->submitJob(&geneticJobs_[i]);

			}
		
		}

		VoxelEng::logger::debugLog("Waiting for all jobs to end");
		threadPool_->awaitNoJobs();
		VoxelEng::logger::debugLog("All jobs ended");


		// DEBUG.
		VoxelEng::logger::debugLog("Updated fitness for population:");
		for (std::size_t i = 0; i < nIndividuals_; i++)
			VoxelEng::logger::debugLog("Individual " + std::to_string(*(hostIndices + i)) + ": " + std::to_string(*(hostFitness_ + *(hostIndices + i))));

		// Send results from CPU to GPU before deleting them in host memory.
		fitness_.write(hostFitness_, nIndividuals_ * 2 * sizeof(float));

		// Free copied memory from device (GPU) to host (CPU) that are no longer needed.
		af::freeHost(hostIndices);
		hostIndices = nullptr;
		af::freeHost(hostFitness_);
		hostFitness_ = nullptr;

	}

	GeneticNeuralNetwork& genetic::individual(unsigned int individualID) {

		if (individualID < individuals_.size())
			return individuals_[individualID];
		else
			VoxelEng::logger::errorLog("The specified individual ID is out of bounds");

	}

	void genetic::selectionOperator(unsigned int implementation) {

		if (implementation == 0) { // Roulette-wheel.
		
			// Clean up the selected_ array.
			selected_ = af::constant(0, nIndividuals_, af::dtype::u32);

			// Get the sum of all fitness functions.
			af::array popFitness = fitness_(*popInds_),
					  fitnessSum = af::sum(popFitness, 0);

			VoxelEng::logger::debugLog("fitnessSum: " + std::to_string(fitnessSum.as(af::dtype::f32).scalar<float>()));

			// Calculate the cumulative sum of probability of being selected for each individual.
			float* prob = af::accum(popFitness / fitnessSum.as(af::dtype::f32).scalar<float>()).host<float>();

			// Select the parents using said random numbers.
			af::array rand = af::randu(nIndividuals_, af::dtype::f32, aiGame_->AIrandEng());
			selected_(af::where(rand <= prob[0])) = 0;
			for (std::size_t i = 1; i <= nIndividuals_; i++)
				selected_(af::where(rand <= prob[i] && rand > prob[i - 1])) = i;

			af::freeHost(prob);

		}
		else if (implementation == 1) { // Select the fittest individual as the only father (only compatible with crossover operator implemenentation 1).
		
			// Get the fittest individual in the population.
			af::array fitnessIndices,
					  fitnessValues;
			af::sort(fitnessValues, fitnessIndices, fitness_(*popInds_), 0, false);

			// DEBUG.
			unsigned int* host = fitnessIndices.as(af::dtype::u32).host<unsigned int>();
			float* hostFit = fitnessValues.as(af::dtype::f32).host<float>();
			for (unsigned int i = 0; i < nIndividuals_ * 2; i++)
				VoxelEng::logger::debugLog("Possible parent" + std::to_string(host[i]) + " with fitness " + std::to_string(hostFit[i]));
			af::freeHost(host);
			af::freeHost(hostFit);
			VoxelEng::logger::debugLog("First index is " + std::to_string(fitnessIndices.as(af::dtype::u32).scalar<unsigned int>()));

			// Fill the selected_ data.
			selected_ = af::constant(fitnessIndices.as(af::dtype::u32).scalar<unsigned int>(), 1, af::dtype::u32);
		
		}
		else
			VoxelEng::logger::errorLog("The specified implementation for the selection operator does not exist");

	}

	void genetic::crossoverOperator(unsigned int implementation) {

		if (implementation == 0) { // 1-point crossover.

			if (selected_.elements() % 2 == 0) {

				hostSelected_ = selected_.host<unsigned int>();
				hostPopInds_ = popInds_->host<unsigned int>();
				hostNewbornInds_ = newbornInds_->host<unsigned int>();

				for (unsigned int i = 0; i < nIndividuals_; i += 2) {

					std::vector<af::array>* newborn1Weights = individuals_[hostNewbornInds_[i]].weights(),
						* newborn2Weights = individuals_[hostNewbornInds_[i + 1]].weights(),
						* parent1Weights = individuals_[hostPopInds_[hostSelected_[i]]].weights(),
						* parent2Weights = individuals_[hostPopInds_[hostSelected_[i + 1]]].weights();

					for (int j = crossoverSplitPoint_; j >= 0; j--) {

						newborn1Weights->operator[](j) = parent2Weights->operator[](j);
						newborn2Weights->operator[](j) = parent1Weights->operator[](j);

					}

					for (int j = crossoverSplitPoint_ + 1; j < newborn1Weights->size(); j++) {

						newborn1Weights->operator[](j) = parent1Weights->operator[](j);
						newborn2Weights->operator[](j) = parent2Weights->operator[](j);

					}

				}

				// Free copied memory from device (GPU) to host (CPU).
				af::freeHost(hostSelected_);
				hostSelected_ = nullptr;
				af::freeHost(hostPopInds_);
				hostPopInds_ = nullptr;
				af::freeHost(hostNewbornInds_);
				hostNewbornInds_ = nullptr;

			}
			else
				VoxelEng::logger::errorLog("Number of parents must be divisible by 2");

		}
		else if (implementation == 1) { // All newborn are copies of the fittest individual.
		
			// Get newborns' data structures indices and the fittest parent from the population.
			hostNewbornInds_ = newbornInds_->host<unsigned int>();
			const GeneticNeuralNetwork& parent = individuals_[0];


			// Manage the jobs to send to the thread pool.
			if (nJobs_ > copyJobs_.capacity())
				copyJobs_.reserve(nJobs_);

			if (!threadPool_)
				threadPool_ = new VoxelEng::threadPool(nJobs_);

			// Send jobs to thread pool and wait until they are done.
			std::size_t rangeStart = 0,
						rangeEnd = 0,
						nConstructedJobs = copyJobs_.size();
			for (std::size_t i = 0; i < nJobs_; i++) {

				rangeStart = nIndividuals_ / nJobs_ * i;
				rangeEnd = (i == nJobs_ - 1) ? nIndividuals_ - 1 : rangeStart + nIndividuals_ / nJobs_ - 1;

				if (i >= nConstructedJobs)
					copyJobs_.emplace_back(rangeStart, rangeEnd, &parent, &individuals_, hostNewbornInds_); // Avoid unnecesary copy from push_back().
				else
					copyJobs_[i].setAttributes(rangeStart, rangeEnd, &parent, &individuals_, hostNewbornInds_); // Reuse 'geneticJob' objects to avoid dynamic memory overhead.
				threadPool_->submitJob(&copyJobs_[i]);

			}

			VoxelEng::logger::debugLog("Waiting for all jobs to end");
			threadPool_->awaitNoJobs();
			VoxelEng::logger::debugLog("All jobs ended");

			// Free copied memory from device (GPU) to host (CPU).
			af::freeHost(hostNewbornInds_);
			hostNewbornInds_ = nullptr;
		
		}
		else
			VoxelEng::logger::errorLog("The specified implementation for the crossover operator does not exist");
	
	}

	void genetic::mutationOperator(unsigned int implementation) {
	
		if (implementation == 0) { // Modify slighly some random newborn's genes.

			hostNewbornInds_ = newbornInds_->host<unsigned int>();

			unsigned int sizeX = 0,
						 sizeY = 0;
			for (unsigned int i = 0; i < nIndividuals_; i++) {
			
				std::vector<af::array>* weights = individuals_[hostNewbornInds_[i]].weights();

				for (int j = 0; j < weights->size(); j++) {
				
					// Calculate genes unaffected by mutation.
					sizeX = weights->operator[](j).dims(0);
					sizeY = weights->operator[](j).dims(1);
					af::array variation = (mutationVariationMax_ - mutationVariationMin_) * af::randu(af::dim4(sizeX, sizeY), af::dtype::f32, aiGame_->AIrandEng()) + mutationVariationMin_;

					// Apply mutation variation.
					weights->operator[](j) += variation * (af::randu(af::dim4(sizeX, sizeY), af::dtype::f32, aiGame_->AIrandEng()) < mutationRate_);
				
				}
			
			}

			// Free copied memory from device (GPU) to host (CPU).
			af::freeHost(hostNewbornInds_);
			hostNewbornInds_ = nullptr;

		}
		else
			VoxelEng::logger::errorLog("The specified implementation for the mutation operator does not exist");
	
	}

	void genetic::replacementOperator(unsigned int implementation) {
	
		if (implementation == 0) { // Generational.

			af::array* aux = popInds_;
			popInds_ = newbornInds_;
			newbornInds_ = aux;

		}
		else if (implementation == 1) { // Elitist.
		
			// Calculate the fitness value for the newborn.
			calculateFitness(true);
			
			// Get the first 'nIndividuals' individuals among which the actual population and the newborn that have the highest fitness.
			af::array fitnessIndices;
			af::sort(fitness_, fitnessIndices, fitness_, 0, false);

			// DEBUG.
			unsigned int* host = fitnessIndices.as(af::dtype::u32).host<unsigned int>();
			float* hostFit = fitness_.host<float>();
			for (unsigned int i = 0; i < nIndividuals_ * 2; i++)
				VoxelEng::logger::debugLog("Possible parent " + std::to_string(host[i]) + " with fitness " + std::to_string(hostFit[i]));
			af::freeHost(host);
			
			// Create the new generation by changing the indices in 'popInds_' and in 'newbornInds_'.
			*popInds_ = fitnessIndices(af::seq(nIndividuals_));
			*newbornInds_ = fitnessIndices(af::seq(nIndividuals_, nIndividuals_*2 - 1));

			// DEBUG.
			host = popInds_->host<unsigned int>();
			for (unsigned int i = 0; i < nIndividuals_; i++)
				VoxelEng::logger::debugLog("New parent is " + std::to_string(host[i]) + " with previous fitness " + std::to_string(hostFit[i]));
			af::freeHost(host);
			af::freeHost(hostFit);
		
		}
		else
			VoxelEng::logger::errorLog("The specified implementation for the replacement operator does not exist");

	}

	genetic::~genetic() {
	
		if (threadPool_) {
		
			threadPool_->shutdown();
			threadPool_->awaitTermination();
			delete threadPool_;
			
		}
		
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