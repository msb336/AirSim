// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef msr_airlib_AirSimSimpleFlightEstimator_hpp
#define msr_airlib_AirSimSimpleFlightEstimator_hpp

#include "firmware/interfaces/CommonStructs.hpp"
// #include "AirSimSimpleFlightCommon.hpp"
#include "physics/Kinematics.hpp"
#include "physics/Environment.hpp"
#include "common/Common.hpp"

//** LEFT OFF POINT
namespace msr { namespace airlib {


class AirSimHackFlightEstimator : public hf::IStateEstimator {
public:
    //for now we don't do any state estimation and use ground truth (i.e. assume perfect sensors)
    void setGroundTruthKinematics(const Kinematics::State* kinematics, const Environment* environment)
    {
        kinematics_ = kinematics;
        environment_ = environment;
    }

    virtual hf::Axis3r getAngles() const override
    {
        hf::Axis3r angles;
        VectorMath::toEulerianAngle(kinematics_->pose.orientation,
            angles.pitch(), angles.roll(), angles.yaw());

        //Utils::log(Utils::stringf("Ang Est:\t(%f, %f, %f)", angles.pitch(), angles.roll(), angles.yaw()));

        return angles;
    }

    virtual hf::Axis3r getAngularVelocity() const override
    {
        const auto& anguler = kinematics_->twist.angular;

        hf::Axis3r conv;
        conv.x() = anguler.x(); conv.y() = anguler.y(); conv.z() = anguler.z();

        return conv;
    }

    virtual hf::Axis3r getPosition() const override
    {
        return AirSimSimpleFlightCommon::toAxis3r(kinematics_->pose.position);
    }

    virtual hf::Axis3r transformToBodyFrame(const hf::Axis3r& world_frame_val) const override
    {
        const Vector3r& vec = AirSimSimpleFlightCommon::toVector3r(world_frame_val);
        const Vector3r& trans = VectorMath::transformToBodyFrame(vec, kinematics_->pose.orientation);
        return AirSimSimpleFlightCommon::toAxis3r(trans);
    }

    virtual hf::Axis3r getLinearVelocity() const override
    {
        return AirSimSimpleFlightCommon::toAxis3r(kinematics_->twist.linear);
    }

    virtual hf::Axis4r getOrientation() const override
    {
        return AirSimSimpleFlightCommon::toAxis4r(kinematics_->pose.orientation);
    }

    virtual hf::GeoPoint getGeoPoint() const override
    {
        return AirSimSimpleFlightCommon::toSimpleFlightGeoPoint(environment_->getState().geo_point);
    }

    virtual hf::GeoPoint getHomeGeoPoint() const override
    {
        return AirSimSimpleFlightCommon::toSimpleFlightGeoPoint(environment_->getHomeGeoPoint());
    }

    virtual hf::KinematicsState getKinematicsEstimated() const override
    {
        hf::KinematicsState state;
        state.position = getPosition();
        state.orientation = getOrientation();
        state.linear_velocity = getLinearVelocity();
        state.angular_velocity = getAngularVelocity();
        state.linear_acceleration = AirSimSimpleFlightCommon::toAxis3r(kinematics_->accelerations.linear);
        state.angular_acceleration = AirSimSimpleFlightCommon::toAxis3r(kinematics_->accelerations.angular);
        
        return state;
    }


private:
    const Kinematics::State* kinematics_;
    const Environment* environment_;
};


}} //namespace
#endif
