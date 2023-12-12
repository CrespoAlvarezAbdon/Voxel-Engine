#ifndef _VOXELENG_EVENT_
#define _VOXELENG_EVENT_

#include <unordered_set>
#include <string>
#include "listener.h"
#include "logger.h"

namespace VoxelEng {

	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class listener;


	////////////
	//Classes.//
	////////////

	/**
	* An objetified-ocurrence of an action during the engine's execution.
	*/
	class event {

	public:

		// Constructors.

		/**
		* @brief Class constructor.
		*/
		event(const std::string& name);


		// Observers.

		const std::string& name() const;


		// Modifiers.

		/**
		* @brief When the event has occurred, notify the listeners for
		* them to execute their methods. These methods execute on the same
		* thread that called this one.
		*/
		virtual void notify();

		// NOTE. IN CASE NOTIFYASYNC() WANTS TO BE ADDED. ADD FIRST A DYNAMIC
		// VERSION OF THE THREADPOOL CLASS THAT SUPPORTS CREATING
		// MORE THREADS ON THE FLY AS THE DEMAND REQUIRES IT.
		// ALSO THE SAFE THREAD VERSION OF AN EVENT NOTIFYING SOMETHING WITH ADDITIONAL
		// DATA SHOULD STORE EACH OF THESE DATA PER EACH THREAD THAT EXECUTES EACH CALL TO NOTIFY.

		/**
		* @brief Attach a listener to this event if it is not already attached.
		*/
		void attach(listener* l);

		/**
		* @brief Attach a listener to this event if it is not already attached.
		*/
		void attach(listener& l);

		/**
		* @brief Detach a listener that has been previously attached to this event.
		*/
		void detach(listener* l);

		/**
		* @brief Detach a listener that has been previously attached to this event.
		*/
		void detach(listener& l);


		// Destructor.

		virtual ~event() {};

	protected:

		std::unordered_set<listener*> listeners_;

	private:

		std::string name_;

	};

	inline void event::attach(listener& l) {
	
		attach(&l);
	
	}

	inline void event::detach(listener& l) {

		detach(&l);

	}

	inline event::event(const std::string& name) 
	: name_(name)
	{}

	inline const std::string& event::name() const {
	
		return name_;
	
	}

}

#endif