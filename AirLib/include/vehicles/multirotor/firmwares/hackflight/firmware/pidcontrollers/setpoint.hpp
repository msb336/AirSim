/*
   setpoint.hpp : Helper class for PID controllers using a set-point

   See https://raw.githubusercontent.com/wiki/iNavFlight/inav/images/nav_poshold_pids_diagram.jpg

   Copyright (c) 2018 Juan Gallostra and Simon D. Levy

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

#include "receiver.hpp"
#include "filters.hpp"

namespace hf {

    class Setpoint {

        private: 

            // PID ants set by ructor
            float _posP;
            float _velP;
            float _velI;
            float _velD;

            // Parameter to avoid integral windup
            float _windupMax;

            // Values modified in-flight
            float deltaT;
            float _posTarget;
            bool  _inBandPrev;
            float _lastError;
            float _integralError;
            float _altitudeTarget;
            float _previousTime;

            bool inBand(float demand)
            {
                return fabs(demand) < Receiver::STICK_DEADBAND; 
            }

            void resetErrors(void)
            {
                _lastError = 0;
                _integralError = 0;
            }

        public:

            bool gotCorrection(float demand, float posActual, float velActual, float currentTime, float & correction)
            {
                // Don't do anything until we have a positive deltaT
                float deltaT = currentTime - _previousTime;
                _previousTime = currentTime;
                if (deltaT == currentTime) return false;

                // Reset altitude target if moved into stick deadband
                bool inBandCurr = inBand(demand);
                if (inBandCurr && !_inBandPrev) {
                    _posTarget = posActual;
                    resetErrors();
                }
                _inBandPrev = inBandCurr;

                if (!inBandCurr) return false;
            
                // compute velocity setpoint and error
                float velTarget = (_posTarget - posActual) * _posP;
                float velError = velTarget - velActual;

                // Update error integral and error derivative
                _integralError = Filter::constrainAbs(_integralError + velError * deltaT, _windupMax);
                float deltaError = (velError - _lastError) / deltaT;
                _lastError = velError;

                // Compute control correction
                correction = _velP * velError + _velD * deltaError + _velI * _integralError;                       

                return true;
            }

            void init(float posP, float velP, float velI, float velD, float windupMax)
            {
                _posP = posP; 
                _velP = velP; 
                _velI = velI; 
                _velD = velD; 
                _windupMax = windupMax;
                resetErrors();
                _posTarget = 0;
                _previousTime = 0;
                _inBandPrev = false;
            }

    };  // class Setpoint

} // namespace hf
