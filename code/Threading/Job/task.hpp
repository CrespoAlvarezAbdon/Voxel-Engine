#ifndef _VOXELENG_THREADING_TASK_
#define _VOXELENG_THREADING_TASK_

#include <functional>

namespace VoxelEng {

	typedef std::function<void(void*)> func;

	/**
	* @brief Tasks are computational steps that solve some small operations that are to be processed
	* in a parallel fashion in order to divide the total workload between different threads.
	*/
	class task {

	public:

		// Constructors.
		task(func function, void* data);

		// Modifiers.

		void execute();

		func function_;
		void* data_;

	};

	inline task::task(func function, void* data) 
	:  function_(function), data_(data)
	{}

	inline void task::execute() {

		function_(data_);

	}

}

#endif