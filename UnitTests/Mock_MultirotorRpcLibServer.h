// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include <vehicles/multirotor/api/MultirotorRpcLibServer.hpp>

namespace msr {
	namespace airlib {
		class MultirotorApiBase;
	}
}

class Mock_MultirotorRpcLibServer : public MultirotorRpcLibServer
{
public:
	Mock_MultirotorRpcLibServer(ApiProvider* api_provider, string server_address, uint16_t port = 41451) :
		MultirotorRpcLibServer(api_provider, server_address, port) {}

	MOCK_METHOD1(getVehicleApi, MultirotorApiBase *(std::string&));
};