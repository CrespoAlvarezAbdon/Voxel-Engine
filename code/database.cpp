#include "database.h"

#include "logger.h"


namespace VoxelEng {

	database::database(const std::string& filename)
	: db_(nullptr) {

		leveldb::Options options;
		options.create_if_missing = true;
		leveldb::Status status = leveldb::DB::Open(options, filename, &db_);

		if (!status.ok())
			logger::errorLog("Could not open database at " + filename);
	
	}

	std::string database::get(const std::string& key) const {
	
		std::string value;
		leveldb::Status status = db_->Get(leveldb::ReadOptions(), key, &value);
		return status.IsNotFound() ? "" : value;
	
	}

	std::string database::getAt(const std::string& key) const {

		std::string value;
		leveldb::Status status = db_->Get(leveldb::ReadOptions(), key, &value);
		if (status.IsNotFound())
			logger::errorLog("Key " + key + " was not found at the database");
		else
			return value;

	}

	bool database::exists(const std::string& key) const {

		std::string dummy;
		leveldb::Status status = db_->Get(leveldb::ReadOptions(), key, &dummy);
		return !status.IsNotFound();

	}

	void database::insertAt(const std::string& key, const std::string& value) {

		if (exists(key))
			logger::errorLog("The key " + key + " already contains a value");
		else
			db_->Put(leveldb::WriteOptions(), key, value);

	}

	void database::erase(const std::string& key) {

		db_->Delete(leveldb::WriteOptions(), key);

	}

	void database::eraseAt(const std::string& key) {

		if (exists(key))
			db_->Delete(leveldb::WriteOptions(), key);
		else
			logger::errorLog("The key " + key + " contains no value");

	}

	database::~database() {
	
		delete db_;
		db_ = nullptr;
	
	}

}