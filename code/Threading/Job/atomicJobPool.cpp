#include "atomicJobPool.hpp"

namespace VoxelEng {

	const job& atomicJobPool::getConst() {

		job& j = atomicRecyclingPool<job>::get();
		j.setPool(this);
		return j;

	}

	job& atomicJobPool::get() {

		job& j = atomicRecyclingPool<job>::get();
		j.setPool(this);
		return j;

	}

}