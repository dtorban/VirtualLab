/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include "VirtualLab/DataValue.h"

using namespace vl;

int main(int argc, char**argv) {

	DataValue val = FloatDataValue(1.0);
	std::cout << val.get<float>(123) << std::endl;
	val = StringDataValue("abc");
	std::cout << val.get<float>(123) << std::endl;
	std::cout << val.get<std::string>("Hi") << std::endl;

	return 0;
}


