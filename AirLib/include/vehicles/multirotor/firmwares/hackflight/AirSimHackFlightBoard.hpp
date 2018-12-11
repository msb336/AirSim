// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef msr_airlib_AirSimHackFlightBoard_hpp
#define msr_airlib_AirSimHackFlightBoard_hpp

#include <exception>
#include <vector>
#include "firmware/board.hpp"
#include "firmware/Params.hpp"
#include "common/Common.hpp"
#include "common/ClockFactory.hpp"
#include "physics/Kinematics.hpp"

//TODO: get values from sensors

namespace msr { namespace airlib {

class AirSimHackFlightBoard : public hf::Board {
public:
    AirSimHackFlightBoard(const hf::Params* params)
        : params_(params)
    {
    }

    //interface for simulator --------------------------------------------------------------------------------
    //for now we don't do any state estimation and use ground truth (i.e. assume perfect sensors)
    void setGroundTruthKinematics(const Kinematics::State* kinematics)
    {
        kinematics_ = kinematics;
    }

    //called to get o/p motor signal as float value
    real_T getMotorControlSignal(uint index) const
    {
        //convert PWM to scaled 0 to 1 control signal
        return static_cast<float>(motor_output_[index]);
    }

    //set current RC stick status
    void setInputChannel(uint index, real_T val)
    {
        input_channels_[index] = static_cast<float>(val);
    }

    void setIsRcConnected(bool is_connected)
    {
        is_connected_ = is_connected;
    }

public:
    //Board interface implementation --------------------------------------------------------------------------

    virtual bool  getQuaternion(float quat[4])
    {for(int i=0; i<4; i++){quat[i]=0;} return true;}

    virtual bool  getGyrometer(float gyroRates[3]) = 0;
    virtual void  writeMotor(uint8_t index, float value) = 0;
    virtual float getTime(void) = 0;

    //------------------------- Support for additional surface-mount sensors -------------------------------------
    virtual bool  getAccelerometer(float accelGs[3]) 
    { accelGs[0] = 0; accelGs[1] =0; accelGs[2]=0; return true; }

    virtual bool  getMagnetometer(float uTs[3]) 
    { uTs[0]=0; uTs[1]=0; uTs[2]=0;  return true;}

    virtual bool  getBarometer(float & pressure) 
    { pressure=0;  return true; }

    //------------------------------- Serial communications via MSP ----------------------------------------------
    virtual uint8_t serialAvailableBytes(void) { return 0; }
    virtual uint8_t serialReadByte(void)  { return 0; }
    virtual void    serialWriteByte(uint8_t c) { (void)c; }

    //------------------------------- Reboot for non-Arduino boards ---------------------------------------------
    // virtual void reboot(void) { }

    //----------------------------------------- Safety -----------------------------------------------------------
    virtual void showArmedStatus(bool armed) { (void)armed; }
    virtual void flashLed(bool shouldflash) { (void)shouldflash; }
    virtual bool isBatteryLow(void) { return false; }

private:
    void sleep(double msec)
    {
        clock()->sleep_for(msec * 1000.0);
    }

    const ClockBase* clock() const
    {
        return ClockFactory::get();
    }

    ClockBase* clock()
    {
        return ClockFactory::get();
    }

private:
    //motor outputs
    std::vector<float> motor_output_;
    std::vector<float> input_channels_;
    bool is_connected_;

    const hf::Params* params_;
    const Kinematics::State* kinematics_;
};

}} //namespace
#endif
