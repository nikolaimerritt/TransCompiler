#include <string>
#include <iostream>
#include <vector>
#include <sstream>

#include "Object.h"
#include "../Compiler/Util.h"
#include "../Compiler/Mistake.h"

// ADD IN CONSTRUCTORS WITH RVALUES

using namespace BuiltinType;

Object::Object()
{
	new (&boolean) bool(true);
	type = NOTHING;
}
Object::Object(double d)
{
	new (&number) double(d);
	type = NUMBER;
}
Object::Object(std::string const& string)
{
	new (&phrase) std::string(string);
	type = PHRASE;
}
Object::Object(std::string&& string)
{
	new (&phrase) std::string();
	std::swap(phrase, string);
	type = PHRASE;
}
Object::Object(bool b)
{
	new (&boolean) bool(b);
	type = BOOLEAN;
}
Object::Object(std::vector<Object>&& vector)
{
	new (&list) std::vector<Object>();
	std::swap(list, vector);
	type = LIST;
}
Object::Object(std::vector<Object> const& vector)
{
	new (&list) std::vector<Object>(vector);
	type = LIST;
}
Object::Object(const Object& other)
{
	switch (other.type)
	{
		case NUMBER:	
			new (&number) double(other.number);			
			break;
		case PHRASE:	
			new (&phrase) std::string(other.phrase);	
			break;
		case BOOLEAN:
			new (&boolean) bool(other.boolean);
			break;
		case LIST:
			new (&list) std::vector<Object>(other.list);
			break;
	}
	type = other.type;
}
Object::Object(Object&& temp)
{
	initAndSwapWith(temp);	
}

Object::~Object()
{
	if (type == PHRASE) phrase.~basic_string();
	else if (type == LIST) list.~vector();
}

void Object::wipe()
{
	if (type == PHRASE) phrase.~basic_string();
	else if (type == LIST) list.~vector();
}

void Object::initAndSwapWith(Object& other)
{
	switch (other.type)
	{
		case PHRASE:	
			phrase = "";
			std::swap(phrase, other.phrase);
			break;
		case NUMBER:
			number = 0;
			std::swap(number, other.number);
			break;
		case BOOLEAN:
			boolean = false;
			std::swap(boolean, other.boolean);
			break;
		case LIST:
			list = std::vector<Object>();
			std::swap(list, other.list);
			break;
	}
	type = other.type;
	other.wipe();
}

std::string Object::typeAsString() const
{
	switch (type)
	{
		case NUMBER:
			return "Number";
		case PHRASE:
			return "Phrase";
		case BOOLEAN:
			return "Boolean";
		case LIST:
			return "List";
		case NOTHING:
			return "Nothing";
	}
	return "Unknown";
}



bool BuiltinType::areEqual(const Object& first, const Object& second)
{
	if (first.type == second.type)
	{
		switch (first.type)
		{
		case Object::NUMBER:
			return first.number == second.number;
		case Object::PHRASE:
			return first.phrase == second.phrase;
		case Object::BOOLEAN:
			return first.boolean == second.boolean;
		case Object::LIST:
			return first.list == second.list;
		case Object::NOTHING:
			return true;
		default:
			return false; // should never get here, but just in case!
		}
	}
	else return false;
}