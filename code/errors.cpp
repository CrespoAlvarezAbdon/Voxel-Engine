#include "errors.hpp"

error::error(errorTypes error_type, const string& error_log)
	: errorType_(error_type), errorLog_(error_log)
{}