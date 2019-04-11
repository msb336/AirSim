// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include <api/ApiProvider.hpp>

namespace msr {
	namespace airlib {
		class VehicleApiBase;
	}
}

using namespace msr::airlib;

class Mock_ApiProvider : public ApiProvider
{
public:
	Mock_ApiProvider(WorldSimApiBase *world_sim_api) : ApiProvider(world_sim_api) {}

	MOCK_METHOD1(getVehicleApi, VehicleApiBase *(const std::string&));
};