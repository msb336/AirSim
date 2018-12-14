// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef msr_airlib_HackFlightDroneController_hpp
#define msr_airlib_HackFlightDroneController_hpp

#include "common/Common.hpp"
#include "common/common_utils/Timer.hpp"
#include "common/CommonStructs.hpp"
#include "common/VectorMath.hpp"
#include "common/AirSimSettings.hpp"
#include "vehicles/multirotor/api/MultirotorApiBase.hpp"
#include "common/PidController.hpp"
#include "sensors/SensorCollection.hpp"

//sensors
#include "sensors/barometer/BarometerBase.hpp"
#include "sensors/imu/ImuBase.hpp"
#include "sensors/gps/GpsBase.hpp"
#include "sensors/magnetometer/MagnetometerBase.hpp"
#include "sensors/distance/DistanceBase.hpp"



#include "firmware/hackflight.hpp"
#include "firmware/boards/ladybug.hpp"
#include "firmware/receivers/sbus.hpp"
#include "firmware/mixers/quadx.hpp"
#include "firmware/pidcontrollers/level.hpp"




namespace msr { namespace airlib {

class HackFlightApi : public MultirotorApiBase {

public:
	void initialize(const AirSimSettings::HackFlightConnectionInfo& connection_info, const SensorCollection* sensors, bool is_simulation)
	{
		connection_info_ = connection_info;
		sensors_ = sensors;
		is_simulation_mode_ = is_simulation;

		try
		{
			//** initialize hackflight
			hf_.init(&board_, &receiver_, &mixer_, &rate_);
		}
		catch (std::exception& ex)
		{
			is_ready_ = false;
			is_ready_message_ = Utils::stringf("Failed to connect: %s", ex.what());
		}
	}
	virtual const SensorCollection& getSensors() const override
	{
		return *sensors_;
	}
	virtual void reset() override
	{
		board_.reboot()
	}
	virtual void update() override
	{
		hf_.update();
		//** send sensor updates
	}
	virtual bool isReady(std::string& message) const override
	{
		if (!is_ready_)
			message = is_ready_message_;
		return is_ready_;
	}
	virtual void getStatusMessages(std::vector<std::string>& messages) override
	{
		update()

		//clear param
		messages.clear();

		//**move messages from private vars to param

	}
	virtual Kinematics::State getKinematicsEstimated() const override
	{
		update()
		Kinematics::State state
		//TODO: reduce code duplication in getPosition() etc methods?
			
		state.pose.position = Vector3r(hf_.getState().positionX, hf_.getState().positionY, hf_.getState().altitude);
		state.pose.orientation = VectorMath::toQuaternion(hf_.getState().eulerAngles[0], hf_.getState().eulerAngles[1], hf_.getState().eulerAngles[2]);
		state.twist.linear = Vector3r(hf_.getState().velocityForward, hf_.getState().velocityRightward, 0); //** no z velocity for hackflight?
		state.twist.angular = Vector3r(hf_.getState().angularVelocities[0], hf_.getState().angularVelocities[1], hf_.getState().angularVelocities[2]);
		state.pose.position = Vector3r(0,0,0); //** No acceleration either?

		return state;
	}
	virtual bool isApiControlEnabled() const override
	{
		return is_api_control_enabled_;
	}
	virtual void enableApiControl(bool is_enabled) override
	{
		//** checkValidVehicle();
		if (is_enabled) {
			//** mav_vehicle_->requestControl(); //Is this necessary for hackflight?
			is_api_control_enabled_ = true;
		}
		else {
			//** mav_vehicle_->releaseControl();
			is_api_control_enabled_ = false;
		}
	}
	virtual Vector3r getPosition() const override
	{
		//** updateState();
		return Vector3r(current_state_.local_est.pos.x, current_state_.local_est.pos.y, current_state_.local_est.pos.z);
	}
	virtual Vector3r getVelocity() const override
	{
		//** updateState();
		return Vector3r(current_state_.local_est.lin_vel.x, current_state_.local_est.lin_vel.y, current_state_.local_est.lin_vel.z);
	}

	virtual Quaternionr getOrientation() const override
	{
		//** updateState();
		return VectorMath::toQuaternion(current_state_.attitude.pitch, current_state_.attitude.roll, current_state_.attitude.yaw);
	}

	virtual LandedState getLandedState() const override
	{
		//** updateState();
		return current_state_.controls.landed ? LandedState::Landed : LandedState::Flying;
	}

	//** virtual real_T getActuation







	void updateState() const
	{
		hf_.update();
		state_t current_state = hf_.getState();
		StatusLock lock(this);
		if (mav_vehicle_ != nullptr) {
			int version = mav_vehicle_->getVehicleStateVersion();
			if (version != state_version_)
			{
				current_state_ = mav_vehicle_->getVehicleState();
				state_version_ = version;
			}
		}
	}

private:
	const SensorCollection* sensors_;
	AirSimSettings::HackFlightConnectionInfo connection_info_;
	bool is_simulation_mode_;
	bool is_ready_;
	std::string is_ready_message_;
	hf::Hackflight hf_;
	hf::Board board_;
	hf::Receiver receiver_;
	hf::Mixer mixer_;
	hf::Raterate_;
	
};

}} //namespace
#endif 