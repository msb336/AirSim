// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include <vehicles/multirotor/api/MultirotorRpcLibClient.hpp>
#include "Mock_MultirotorRpcLibServer.h"
#include "Mock_MultirotorApiBase.h"
#include "Mock_ApiProvider.h"

class MultirotorRpcLibClientTest : public ::testing::Test
{
public:
	std::unique_ptr<MultirotorRpcLibClient> SubjectUnderTest;

	////Dependencies
	const std::unique_ptr<Mock_ApiProvider> MockApiProvider;
	const std::unique_ptr<Mock_MultirotorRpcLibServer> MockServer;
	const std::string ServerAddress;
	const int Port;

	MultirotorRpcLibClientTest() :
		SubjectUnderTest(std::make_unique<MultirotorRpcLibClient>()),
		MockApiProvider(std::make_unique<Mock_ApiProvider>(nullptr)),
		ServerAddress("123.123.123.0"),
		Port(555),
		MockServer(std::make_unique<Mock_MultirotorRpcLibServer>(MockApiProvider.get(), ServerAddress, Port))
	{ }
};

TEST_F(MultirotorRpcLibClientTest, TakeoffCommandReachesServer)
{
	float timeout = 10.f;
	std::string vehicleName = "test_vehicle_name";
	const auto MockApi = std::make_unique<Mock_MultirotorApiBase>();

	EXPECT_CALL(*MockApiProvider, getVehicleApi(vehicleName))
		.Times(1)
		.WillOnce(::testing::Return(MockApi.get()));

	EXPECT_CALL(*MockApi, takeoff(timeout))
		.Times(1)
		.WillOnce(::testing::Return(true));

	SubjectUnderTest->takeoffAsync(timeout, vehicleName);
}