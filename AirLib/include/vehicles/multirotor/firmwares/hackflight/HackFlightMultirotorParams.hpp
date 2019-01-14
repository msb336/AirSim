#ifndef msr_airlib_vehicles_HackFlightMultirotor_hpp
#define msr_airlib_vehicles_HackFlightMultirotor_hpp

#include "vehicles/multirotor/firmwares/hackflight/HackFlightApi.hpp"
#include "common/AirSimSettings.hpp"
#include "sensors/SensorFactory.hpp"
#include "vehicles/multirotor/MultiRotorParams.hpp"

namespace msr {
	namespace airlib {
		class HackFlightMultiRotorParams : public MultiRotorParams {
		public :
			HackFlightMultiRotorParams(const AirSimSettings::HackFlightVehicleSetting& vehicle_setting, std::shared_ptr<const SensorFactory> sensor_factory) {};

			virtual ~HackFlightMultiRotorParams() = default;

			virtual std::unique_ptr<MultirotorApiBase> createMultirotorApi() override
			{
				unique_ptr<MultirotorApiBase> api(new HackFlightApi());
				auto api_ptr = static_cast<HackFlightApi*>(api.get());
				api_ptr->initialize(&getSensors(), true);

				return api;
			}

		};
} }