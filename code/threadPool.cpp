#include "threadPool.h"

#include <iostream>
#include <ostream>
using namespace std;

threadPool::threadPool(unsigned int nThreads)
	: nThreads_(nThreads), shutdown_(false), terminated_(false)
{

	for (unsigned int i = 0; i < nThreads; i++)
		pool_.push_back(thread([this]() {this->waitForTask(); }));

}

void threadPool::waitForTask()
{

	Job* job;

	while (true)
	{

		{

			// Wait until there is a job to do
			unique_lock<mutex> lock(jobsMutex_);
			jobsAvailable_.wait(lock, [this] {return !jobs_.empty() || shutdown_; });
			
			// If new jobs are not going to we added to the queue (that is, shutdown() was called), 
			// then finish this thread's execution
			if (shutdown_ && jobs_.empty())
				return;

			job = jobs_.front();
			jobs_.pop_front();

		}

		job->process();

	}


}

void threadPool::submitJob(Job* job)
{

	if (shutdown_)
		throw runtime_error("[ERROR]: Cannot submit new jobs to"
			" a thread pool if shutdown() was called previously for"
			" that pool");

	{

		unique_lock<mutex> lock(jobsMutex_);
		jobs_.push_back(job);

	}

	jobsAvailable_.notify_one();

}

/*
Orders the thread pool to not to accept any new jobs submitted.
The rest of the jobs that remain unfinished in the jobs queue will
continue to get processed.
*/
void threadPool::shutdown()
{

	shutdown_ = true;
	jobsAvailable_.notify_all();

}

void threadPool::awaitTermination()
{

	if (shutdown_)
	{

		{

			unique_lock<mutex> lock(jobsMutex_);
			jobsAvailable_.wait(lock, [this] {return jobs_.empty(); });

		}

		for (thread& t : pool_)
			t.join();

		pool_.clear();
		terminated_ = true;

	}

}

/*
If method terminated() returns false, that is, if shutdown() was not called or all the threads have not finished running, the destructor will call
the methods shutdown() first and awaitTermination() last to ensure that all submitted tasks are finished and that all threads stop their execution
properly.
*/
threadPool::~threadPool()
{

	if (!terminated_)
	{

		shutdown();
		awaitTermination();

	}

}