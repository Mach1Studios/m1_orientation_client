#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <variant>

// All orientation is calculated internally as degree YPR which is used to set the quaternion
// TODO: update internal math to use normal and radians only for YPR
// All getters just convert from the internal calculations

struct M1OrientationYPR {
    float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
    float yaw_min = -1.0f, pitch_min = -1.0f, roll_min = -1.0f;
    float yaw_max =  1.0f, pitch_max =  1.0f, roll_max =  1.0f;
    enum AngleType {
        // default construction as signed normalled
        SIGNED_NORMALLED   = (int) 0, // -1.0   -> 1.0
        UNSIGNED_NORMALLED = (int) 1, // 0.0    -> 1.0
        DEGREES            = (int) 2, // -180.0 -> 180.0
        RADIANS            = (int) 3  // -PI    -> PI
    } angleType;
    
    // used to add two rotations together
    M1OrientationYPR& operator +(const M1OrientationYPR& a) {
        // add together and keep within set bounds
        yaw = std::fmod((yaw + a.yaw), yaw_max - yaw_min);
        pitch = std::fmod((pitch + a.pitch), pitch_max - pitch_min);
        roll = std::fmod((roll + a.roll), roll_max - roll_min);
        return *this;
    }
    
    // used to find the delta of two rotations
    M1OrientationYPR& operator -(const M1OrientationYPR& a) {
        // subtract together and keep within set bounds
        yaw = std::fmod((yaw - a.yaw), yaw_max - yaw_min);
        pitch = std::fmod((pitch - a.pitch), pitch_max - pitch_min);
        roll = std::fmod((roll - a.roll), roll_max - roll_min);
        return *this;
    }
    
    M1OrientationYPR& update_yaw(float yaw) {
        this->yaw = yaw;
    }
    
    M1OrientationYPR& update_pitch(float pitch) {
        this->pitch = pitch;
    }
    
    M1OrientationYPR& update_roll(float roll) {
        this->roll = roll;
    }
};

struct M1OrientationQuat {
    // TODO: add getter/setter for in/out flow
    float w = 1.0f, x = 0.0f, y = 0.0f, z = 0.0f; // Used for getting/reading processed values
    float wIn = 1.0f, xIn = 0.0f, yIn = 0.0f, zIn = 0.0f; // Used for setting/writing new values
    float wb = 1.0f, xb = 0.0f, yb = 0.0f, zb = 0.0f; // Used for resets
};

class Orientation {
public:
    // setting absolute orientation values
    void setYPR(M1OrientationYPR orientation); // use AngleType enum to determine how to set
    void setQuat(M1OrientationQuat orientation);
    // offsetting or adding to existing orientation values
    void offsetYPR(M1OrientationYPR offset_delta);
    void offsetQuat(M1OrientationQuat offset_delta);
    // getting orientation
    M1OrientationYPR getYPRasUnsignedNormalled();
    M1OrientationYPR getYPRasSignedNormalled();
    M1OrientationYPR getYPRinDegrees();
    M1OrientationYPR getYPRinRadians();
    M1OrientationQuat getQuat();
    void resetOrientation();
    void recenter();
    
private:
    // Use getters and setters only
    M1OrientationYPR orientationYPR;
    M1OrientationQuat orientationQuat;
};

// orientation transform functions
M1OrientationYPR getUnsignedNormalled(M1OrientationYPR orientation);
M1OrientationQuat getNormalled(M1OrientationQuat orientation);

struct M1OrientationTrackingResult {
    Orientation currentOrientation;
    bool success;
};

enum M1OrientationDeviceType {
	M1OrientationManagerDeviceTypeEmulator = -1,
	M1OrientationManagerDeviceTypeNone = 0,
    M1OrientationManagerDeviceTypeSerial,
    M1OrientationManagerDeviceTypeBLE,
    M1OrientationManagerDeviceTypeOSC,
	M1OrientationManagerDeviceTypeCamera,
    M1OrientationManagerDeviceTypeFusion, // TODO: Add Camera + [any] type
};

enum M1OrientationStatusType {
    M1OrientationManagerStatusTypeError = -2,
    M1OrientationManagerStatusTypeNotConnectable,
    M1OrientationManagerStatusTypeConnectable,
    M1OrientationManagerStatusTypeConnected,
};

struct M1OrientationDeviceInfo {
public:
    // Constructor
    M1OrientationDeviceInfo() {}
    M1OrientationDeviceInfo(std::string name_, M1OrientationDeviceType type_, std::string address_, std::variant<bool, int> signalStrength_ = false, std::variant<bool, int> batteryPercentage_ = false) {
        name = name_;
        type = type_;
        address = address_;
        signalStrength = signalStrength_;
        batteryPercentage = batteryPercentage_;
    }
    
	bool notConnectable = false;
    bool newErrorToParse = false;
    std::string error = "";
    // osc specific
    int osc_port = 9901;
    std::string osc_msg_addr_pttrn = "/orientation";

    std::string getDeviceName(){
        return name;
    }
    
    M1OrientationDeviceType getDeviceType(){
        return type;
    }
    
    std::string getDeviceAddress(){
        // [Serial]: returns the path
        // [BLE]: returns the UUID
        return address;
    }
    
    // Keeping these getters for ease of documentation but these variables are now public
    std::variant<bool, int> getDeviceSignalStrength(){
        return signalStrength;
        
        /* Reference:
        if (std::holds_alternative<bool>(signalStrength)) {
            // it's false, which means the signal strength is unknown
        } else {
            // it has a signal strength value
        }
        */
    }
    
    // Keeping these getters for ease of documentation but these variables are now public
    std::variant<bool, int> getDeviceBatteryPercentage(){
        return batteryPercentage;
        
        /* Reference:
        if (std::holds_alternative<bool>(batteryPercentage)) {
            // it's false, which means the battery percentage is unknown
        } else {
            // it has a battery percentage value
        }
        */
    }
    
    // Custom search function for string name of device
    struct find_id {
        std::string name;
        find_id(std::string name):name(name) { }
        bool operator()(M1OrientationDeviceInfo const& m) const {
            return m.name == name;
        }
    };
    
    bool operator==(const M1OrientationDeviceInfo& rhs) {
        return ((name == rhs.name) && (address == rhs.address));
    }
    
    bool operator!=(const M1OrientationDeviceInfo& rhs) {
        return ((name != rhs.name) || (address != rhs.address));
    }
    
private:
    std::string name = "";
    M1OrientationDeviceType type = M1OrientationDeviceType::M1OrientationManagerDeviceTypeNone;
    std::string address = ""; // Device path or UUID
public:
    std::variant<bool, int> signalStrength = false;
    std::variant<bool, int> batteryPercentage = false;
};

extern std::map<enum M1OrientationDeviceType, std::string> M1OrientationDeviceTypeName;
extern std::map<enum M1OrientationStatusType, std::string> M1OrientationStatusTypeName;
