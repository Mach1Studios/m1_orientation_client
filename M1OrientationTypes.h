#pragma once

#include "m1_mathematics/Orientation.h"

#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <variant>

struct M1OrientationTrackingResult {
    Mach1::Orientation currentOrientation;
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

extern std::map<enum M1OrientationDeviceType, std::string> M1OrientationDeviceTypeName;

enum M1OrientationStatusType {
    M1OrientationManagerStatusTypeError = -2,
    M1OrientationManagerStatusTypeNotConnectable,
    M1OrientationManagerStatusTypeConnectable,
    M1OrientationManagerStatusTypeConnected,
};

extern std::map<enum M1OrientationStatusType, std::string> M1OrientationStatusTypeName;

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

    struct Hash {
        std::size_t operator()(const M1OrientationDeviceInfo& k) const
        {
            // Joshua Bloch, Effective Java, Addison-Wesley Professional (2018), p. 53
            std::size_t result = k.getDeviceNameHash();
            result = 31 * result + k.getDeviceAddressHash();
            return result;
        }
    };

    std::string getDeviceName() {
        return name;
    }

    std::size_t getDeviceNameHash() const {
        return std::hash<std::string>()( name );
    }

    bool isDeviceName(const std::string& query_name) const {
        return name.find(query_name) != std::string::npos;
    }

    M1OrientationDeviceType getDeviceType() {
        return type;
    }
    
    std::string getDeviceAddress() {
        // [Serial]: returns the path
        // [BLE]: returns the UUID
        return address;
    }

    std::size_t getDeviceAddressHash() const {
        return std::hash<std::string>()( address);
    }

    bool isDeviceAddress(const std::string& query_address) const {
        return address.find(query_address) != std::string::npos;
    }

    // Keeping these getters for ease of documentation but these variables are now public
    std::variant<bool, int> getDeviceSignalStrength() {
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
    std::variant<bool, int> getDeviceBatteryPercentage() {
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
    
    bool operator==(const M1OrientationDeviceInfo& rhs) const {
        return ((name == rhs.name) && (address == rhs.address));
    }
    
    bool operator!=(const M1OrientationDeviceInfo& rhs) const {
        return ((name != rhs.name) || (address != rhs.address));
    }

public:
    bool notConnectable = false;
    bool newErrorToParse = false;
    std::string error = "";
    // osc specific
    int osc_port = 9901;
    std::string osc_msg_addr_pttrn = "/orientation";

private:
    std::string name = "";
    M1OrientationDeviceType type = M1OrientationDeviceType::M1OrientationManagerDeviceTypeNone;
    std::string address = ""; // Device path or UUID

public:
    std::variant<bool, int> signalStrength = false;
    std::variant<bool, int> batteryPercentage = false;
};
