#ifndef _VOXELENG_THREADING_ATOMICJOBPOOL_
#define _VOXELENG_THREADING_ATOMICJOBPOOL_

#include <atomicRecyclingPool.h>
#include <Threading/Job/job.hpp>

namespace VoxelEng {

	/**
	* @brief Atomic pool of jobs that automatically associates any created job with itself.
	*/
	class atomicJobPool : public atomicRecyclingPool<job> {
	
	public:

		// Constructors.

		/**
		* @brief Class constructor.
		* @param nElements Number of elements to default construct and insert into the pool.
		*/
		atomicJobPool(unsigned int nElements = 0);

		// Modifiers.

		/**
		* @brief Get an element from the pool. If no one is present to be returned, one will be created.
		* @param An element from the pool.
		*/
		virtual const job& getConst();

		/**
		* @brief Get an element from the pool. If no one is present to be returned, one will be created.
		* @param An element from the pool.
		*/
		virtual job& get();

	private:


	
	};

	inline atomicJobPool::atomicJobPool(unsigned int nElements)
	: atomicRecyclingPool<job>(nElements) 
	{}

}

#endif