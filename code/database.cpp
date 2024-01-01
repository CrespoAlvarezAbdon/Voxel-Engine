#include "database.h"

#include "logger.h"


namespace VoxelEng {

	database::database(const std::string& filename) {
	
		leveldb::DB* db = nullptr;
		leveldb::Options options;
		options.create_if_missing = true;
		leveldb::Status status = leveldb::DB::Open(options, filename, &db);

		if (status.ok()) {

			int value = 148134243;
			db->Put(leveldb::WriteOptions(), "key1", std::to_string(value));
			std::string readvalue = "ey";
			db->Get(leveldb::ReadOptions(), "key", &readvalue);

			delete db;
			db = nullptr;

		}
		else
			logger::errorLog("Could not open database at " + filename);
	
	}

}