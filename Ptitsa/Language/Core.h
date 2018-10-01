#ifndef CORE_INCLUDE
#define CORE_INCLUDE

#include <iostream>
#include <sstream>
#include "Object.h"

namespace Library
{
	using namespace BuiltinType;

	inline void innerPrint(std::ostream& stream) {  }
	template <typename T, typename... Rest> inline void innerPrint(std::ostream& stream, T const first, Rest const ...rest)
	{
		stream << first << ' ';
		innerPrint(stream, rest...);
	}
	template <typename T, typename... Rest> inline void show(T const first, Rest const ...rest)
	{
		std::stringbuf buffer;
		std::ostream stream(&buffer);
		innerPrint(stream, first, rest...);
		std::cout << buffer.str();
	}
	template <typename T, typename... Rest> inline void showLine(T const first, Rest const ...rest) { show(first, rest..., "\n"); }

	bool isTrue(const Object&);
	bool isTrue(bool);

	Object exp(const Object&);
}

#endif // !CORE_INCLUDE
