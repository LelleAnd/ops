/**
 *
 * OPS generated code, DO NOT MODIFY!
 */
#ifndef PizzaProject_PizzaProjectTypeFactory_h
#define PizzaProject_PizzaProjectTypeFactory_h

#include "SerializableFactory.h"
#include <string>

#include "CapricosaData.h"
#include "LHCData.h"
#include "ExtraAllt.h"
#include "Cheese.h"
#include "VessuvioData.h"
#include "PizzaData.h"
#include "OPSMessage.h"

namespace PizzaProject {


class PizzaProjectTypeFactory : public ops::SerializableFactory
{
public:
    ops::Serializable* create(CreateType_T type)
    {
		if (type == ("ops.protocol.OPSMessage"))
		{
			return new ops::OPSMessage();
		}
		if(type == "pizza.CapricosaData")
		{
			return new pizza::CapricosaData();
		}
		if(type == "pizza.special.LHCData")
		{
			return new pizza::special::LHCData();
		}
		if(type == "pizza.special.ExtraAllt")
		{
			return new pizza::special::ExtraAllt();
		}
		if(type == "pizza.special.Cheese")
		{
			return new pizza::special::Cheese();
		}
		if(type == "pizza.VessuvioData")
		{
			return new pizza::VessuvioData();
		}
		if(type == "pizza.PizzaData")
		{
			return new pizza::PizzaData();
		}
		return NULL;

    }
};

}

//end namespaces

#endif
