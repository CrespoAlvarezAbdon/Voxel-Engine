#ifndef _VOXELENG_DATABASE_
#define _VOXELENG_DATABASE_

#include <string>

// Possible values are: LEVELDB, SQLITE
#define VOXELENG_DATABASE_BACKEND LEVELDB

#if VOXELENG_DATABASE_BACKEND == LEVELDB

#include "leveldb/db.h"

#else

#if VOXELENG_DATABASE_BACKEND == SQLITE



#endif

#endif

namespace VoxelEng {

	/**
	* @brief Wrapper class for the database backend used by the engine in order to
	* store things such as worlds
	*/
	class database {

	public:

		// Constructor.

		database(const std::string& filename);


		// Observers.

		/**
		* @brief Returns the value with the specified key.
		* If no such value exists, expect undefined behaviour.
		*/
		std::string get(const std::string& key) const;

		/**
		* @brief Returns the value with the specified key.
		* If no such value exists, it throws an exception.
		*/
		std::string getAt(const std::string& key) const;

		/**
		* @brief Returns true if the specified key exists in the database
		* or false otherwise.
		*/
		bool exists(const std::string& key) const;


		// Modifiers.

		/**
		* @brief Inserts the key-value pair into the database.
		* If there is already a value with the specified key, its value is updated with the new one.
		*/
		void insert(const std::string& key, const std::string& value);

		/**
		* @brief Inserts the key-value pair into the database.
		* If there is already a value with the specified key, it throws an exception.
		*/
		void insertAt(const std::string& key, const std::string& value);

		/**
		* @brief Erase the specified key. 
		* If no such key is found at the database, it does nothing.
		*/
		void erase(const std::string& key);

		/**
		* @brief Erase the specified key.
		* If no such key is found at the database, it throws an exception.
		*/
		void eraseAt(const std::string& key);


		// Destructors.

		~database();

	private:

		#if VOXELENG_DATABASE_BACKEND == LEVELDB

			leveldb::DB* db_;

		#else

			#if VOXELENG_DATABASE_BACKEND == SQLITE



			#endif

		#endif

	};

}

#endif