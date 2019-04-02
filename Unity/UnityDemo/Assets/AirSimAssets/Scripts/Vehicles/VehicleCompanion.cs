using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using AirSimUnity.DroneStructs;
using AirSimUnity.CarStructs;
using UnityEngine;

namespace AirSimUnity {
    /*
     * An implementation of IAirSimInterface, facilitating calls from Unity to AirLib.
     * And also a bridge for the calls originating from AirLib into Unity.
     *
     * Unity client components should use an instance of this class to interact with AirLib.
     */
    struct StaticMesh
    {
        // public GameObject game_object;
        public AirSimPose pose;
        public string name;
        public GameObject game_object;
    }

    class TeleportClass
    {
        //* Change to unity version */
        public Transform pose;
        public string name;
        public bool triggered = false;
    }
    internal class VehicleCompanion : IAirSimInterface {

        //All the vehicles that are created in this game.
        private static List<VehicleCompanion> Vehicles = new List<VehicleCompanion>();

        private static List<StaticMesh> GameObjects = new List<StaticMesh>();
        private static TeleportClass teleport_;

        private static int basePortId;

        //An interface to interact with Unity vehicle component.
        private IVehicleInterface VehicleInterface;

        private string vehicleName;
        private readonly bool isDrone;

        static VehicleCompanion() {
            InitDelegators();
        }

        private VehicleCompanion(IVehicleInterface vehicleInterface) {
            VehicleInterface = vehicleInterface;
            isDrone = vehicleInterface is Drone ? true : false;
            basePortId = AirSimSettings.GetSettings().GetPortIDForVehicle(isDrone);
        }

        public static VehicleCompanion GetVehicleCompanion(IVehicleInterface vehicleInterface) {
            var companion = new VehicleCompanion(vehicleInterface);

            if (AirSimSettings.GetSettings().SimMode == "Car")
                companion.vehicleName = "PhysXCar";
            else if (AirSimSettings.GetSettings().SimMode == "Multirotor")
                companion.vehicleName = "SimpleFlight";

            Vehicles.Add(companion);
            return companion;
        }

        public bool StartVehicleServer(string hostIP) {
            return PInvokeWrapper.StartServer(vehicleName, AirSimSettings.GetSettings().SimMode, basePortId);
        }

        public void StopVehicleServer() {
            PInvokeWrapper.StopServer(vehicleName);
        }

        public void InvokeTickInAirSim(float deltaSecond)
        {
            PInvokeWrapper.CallTick(deltaSecond);
        }

        public void InvokeCollisionDetectionInAirSim(CollisionInfo collisionInfo)
        {
            PInvokeWrapper.InvokeCollisionDetection(collisionInfo);
        }

        public KinemticState GetKinematicState() {
            return PInvokeWrapper.GetKinematicState(vehicleName);
        }

        public static DataRecorder.ImageData GetRecordingData() {
            return Vehicles[0].VehicleInterface.GetRecordingData();
        }

        public static DataCaptureScript GetCameraCaptureForRecording() {
            AirSimSettings.CamerasSettings recordCamSettings = AirSimSettings.GetSettings().Recording.Cameras[0];
            DataCaptureScript recordCam = Vehicles[0].VehicleInterface.GetCameraCapture(recordCamSettings.CameraName);
            return recordCam;
        }

        //Register the delegate functions to AirLib, based on IVehicleInterface
        private static void InitDelegators() {
            PInvokeWrapper.InitVehicleManager(
                Marshal.GetFunctionPointerForDelegate(new Func<AirSimPose, bool, string, bool>(SetPose)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, AirSimPose>(GetPose)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, CollisionInfo>(GetCollisionInfo)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, AirSimRCData>(GetRCData)),
                Marshal.GetFunctionPointerForDelegate(new Func<ImageRequest, string, ImageResponse>(GetSimImages)),
                Marshal.GetFunctionPointerForDelegate(new Func<int, RotorInfo, string, bool>(SetRotorSpeed)),
                Marshal.GetFunctionPointerForDelegate(new Func<bool, string, bool>(SetEnableApi)),
                Marshal.GetFunctionPointerForDelegate(new Func<CarControls, string, bool>(SetCarApiControls)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, CarState>(GetCarState)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, string, CameraInfo>(GetCameraInfo)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, AirSimQuaternion, string, bool>(SetCameraOrientation)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, int, bool, bool>(SetSegmentationObjectId)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, int>(GetSegmentationObjectId)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, string, string, int, bool>(PrintLogMessage)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, UnityTransform>(GetTransformFromUnity)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, bool>(Reset)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, AirSimVector>(GetVelocity)),
                Marshal.GetFunctionPointerForDelegate(new Func<AirSimVector, AirSimVector, string, RayCastHitResult>(GetRayCastHit)),
                Marshal.GetFunctionPointerForDelegate(new Func<string, float, bool>(Pause))
            );
        }

        /*********************** Delegate functions to be registered with AirLib *****************************/

        private static bool SetPose(AirSimPose pose, bool ignoreCollision, string vehicleName) {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            if ( vehicle == null){
                var static_mesh = GameObjects.Find(element => element.name == vehicleName);
                
                if (static_mesh.name == null){
                    return false;
                }
                else{
                    Debug.Log(static_mesh.name);
                    Vector3 position;
                    Quaternion rotation;
                    Transform new_pose;
                    AirSimVector test;
                    DataManager.SetToUnity(pose.position, ref new_pose.position);
                    DataManager.SetToUnity(pose.orientation, ref new_pose.rotation);
                    teleport_.pose.position = new_pose.position;
                    teleport_.pose.rotation = new_pose.rotation;
                    teleport_.triggered = true;
                    teleport_.name = vehicleName;
                    return true;
                }
            }
            else{
                vehicle.VehicleInterface.SetPose(pose, ignoreCollision);
                return true;
            }
            // return true;
        }
        public static void PerformTransform()
        {
            if (teleport_.triggered)
            {
                var static_mesh = GameObjects.Find(element => element.name == vehicleName);
                static_mesh.tranform.position = teleport_.pose;
            }
        }
        private static AirSimPose GetPose(string vehicleName) {
            // All get pose calls, vehicle or otherwise, occur here
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            Debug.Log(vehicle);
            AirSimPose pose;
            // Check if there is a Static mesh with the correct name
            if ( vehicle == null){
                var static_mesh = GameObjects.Find(element => element.name == vehicleName);
                Debug.Log(static_mesh.name);
                pose = static_mesh.pose;
            }
            else{
                pose = vehicle.VehicleInterface.GetPose();
            }
            

            // Need a wrapper in case there are no game objects with this name

            return pose;
            
        }


        public static void GetGameObjects(){
            var all_objects = GameObject.FindObjectsOfType<GameObject>();
            for (var i = 0; i < all_objects.Length; i++){
                var single_object = new StaticMesh();
                single_object.game_object = all_objects[i];
                DataManager.SetToAirSim(all_objects[i].transform.position, ref single_object.pose.position);
                DataManager.SetToAirSim(all_objects[i].transform.rotation, ref single_object.pose.orientation);
                single_object.name = all_objects[i].name;
                GameObjects.Add(single_object);
            }

            
        }

        private static CollisionInfo GetCollisionInfo(string vehicleName) {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.GetCollisionInfo();
        }

        private static AirSimRCData GetRCData(string vehicleName) {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.GetRCData();
        }

        private static ImageResponse GetSimImages(ImageRequest request, string vehicleName) {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.GetSimulationImages(request);
        }

        private static UnityTransform GetTransformFromUnity(string vehicleName)
        {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.GetTransform();
        }

        private static bool Reset(string vehicleName)
        {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            vehicle.VehicleInterface.ResetVehicle();
            return true;
        }

        private static AirSimVector GetVelocity(string vehicleName)
        {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.GetVelocity();
        }

        private static RayCastHitResult GetRayCastHit(AirSimVector start, AirSimVector end, string vehicleName)
        {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.GetRayCastHit(start, end);
        }

        private static bool SetRotorSpeed(int rotorIndex, RotorInfo rotorInfo, string vehicleName) {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.SetRotorSpeed(rotorIndex, rotorInfo);
        }

        private static bool SetEnableApi(bool enableApi, string vehicleName) {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.SetEnableApi(enableApi);
        }

        private static bool SetCarApiControls(CarControls controls, string vehicleName) {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.SetCarControls(controls);
        }

        private static CarState GetCarState(string vehicleName) {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.GetCarState();
        }

        private static CameraInfo GetCameraInfo(string cameraName, string vehicleName) {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.GetCameraInfo(cameraName);
        }

        private static bool SetCameraOrientation(string cameraName, AirSimQuaternion orientation, string vehicleName) {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.SetCameraOrientation(cameraName, orientation);
        }

        private static bool PrintLogMessage(string message, string messageParams, string vehicleName, int severity) {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.PrintLogMessage(message, messageParams, vehicleName, severity);
        }

        private static bool SetSegmentationObjectId(string objectName, int objectId, bool isNameRegex) {
            return Vehicle.SetSegmentationObjectId(objectName, objectId, isNameRegex);
        }

        private static int GetSegmentationObjectId(string objectName) {
            return Vehicle.GetSegmentationObjectId(objectName);
        }

        private static bool Pause(string vehicleName, float timeScale)
        {
            var vehicle = Vehicles.Find(element => element.vehicleName == vehicleName);
            return vehicle.VehicleInterface.Pause(timeScale);
        }
    }
}