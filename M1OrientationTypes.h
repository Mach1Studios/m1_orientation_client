#pragma once

#include <string>
#include <vector>
#include <map>

struct M1OrientationYPR {
    float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
    float yaw_min = -180.0f, pitch_min = -180.0f, roll_min = -180.0f;
    float yaw_max = 180.0f, pitch_max = 180.0f, roll_max = 180.0f;
    float custom_output_yaw = 0.0f, custom_output_pitch = 0.0f, custom_output_roll = 0.0f;
    enum AngleType {
        DEGREES = (int) 0,
        RADIANS = (int) 1,
        NORMALED = (int) 2
    };
};

struct M1OrientationQuat {
    float w = 0.0f, x = 0.0f, y = 0.0f, z = 0.0f;
    float lastw, lastx, lasty, lastz;
};

class Orientation {
    M1OrientationYPR orientationYPR;
    M1OrientationQuat orientationQuat;

public:
    void setYPR(M1OrientationYPR orientation);
    void setQuat(M1OrientationQuat orientation);

    M1OrientationYPR getYPR();
    M1OrientationQuat getQuat();

    void resetOrientation();
};

struct M1OrientationTrackingResult
{
    Orientation currentOrientation;
    bool success;
};

enum M1OrientationDeviceType {
    M1OrientationManagerDeviceTypeNone = 0,
    M1OrientationManagerDeviceTypeSerial,
    M1OrientationManagerDeviceTypeBLE,
    M1OrientationManagerDeviceTypeOSC,
    M1OrientationManagerDeviceTypeCamera,
};

enum M1OrientationStatusType {
    M1OrientationManagerStatusTypeError = -3,
    M1OrientationManagerStatusTypeNotConnectable,
    M1OrientationManagerStatusTypeUnknown,
    M1OrientationManagerStatusTypeConnectable,
    M1OrientationManagerStatusTypeConnected,
};

struct M1OrientationDevice {
	std::string name;
	M1OrientationDeviceType type;
    std::string path; // Device path or UUID
    M1OrientationStatusType state;
    int rssi;
    
//    M1OrientationDevice(std::string deviceName, M1OrientationDeviceType deviceType, std::string devicePath, M1OrientationStatusType deviceState, int deviceRssi){
//        name = deviceName;
//        type = deviceType;
//        path = devicePath;
//        state = deviceState;
//        rssi = deviceRssi;
//    }
    std::string getDeviceName(){
        return name;
    }
    M1OrientationDeviceType getDeviceType(){
        return type;
    }
    std::string getDevicePath(){
        return path;
    }
    M1OrientationStatusType getDeviceState(){
        return state;
    }
    int getDeviceRssi(){
        return rssi;
    }
    // Custom search function for string name of device
    struct find_id : std::unary_function<M1OrientationDevice, bool> {
        std::string name;
        find_id(std::string name):name(name) { }
        bool operator()(M1OrientationDevice const& m) const {
            return m.name == name;
        }
    };
};

extern std::map<enum M1OrientationDeviceType, std::string> M1OrientationDeviceTypeName;

extern std::map<enum M1OrientationStatusType, std::string> M1OrientationStatusTypeName;

