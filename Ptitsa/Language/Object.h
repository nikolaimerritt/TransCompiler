#ifndef OBJECT_INCLUDE
#define OBJECT_INCLUDE

#include <string>
#include <vector>
#include <iostream>

namespace BuiltinType
{
	struct Object
	{
		enum ObjectType { NOTHING = 0, NUMBER, PHRASE, BOOLEAN, LIST } type;

		Object();
		Object(double);
		Object(std::string&&);
		Object(std::string const&);
		Object(bool);
		Object(std::vector<Object>&&);
		Object(std::vector<Object> const&);
		
		Object(const Object&);
		Object(Object&&);
		
		~Object();

		Object& operator=(Object);
		Object& operator=(double);
		Object& operator=(std::string);
		Object& operator=(bool);
		Object& operator=(std::vector<Object>); 		

		bool operator==(const Object&);

		union 
		{
			double number;
			std::string phrase;
			bool boolean;
			std::vector<Object> list;
		};

		void wipe();
		void initAndSwapWith(Object&);
		std::string typeAsString() const;
	};

	std::ostream& operator<<(std::ostream&, const Object&);	
	bool areEqual(const Object&, const Object&);
	
	bool operator==(Object const&, Object const&);
	bool operator!=(Object const&, Object const&);
	
	Object operator+(const Object&, const Object&);
	Object operator-(const Object&, const Object&);
	Object operator*(const Object&, const Object&);
	Object operator/(const Object&, const Object&);
	Object operator^(const Object&, const Object&);
}

#endif // !OBJECT_INCLUDE

