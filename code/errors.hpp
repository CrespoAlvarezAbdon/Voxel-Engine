#ifndef _APP_ERRORS_
#define _APP_ERRORS_

#include <string>
using namespace std;

///////////
//Classes//
///////////

/*
Abstraction for a error report system that allows the programmer
to debug code easier.
*/
class error
{

public:

	// Error type enumeration.
	enum class errorTypes {GLEW_INIT_FAILED, NO_UNIFORM_FOUND};

	// Constructors.

	error(errorTypes error_type, const string& error_log = string());


	// Observers.

	const errorTypes type() const;
	const string& message() const;

private:

	errorTypes errorType_;
	string errorLog_;

};

inline const error::errorTypes error::type() const
{

	return errorType_;

}

inline const string& error::message() const
{

	return errorLog_;

}

#endif