#ifndef _VOXELENG_LOGGER_
#define _VOXELENG_LOGGER_
#include <string>
#include <stdexcept>


namespace VoxelEng {

	////////////
	//Classes.//
	////////////


	// 'logger' class.

	class logger {

	public:

		// Other methods.

		static void say(const std::string& msg);

		static void log(const std::string& msg);

		static void debugLog(const std::string& msg);

		static void warningLog(const std::string& msg);

		static void errorLog(const std::string& msg);

		static void errorOutOfRange(const std::string& msg);

	private:



	};

}

#endif