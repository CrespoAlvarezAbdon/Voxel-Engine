#ifndef _THREADPOOL_
#define _THREADPOOL_

#include <thread>
#include <atomic>
#include <vector>
#include <deque>
#include <functional>
#include <glm.hpp>
#include <mutex>
#include <condition_variable>
using namespace std;


/*
A class that inherits from this one must define a void process() function, which represents the job to do associated to an object
of the said class. 
For example, if using the threadPool class, you could call threadPoll::submitJob(Job* job) and "job" could be the
address of an object whose class inherits from Job. Then, a thread inside that pool could execute the process() function associated to 
said object.
*/
class Job 
{

public:

	virtual void process() = 0;

private:

};

class threadPool 
{

public:

	threadPool(unsigned int nThreads);

	void waitForTask();
	void submitJob(Job* job);
	void shutdown();
	void awaitTermination();
	bool terminated() const noexcept;

	~threadPool();

private:

	unsigned int nThreads_;
	mutex jobsMutex_;
	condition_variable jobsAvailable_;
	atomic<bool> shutdown_,
		         terminated_;
	vector<thread> pool_;
	deque<Job*> jobs_;
	
};

inline bool threadPool::terminated() const noexcept
{

	return terminated_;

}



#endif