#pragma once

#include <string>
#include <vector>
#include <map>
#include <variant>

struct M1OrientationYPR {
    float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
    float yaw_min = -180.0f, pitch_min = -180.0f, roll_min = -180.0f;
    float yaw_max = 180.0f, pitch_max = 180.0f, roll_max = 180.0f;
    float custom_output_yaw = 0.0f, custom_output_pitch = 0.0f, custom_output_roll = 0.0f;
    enum AngleType {
        DEGREES = (int) 0,
        RADIANS = (int) 1,
        NORMALED = (int) 2
    } angleType;
};

struct M1OrientationQuat {
    float w = 1.0f, x = 0.0f, y = 0.0f, z = 0.0f; // Used for getting/reading processed values
    float wIn = 1.0f, xIn = 0.0f, yIn = 0.0f, zIn = 0.0f; // Used for setting new input values
    float wb = 1.0f, xb = 0.0f, yb = 0.0f, zb = 0.0f; // Used for resets
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
	M1OrientationManagerDeviceTypeEmulator = -1,
	M1OrientationManagerDeviceTypeNone = 0,
    M1OrientationManagerDeviceTypeSerial,
    M1OrientationManagerDeviceTypeBLE,
    M1OrientationManagerDeviceTypeOSC,
	M1OrientationManagerDeviceTypeCamera,
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
