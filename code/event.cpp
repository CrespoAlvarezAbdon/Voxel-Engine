#include "event.h"

namespace VoxelEng {

	void event::notify() {
	
		for (auto it = listeners_.cbegin(); it != listeners_.end(); it++)
			(*it)->onEvent(this);
	
	}

	void event::attach(listener* l) {

		if (listeners_.contains(l))
			logger::errorLog("The specified listener is already attached to event " + name_);
		else
			listeners_.insert(l);

	}

	void event::detach(listener* l) {
	
		if (listeners_.contains(l))
			listeners_.erase(l);
		else
			logger::errorLog("The specified listener is not attached to event " + name_);
	
	}

}