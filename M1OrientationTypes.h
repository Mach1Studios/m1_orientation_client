#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <variant>

// All orientation is calculated internally as signed normalled YPR which is used to set the quaternion
// All getters just convert from the internal calculations
// It is recommended to not use UNSIGNED_NORMALLED as there can be confusion about what 0 represents

struct M1OrientationYPR {
    // used for reading the orientation setting absolute rotations
    float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
    
    // used for calculators to change format of YPR
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
		M1OrientationYPR result = *this;
		result.yaw = std::fmod((yaw + a.yaw), yaw_max - yaw_min);
		result.pitch = std::fmod((pitch + a.pitch), pitch_max - pitch_min);
		result.roll = std::fmod((roll + a.roll), roll_max - roll_min);
        return result;
    }
    
    // used to find the delta of two rotations
    M1OrientationYPR& operator -(const M1OrientationYPR& a) {
        // subtract together and keep within set bounds
		M1OrientationYPR result = *this;
		result.yaw = std::fmod((yaw - a.yaw), yaw_max - yaw_min);
		result.pitch = std::fmod((pitch - a.pitch), pitch_max - pitch_min);
		result.roll = std::fmod((roll - a.roll), roll_max - roll_min);
		return result;
	}
    
    // used to find if the two orientations are the same
    bool operator ==(const M1OrientationYPR& a) {
        M1OrientationYPR result = *this;
        return (result.yaw == a.yaw && result.pitch == a.pitch && result.roll == a.roll);
    }
    
    void setYaw(float _yaw) {
        this->yaw = _yaw;
    }
    
    void setPitch(float _pitch) {
        this->pitch = _pitch;
    }
    
    void setRoll(float _roll) {
        this->roll = _roll;
    }
};

struct M1OrientationQuat {
    // TODO: setters and getters
    float w = 1.0f, x = 0.0f, y = 0.0f, z = 0.0f; // Used for getting/reading processed values
    float wIn = 1.0f, xIn = 0.0f, yIn = 0.0f, zIn = 0.0f; // Used for setting/writing new values
    float wb = 1.0f, xb = 0.0f, yb = 0.0f, zb = 0.0f; // Used for resets and shifts
};

class Orientation {
public:
    // setting absolute orientation values
    void setYPR(M1OrientationYPR orientation); // use AngleType enum to determine how to set
    void setQuat(M1OrientationQuat orientation);
    // offsetting or adding to existing orientation values
    void offsetYPR(M1OrientationYPR offset);
    void offsetQuat(M1OrientationQuat offset);
    // set the YPR style in type and range
    void setYPR_type(M1OrientationYPR::AngleType type);
    void setYPR_range(float min_yaw, float min_pitch, float min_roll, float max_yaw, float max_pitch, float max_roll);
    
    // getting orientation
    M1OrientationYPR getYPRasUnsignedNormalled();
    M1OrientationYPR getYPRasSignedNormalled();
    M1OrientationYPR getYPRasDegrees();
    M1OrientationYPR getYPRasRadians();
    M1OrientationQuat getQuat();
    void resetOrientation();
    void recenter();
    
private:
    // Use getters and setters only
    M1OrientationYPR orientationYPR;
    M1OrientationYPR shiftYPR; // used to shift and reset orientation in YPR
    M1OrientationQuat orientationQuat;
};

// orientation transform functions
M1OrientationYPR getSignedNormalled(M1OrientationYPR orientation);
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
