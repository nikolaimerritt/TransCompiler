#include "Mistake.h"
#include <string>

namespace Mistake
{
	// BasicException
	BaiscException::BaiscException(const std::string& message) : message(message) {  }
	BaiscException::BaiscException(const std::string& message, unsigned lineNumber)
	{
		this->message = "Error on line " + std::to_string(lineNumber) + ":\n" + message;
	}
	const char* BaiscException::what() const throw() { return message.c_str(); }	
}



