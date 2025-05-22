/**
* @file threadPool.h
* @version 1.0
* @date 25/04/2023
* @author Abdon Crespo Alvarez
* @title Thread pool.
* @brief Contains the declaration of the 'threadPool' class.
*/
#ifndef _VOXELENG_THREADPOOL_
#define _VOXELENG_THREADPOOL_

#include <atomic>
#include <condition_variable>
#include <deque>
#include <thread>
#include <vector>
#include <functional>
#include <mutex>
#include <atomicRecyclingPool.h>

#if GRAPHICS_API == OPENGL

#include <glm.hpp>

#endif


namespace VoxelEng {

	typedef std::function<void(void*)> task;

	/**
	* @brief Jobs are computational steps that solve some small tasks that are to be processed
	* in a parallel fashion in order to divide the workload of completing a heavy
	* task in terms of computational cost between many threads. That is, said heavy task is divided
	* into smaller tasks that can be completed in parallel.
	*/
	class job {

	public:

		/**
		* @brief Set the task to be process along with the data needed for it (optional).
		*/
		void setTask(const task& task, void* data = nullptr, atomicRecyclingPool<job>* jobPool = nullptr);

		/**
		* @brief Process the given task.
		*/
		void process();

	private:

		task task_;
		void* data_;
		atomicRecyclingPool<job>* jobPool_;

	};

	inline void job::setTask(const task& task, void* data, atomicRecyclingPool<job>* jobPool) {
	
		task_ = task;
		data_ = data;
		jobPool_ = jobPool;
	
	}

	inline void job::process() {

		task_(data_);

		if (jobPool_)
			jobPool_->free(*this);

	}


	/**
	* @brief A collection of threads that are commonly used to divide the workload
	* of one heavy task in terms of computational power into several smaller tasks that
	* can be completed in parallel by those threads.
	*/
	class threadPool {

	public:

		// Constructors.

		/**
		* @brief Class constructor.
		*/
		threadPool(unsigned int nThreads);


		// Observers.

		/**
		* @brief Returns the current number of jobs in the pool.
		*/
		unsigned int size();

		/**
		* @brief Returns true if the thread pool is terminated or false otherwise.
		*/
		bool terminated() const;


		// Modifiers.

		/**
		* @brief The caller thread will be blocked until a task to complete 
		* can be assigned to it. Then that thread will process said task and, if
		* there are no other tasks to do, wait until a new task is given or the corresponding threadPool
		* object is shutdown or terminated.
		*/
		void waitForTask();

		/**
		* @brief Submit a previously existing job to complete to the thread pool.
		* WARNING. Be sure to properly manage the heap memory assigned to the 'job' objects.
		* @param pushJobBack. Whether to insert the job at the back of the queue (true) or at the beginning (false).
		*/
		void submitJob(job* job, bool pushJobBack);

		/**
		* @brief The thread pool will no longer accept submitted jobs.
		*/
		void shutdown();

		/**
		* @brief Locks the caller thread until this 'threadpool' object has no jobs left to process.
		*/
		void awaitNoJobs();

		/**
		* @brief Locks the caller thread until the thread pool is terminated, that is, until
		* the thread pool is shutdown and all the threads of the pool have joined properly.
		* WARNING. Call shutdown() before this method. The resources allocated by the thread pool
		* are freed when it is terminated.
		*/
		void awaitTermination();


		// Destructors.

		/**
		* @brief Class destructor.
		*/
	    ~threadPool();

	private:

		unsigned int nThreads_;
		std::mutex jobsMutex_,
				   callerMutex_;
		std::condition_variable jobsAvailable_,
								callerCV_;
		std::atomic<bool> shutdown_,
					      terminated_;
		std::atomic<unsigned int> nWorkingThreads_;
		std::vector<std::thread> pool_;
		std::deque<job*> jobs_;
	
	};

	inline bool threadPool::terminated() const {

		return terminated_;

	}

}

#endif