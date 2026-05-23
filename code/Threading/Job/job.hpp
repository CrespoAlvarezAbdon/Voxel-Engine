#ifndef _VOXELENG_THREADING_JOB_
#define _VOXELENG_THREADING_JOB_

#include <list>

#include <Threading/Job/task.hpp>

namespace VoxelEng {

	// Forward declarations.

	class atomicJobPool;


	// Classes.

	/**
	* @brief Collection of tasks to be executed each other in sequence.
	* A pool of jobs may execute the jobs in parallel between different threads. But a job's tasks are only processed in the same thread 
	* in a predefined sequence.
	*/
	class job {

	public:

		/**
		* @brief Push the task to be processed along with the data needed for it (optional) at the back of the list of tasks to process.
		* @param task The task to perform.
		* @param data Data associated with the task.
		* @param jobPool Pool of jobs this job is associated to.
		*/
		void pushBackTask(const task& task);

		/**
		* @brief Push the task to be processed along with the data needed for it (optional) at the back of the list of tasks to process.
		* @param task The task to perform.
		* @param data Data associated with the task.
		* @param jobPool Pool of jobs this job is associated to.
		*/
		void pushBackTask(func function, void* data_);

		/**
		* @brief Set the job pool associated with this job.
		* @param jobPool The job pool to associated. If null, the job is not associated with any job pool.
		*/
		void setPool(atomicJobPool* jobPool);

		/**
		* @brief Process the given job with all its associated tasks.
		*/
		void process();

	private:

		std::list<task> tasks_;
		atomicJobPool* jobPool_;

	};

	inline void job::pushBackTask(const task& task) {

		tasks_.push_back(task);

	}

	inline void job::pushBackTask(func function, void* data) {

		tasks_.emplace_back(function, data);

	}

	inline void job::setPool(atomicJobPool* jobPool) {

		jobPool_ = jobPool;

	}

}

#endif