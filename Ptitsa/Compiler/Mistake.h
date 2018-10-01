#ifndef MISTAKE_INCLUDE
#define MISTAKE_INCLUDE

#include <iostream>
#include <exception>

namespace Mistake
{
	class BaiscException : public std::exception
	{
	protected:
		std::string message;
	public:
		explicit BaiscException(const std::string&);
		explicit BaiscException(const std::string&, unsigned lineNumber);

		const char* what() const throw();
	};

	class Variable_Does_Not_Exist : public BaiscException { using BaiscException::BaiscException; }; // PRAISE STROUSTROUP FOR THIS MAGIC

	class Wrong_Type_Used :			public BaiscException { using BaiscException::BaiscException; };

	class Bad_Variable_Name :		public BaiscException { using BaiscException::BaiscException; };

	class Division_By_Zero :		public BaiscException { using BaiscException::BaiscException; };

	class Item_Not_In_List :		public BaiscException { using BaiscException::BaiscException; };

	class Bad_Number_Used :			public BaiscException { using BaiscException::BaiscException; };

	class File_Does_Not_Exist:		public BaiscException { using BaiscException::BaiscException; };

	class Could_Not_Convert:		public BaiscException { using BaiscException::BaiscException; };
}

#endif

