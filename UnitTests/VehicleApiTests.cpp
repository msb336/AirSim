// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include "vehicles/multirotor/firmwares/mavlink/ArduCopterSoloApi.hpp"

class ArduCopterTest : public ::testing::Test
{
public:
	std::unique_ptr<ArduCopterSoloApi> SubjectUnderTest;
	ArduCopterTest() : SubjectUnderTest(std::make_unique<ArduCopterSoloApi>()) {}
};

TEST_F(ArduCopterTest, InitializeOpensConnections)
{
	EXPECT_TRUE(true);
}