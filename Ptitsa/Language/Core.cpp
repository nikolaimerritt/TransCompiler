#include <vector>
#include <cmath>

#include "Core.h"
#include "Object.h"
#include "Core.h"
#include "..\Compiler\Mistake.h"

bool Library::isTrue(bool b) { return b; }
bool Library::isTrue(BuiltinType::Object const& object)
{
	if (object.type == Object::BOOLEAN) return object.boolean;
	throw Mistake::Wrong_Type_Used("Cannot check whether a " + object.typeAsString() + " is true or false.");
}

BuiltinType::Object Library::exp(BuiltinType::Object const& power)
{

	if (power.type == Object::NUMBER) return Object(pow(2.71828182845904523536, power.number));
	throw Mistake::Wrong_Type_Used("Could not run 'exp' on a " + power.typeAsString());
}
