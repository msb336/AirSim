// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include <vehicles/multirotor/api/MultirotorApiBase.hpp>

namespace msr {
	namespace airlib {
		struct YawMode;
	}
}

using namespace msr::airlib;

class Mock_MultirotorApiBase : public MultirotorApiBase
{
protected:

	//VehicleApiBase functions.
	MOCK_METHOD1(enableApiControl, void(bool));
	MOCK_CONST_METHOD0(isApiControlEnabled, bool());
	MOCK_METHOD1(armDisarm, bool(bool));
	MOCK_CONST_METHOD0(getHomeGeoPoint, GeoPoint());

	/************************* low level move APIs *********************************/
	MOCK_METHOD4(commandRollPitchZ, void(float, float, float, float));
	MOCK_METHOD4(commandRollPitchThrottle, void(float, float, float, float));
	MOCK_METHOD4(commandVelocity, void(float, float, float, const YawMode&));
	MOCK_METHOD4(commandVelocityZ, void(float, float, float, const YawMode&));
	MOCK_METHOD4(commandPosition, void(float, float, float, const YawMode&));

	/************************* State APIs *********************************/
	MOCK_CONST_METHOD0(getKinematicsEstimated, Kinematics::State());
	MOCK_CONST_METHOD0(getLandedState, LandedState());
	MOCK_CONST_METHOD0(getGpsLocation, GeoPoint());
	MOCK_CONST_METHOD0(getMultirotorApiParams, MultirotorApiParams&());

	/************************* basic config APIs *********************************/
	MOCK_CONST_METHOD0(getCommandPeriod, float());
	MOCK_CONST_METHOD0(getTakeoffZ, float());
	MOCK_CONST_METHOD0(getDistanceAccuracy, float());

public:
	Mock_MultirotorApiBase() {}

	MOCK_METHOD1(takeoff, bool(float));
};