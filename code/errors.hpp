#ifndef _APP_ERRORS_
#define _APP_ERRORS_
#include <string>
using namespace std;

class Error
{

public:

	enum class Error_types {GLEW_INIT_FAILED, NO_UNIFORM_FOUND};

	// Constructors
	Error(Error_types error_type, const string& error_log = string()) noexcept : error_type_(error_type), error_log_(error_log) {}
	Error(const Error& e) = default;
	Error(Error && e) = default;

	// Operators
	Error& operator =(const Error& e) = delete;
	Error& operator =(Error&& e) = delete;

	// Non-modifying methods
	const Error_types type() const noexcept;
	const string& error_log() const noexcept;

private:

	Error_types error_type_;
	string error_log_;

};

inline const Error::Error_types Error::type() const noexcept
{

	return error_type_;

}

inline const string& Error::error_log() const noexcept
{

	return error_log_;

}

#endif