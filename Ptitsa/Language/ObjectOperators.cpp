#include <string>
#include <sstream>
#include <cmath>

#include "Object.h"
#include "../Compiler/Mistake.h"
#include "../Compiler/Util.h"

BuiltinType::Object& BuiltinType::Object::operator=(double d)
{
	wipe();
	new (&number) double(d);
	type = NUMBER;
	return *this;
}
BuiltinType::Object& BuiltinType::Object::operator=(std::string string)
{
	wipe();
	new (&phrase) std::string();
	std::swap(phrase, string);
	type = PHRASE;
	return *this;
}
BuiltinType::Object& BuiltinType::Object::operator=(bool b)
{
	wipe();
	new (&boolean) bool(b);
	type = BOOLEAN;
	return *this;
}
BuiltinType::Object& BuiltinType::Object::operator=(std::vector<Object> vector)
{
	wipe();
	new (&list) std::vector<Object>();
	std::swap(list, vector);
	type = LIST;
	return *this;
}
BuiltinType::Object& BuiltinType::Object::operator=(Object temp)
{
	wipe();
	initAndSwapWith(temp);
	return *this;
}

bool BuiltinType::Object::operator==(const Object& other) { return BuiltinType::areEqual(*this, other); }

bool BuiltinType::operator==(const Object& first, const Object& second) { return areEqual(first, second); }
bool BuiltinType::operator!=(Object const& first, Object const& second) { return !areEqual(first, second); }

BuiltinType::Object BuiltinType::operator+(const Object& first, const Object& second)
{
	if (first.type == Object::LIST)
	{
		std::vector<Object> allObjects = first.list;
		if (second.type == Object::LIST)
		{
			allObjects.insert(allObjects.end(), second.list.begin(), second.list.end());
		}
		else allObjects.push_back(second);

		return Object(allObjects);
	}

	else if (second.type == Object::LIST) return second + first;

	else if (first.type == Object::NUMBER && second.type == Object::NUMBER)  return Object(first.number + second.number);

	else if (first.type == Object::BOOLEAN && second.type == Object::BOOLEAN) return Object(first.boolean || second.boolean);

	else if (first.type == Object::PHRASE && second.type == Object::PHRASE) return Object(first.phrase + second.phrase);

	throw Mistake::Wrong_Type_Used("Could not add a " + first.typeAsString() + " and a " + second.typeAsString());
}

BuiltinType::Object BuiltinType::operator-(const Object& first, const Object& second)
{
	if (first.type == Object::LIST) // removing an element from first
	{
		std::vector<Object> newList = first.list;
		unsigned removeIdx;

		if (Util::couldSetIndex<Object>(newList, second, removeIdx))
		{
			newList.erase(newList.begin() + removeIdx);
			return Object(newList);
		}
		else if (second.type == Object::LIST) // removing a sequence of items from first
		{
			for (const Object& toRemove : second.list)
			{
				if (Util::couldSetIndex(newList, toRemove, removeIdx)) newList.erase(newList.begin() + removeIdx);
				else throw Mistake::Item_Not_In_List("Could not remove the items.");
			}
		}
		else throw Mistake::Item_Not_In_List("Could not remove the item.");
	}
	else if (first.type == Object::NUMBER && second.type == Object::NUMBER) return Object(first.number - second.number);

	throw Mistake::Wrong_Type_Used("Could not take away a " + first.typeAsString() + " from a " + second.typeAsString() + ".");
}

BuiltinType::Object BuiltinType::operator*(const Object& first, const Object& second)
{
	if (second.type == Object::NUMBER)
	{
		if (first.type == Object::NUMBER) return std::move(Object(first.number * second.number));
		else
		{
			unsigned const secondNumberAsNatural = static_cast<unsigned>(second.number);
			if (secondNumberAsNatural - second.number == 0 && second.number >= 0) // if second.number is a natural number
			{
				if (first.type == Object::PHRASE) // repeating a phrase n times
				{
					std::ostringstream stringStream;
					for (unsigned i = 0; i < secondNumberAsNatural; ++i) stringStream << first.phrase;
					return Object(stringStream.str());
				}
				else if (first.type == Object::LIST) // repeating a list n times
				{
					std::vector<Object> repeatedList;
					repeatedList.reserve(first.list.size() * secondNumberAsNatural);
					for (unsigned i = 0; i < secondNumberAsNatural; ++i) repeatedList.insert(repeatedList.end(), first.list.begin(), first.list.end());
					return Object(repeatedList);
				}
			}
			else
			{
				std::string const secondNumberAsString = std::to_string(second.number);
				throw Mistake::Bad_Number_Used("Could not repeat a phrase " + secondNumberAsString + " times by using '*', because " + secondNumberAsString + " is not a positive whole number.");
			}
			
		}		
	}
	else if (second.type == Object::NUMBER &&
			(first.type == Object::PHRASE || first.type == Object::LIST)) // if arguments are like above, but the other way round
	{
		return second * first;
	}
	throw Mistake::Wrong_Type_Used("Could not multiply a " + second.typeAsString() + " and a number.");

}

BuiltinType::Object BuiltinType::operator/(const Object& first, const Object& second)
{
	if (first.type == Object::NUMBER && second.type == Object::NUMBER) return Object(first.number / second.number);
	throw Mistake::Wrong_Type_Used("Could not divide a " + first.typeAsString() + " by a " + second.typeAsString());
}

BuiltinType::Object BuiltinType::operator^(const Object& first, const Object& second)
{
	if (first.type == Object::NUMBER && second.type == Object::NUMBER) return Object(pow(first.number, second.number));
	throw Mistake::Wrong_Type_Used("Could not raise a " + first.typeAsString() + " to the power of a " + second.typeAsString());
}

std::ostream& BuiltinType::operator<<(std::ostream& ostream, const Object& object)
{
	if (object.type == Object::NUMBER) ostream << object.number;
	else if (object.type == Object::PHRASE) ostream << object.phrase;
	else if (object.type == Object::BOOLEAN) ostream << (object.boolean ? "true" : "false");
	else if (object.type == Object::LIST)
	{
		ostream << '[';
		for (unsigned i = 0; i < object.list.size(); ++i)
		{
			ostream << object.list[i];
			if (i + 1 < object.list.size()) ostream << ", ";
		}
		ostream << ']';
	}
	else ostream << "Nothing";
	return ostream;
}