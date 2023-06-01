#include "threadPool.h"
#include "logger.h"


namespace VoxelEng {

	threadPool::threadPool(unsigned int nThreads)
	: nThreads_(nThreads), shutdown_(false), terminated_(false), nWorkingThreads_(0) {

		for (unsigned int i = 0; i < nThreads; i++)
			pool_.push_back(std::thread([this]() {this->waitForTask(); }));

	}

	void threadPool::waitForTask() {

		job* job;
		while (true) {

			{

				std::unique_lock<std::mutex> lock(jobsMutex_);
				// If there are no more jobs, notify callers that are waiting on the awaitNoJobs() method.
				{

					std::unique_lock<std::mutex> lock(callerMutex_);

					if (jobs_.empty() && !nWorkingThreads_)
						callerCV_.notify_all();

				}

				while (jobs_.empty() && !shutdown_) // Wait until there is a job to do (if we should wait for one).
					jobsAvailable_.wait(lock); 


				// If new jobs are not going to we added to the queue (that is, shutdown() was called), 
				// then finish this thread's execution
				if (shutdown_ && jobs_.empty())
					return;

				job = jobs_.front();
				jobs_.pop_front();

			}

			nWorkingThreads_++;
			job->process();
			nWorkingThreads_--;

		}

	}

	void threadPool::submitJob(job* job) {

		if (shutdown_)
			VoxelEng::logger::errorLog("Cannot submit new jobs to a thread pool if shutdown() was called previously for that pool");

		{

			std::unique_lock<std::mutex> lock(jobsMutex_);
			jobs_.push_back(job);

		}

		jobsAvailable_.notify_one();

	}

	/*
	Orders the thread pool to not to accept any new jobs submitted.
	The rest of the jobs that remain unfinished in the jobs queue will
	continue to get processed.
	*/
	void threadPool::shutdown() {

		shutdown_ = true;
		jobsAvailable_.notify_all();

	}

	void threadPool::awaitNoJobs() {

		{

			std::unique_lock<std::mutex> lock(callerMutex_);
			
			do {

				callerCV_.wait(lock);

			} while (!jobs_.empty() && nWorkingThreads_); // If 'jobs_' is already empty and there are no jobs currently being done, unblock.
				
		}

	}

	void threadPool::awaitTermination() {

		if (shutdown_) {

			{

				std::unique_lock<std::mutex> lock(jobsMutex_);
				while (!jobs_.empty() && nWorkingThreads_)
					jobsAvailable_.wait(lock);

			}

			for (std::thread& t : pool_)
				t.join();

			pool_.clear();
			terminated_ = true;

		}
		else
			VoxelEng::logger::errorLog("shutdown() was not called before this method");

	}

	/*
	If method terminated() returns false, that is, if shutdown() was not called or all the threads have not finished running, the destructor will call
	the methods shutdown() first and awaitTermination() last to ensure that all submitted tasks are finished and that all threads stop their execution
	properly.
	*/
	threadPool::~threadPool() {

		if (!terminated_) {

			shutdown();
			awaitTermination();

		}

	}

}