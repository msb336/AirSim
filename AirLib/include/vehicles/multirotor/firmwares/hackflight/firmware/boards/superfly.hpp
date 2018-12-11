/*
   superfly.hpp : Superfly Hackable ESP8266 Flight Controller implementation of Hackflight Board routines

   Uses EM7180 SENtral Sensor Hub in master mode mode

   Copyright (c) 2018 Simon D. Levy

   This file is part of Hackflight.

   Hackflight is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Hackflight is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with Hackflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <Wire.h>
#include "arduino.hpp"
#include "sentral.hpp"

namespace hf {

    class SuperFly : public ArduinoBoard, public SentralBoard {

        private:

            const uint8_t MOTOR_PINS[4] = {4, 5, 12, 14};

        protected:

            void writeMotor(uint8_t index, float value)
            {
                // Scale motor value from [0,1] to [0,1023]
                analogWrite(MOTOR_PINS[index], (uint16_t)(value * 1023));
            }

            bool getGyrometer(float gyro[3])
            {
                return SentralBoard::getGyrometer(gyro);
            }

            bool getQuaternion(float quat[4])
            {
                return SentralBoard::getQuaternion(quat);
            }

        public:

            SuperFly(void) : ArduinoBoard(15)
            {
                // Start I^2C
                Wire.begin(0,2); // SDA (0), SCL (2) on ESP8266

                // Hang a bit before starting up the EM7180
                delay(100);

                SentralBoard::begin();

                 // Initialize the motors
                analogWriteFreq(200);  
                for (int k=0; k<4; ++k) {
                    analogWrite(MOTOR_PINS[k], 0);  
                }

                // Hang a bit more
                delay(100);
            }

    }; // class SuperFly

} // namespace hf
