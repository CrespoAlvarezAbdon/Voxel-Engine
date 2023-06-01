/**
* @file logger.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Logger.
* @brief Contains the logger class, used to monitor useful information and report
* errors related to the engine's execution.
*/
#ifndef _VOXELENG_LOGGER_
#define _VOXELENG_LOGGER_
#include <string>
#include <stdexcept>


namespace VoxelEng {

	////////////
	//Classes.//
	////////////


	/**
	* @brief Utility class to log information and report execution
	* errors to the standard output.
	*/
	class logger {

	public:

		// Other methods.

		/**
		* @brief Directly print 'msg' into the standard output.
		*/
		static void say(const std::string& msg);

		/**
		* @brief Print "[log]: " + 'msg' into the standard output.
		*/
		static void log(const std::string& msg);

		/**
		* @brief Print "[DEBUG]: " + 'msg' into the standard output.
		*/
		static void debugLog(const std::string& msg);

		/**
		* @brief Print "[WARNING]: " + 'msg' into the standard output.
		*/
		static void warningLog(const std::string& msg);

		/**
		* @brief Print "[ERROR]:" + 'msg' into the standard output and throw
		* a std::runtime_exception with 'msg' as the error message. Try to save
		* the loaded level into a special file at the root directory named 
		* chunkManager::openedTerrainFileName() + "ERRORED" if possible.
		*/
		static void errorLog(const std::string& msg);

		/**
		* @brief Print "[ERROR]:" + 'msg' into the standard output and throw
		* a std::out_of_range with 'msg' as the error message. Try to save
		* the loaded level into a special file at the root directory named
		* chunkManager::openedTerrainFileName() + "ERRORED" if possible.
		*/
		static void errorOutOfRange(const std::string& msg);

	private:



	};

}

#endif