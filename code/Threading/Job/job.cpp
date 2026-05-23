#include "job.hpp"

#include <Threading/Job/atomicJobPool.hpp>

namespace VoxelEng {

	void job::process() {

		for (auto it = tasks_.begin(); it != tasks_.cend(); it++)
			it->execute();

		if (jobPool_)
			jobPool_->free(*this);

	}

}