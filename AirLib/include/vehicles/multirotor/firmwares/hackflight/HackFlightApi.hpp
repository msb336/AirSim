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

#include "firmware/datatypes.hpp"

#include "firmware/hackflight.hpp"

// Boards
#include "firmware/boards/arduino.hpp"
#include "firmware/boards/bonadrone.hpp"
#include "firmware/boards/ladybug.hpp"
#include "firmware/boards/mock.hpp"
#include "firmware/boards/realboard.hpp"
#include "firmware/boards/sentral.hpp"
#include "firmware/boards/softquat.hpp"
#include "firmware/boards/superfly.hpp"
#include "firmware/boards/thingdev.hpp"

// Receivers
#include "firmware/receivers/cppm.hpp"
#include "firmware/receivers/dsmx.hpp"
#include "firmware/receivers/esp8266.hpp"
#include "firmware/receivers/mock.hpp"
#include "firmware/receivers/sbus.hpp"

//Mixers
#include "firmware/mixers/quadx.hpp"




namespace msr { namespace airlib {

class HackFlightApi : public MultirotorApiBase {

public:
	void configure(const AirSimSettings::HackFlightConnectionInfo& connection_info)
	{

		// TODO: fill in other possible options
		//if (connection_info.board_type == "mock") {
			hf::MockBoard board(5);
		//}

		//if (connection_info.receiver_type == "mock") {
			hf::Mock_Receiver receiver;
		//}
		hf::MixerQuadX mixer;

		hf::Rate rate(0, 0, 0, 0, 0, 0, 0, 0);
		try {
			hf_->init(&board, &receiver, &mixer, &rate);
		}
		catch (std::exception& ex) {
			is_ready_ = false;
			is_ready_message_ = Utils::stringf("Failed to connect: %s", ex.what());
		}
	}
	void initialize(const AirSimSettings::HackFlightConnectionInfo& connection_info, const SensorCollection* sensors, bool is_simulation)
	{
		sensors_ = sensors;
		is_simulation_mode_ = is_simulation;
		// Setup hackflight parameters
		configure(connection_info);
	}
	virtual const SensorCollection& getSensors() const override
	{
		return *sensors_;
	}
	virtual void reset() override
	{
		//** no public method to do this
	}
	virtual void update() override
	{
		hf_->update();
		//** send sensor updates
	}
	void updateState() const
	{
		hf_->update();
	}
	virtual bool isReady(std::string& message) const override
	{
		if (!is_ready_)
			message = is_ready_message_;
		return is_ready_;
	}
	virtual void getStatusMessages(std::vector<std::string>& messages) override
	{
		update();

		//clear param
		messages.clear();

		//**move messages from private vars to param

	}
	virtual Kinematics::State getKinematicsEstimated() const override
	{
		updateState();
		Kinematics::State state;
		//TODO: reduce code duplication in getPosition() etc methods?
			
		state.pose.position = Vector3r(hf_->getState().positionX, hf_->getState().positionY, hf_->getState().altitude);
		state.pose.orientation = VectorMath::toQuaternion(hf_->getState().eulerAngles[0], hf_->getState().eulerAngles[1], hf_->getState().eulerAngles[2]);
		state.twist.linear = Vector3r(hf_->getState().velocityForward, hf_->getState().velocityRightward, 0); //** no z velocity for hackflight?
		state.twist.angular = Vector3r(hf_->getState().angularVelocities[0], hf_->getState().angularVelocities[1], hf_->getState().angularVelocities[2]);
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
		return Vector3r(hf_->getState().positionX, hf_->getState().positionY, hf_->getState().altitude);
	}
	virtual Vector3r getVelocity() const override
	{
		return Vector3r(current_state_.velocityForward, current_state_.velocityRightward);
	}

	virtual Quaternionr getOrientation() const override
	{
		//** updateState();
		return  VectorMath::toQuaternion(hf_->getState().eulerAngles[0], hf_->getState().eulerAngles[1], hf_->getState().eulerAngles[2]);
	}

	//virtual LandedState getLandedState() const override
	//{

	//	//** not sure this exists in hackflight
	//	// update();
	//	//return current_state_.controls.landed ? LandedState::Landed : LandedState::Flying;
	//}

	virtual real_T getActuation(unsigned int rotor_index) const override
	{
		//** no method for this inside of hackflight. maybe add to individual board controls?
		if (!is_simulation_mode_)
			throw std::logic_error("Attempt to read motor controls while not in simulation mode");
	}

	virtual bool armDisarm(bool arm) override
	{
		// // This is a protected method. How else to arm/Disarm the drone?
		//hf_->handle_SET_ARMED_Request(arm);
	}

	//virtual bool takeoff(float timeout_sec) override
	//{
	//	SingleCall lock(this);
	//	auto vec = getPosition();
	//	float z = vec.z() + getTakeoffZ();

	//}




	void updateState() const
	{
		hf_->update();
		state_t current_state = hf_->getState();
	}


protected: //methods
	//** neeed to build in commands to match MultirotorApiBase virtual functions
	virtual void commandRollPitchZ(float pitch, float roll, float z, float yaw) override
	{
		if (target_height_ != -z) {

			thrust_controller_.setPoint(-z, .05f, .005f, 0.09f);
			target_height_ = -z;
		}
		checkValidVehicle();
		auto state = mav_vehicle_->getVehicleState();
		float thrust = 0.21f + thrust_controller_.control(-state.local_est.pos.z);
		mav_vehicle_->moveByAttitude(roll, pitch, yaw, 0, 0, 0, thrust);
	}

private:
	const SensorCollection* sensors_;
	bool is_simulation_mode_;
	bool is_ready_;
	std::string is_ready_message_;
	std::shared_ptr<hf::Hackflight> hf_;
	bool is_api_control_enabled_;
	state_t current_state_;
	float target_height_;
};

}} //namespace
#endif 