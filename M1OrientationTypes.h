#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <cmath>
#include <stdexcept>
#include <iostream>

class M1Quaternion {
public:
    double w, x, y, z;

    M1Quaternion(double w = 1.0, double x = 0.0, double y = 0.0, double z = 0.0)
        : w(w), x(x), y(y), z(z) {}

    // Normalize the quaternion
    void normalize() {
        double norm = std::sqrt(w * w + x * x + y * y + z * z);
        w /= norm;
        x /= norm;
        y /= norm;
        z /= norm;
    }

    // Multiply two quaternions
    M1Quaternion operator*(const M1Quaternion& other) const {
        return M1Quaternion(
            w * other.w - x * other.x - y * other.y - z * other.z,
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w);
    }
};

class EulerAngleSet {
public:
    double yaw;
    double pitch;
    double roll;
    
    EulerAngleSet() {
        yaw = 0;
        pitch = 0;
        roll = 0;
    }

    EulerAngleSet(double yaw, double pitch, double roll)
        : yaw(yaw), pitch(pitch), roll(roll) {}
};

class M1Orientation {
public:
    M1Orientation() : quat() {}
    M1Orientation(double w, double x, double y, double z) : quat(w, x, y, z) {}
    
    // Set orientation directly from a Quaternion object
    void setFromQuaternion(const M1Quaternion& q) {
        quat = q;
        quat.normalize();
    }

    // Set orientation directly from quaternion values
    void setFromQuaternion(double w, double x, double y, double z) {
        quat = M1Quaternion(w, x, y, z);
        quat.normalize();
    }

    void setFromEulerYPRRadians(double yaw, double pitch, double roll, bool signedAngles = true) {
        setFromEulerYPRRadiansInternal(yaw, pitch, roll, signedAngles, false);
    }
    
    void setFromEulerYPRDegrees(double yaw, double pitch, double roll, bool signedAngles = true) {
        const double degToRad = M_PI / 180.0;

        // Convert from degrees to radians
        yaw *= degToRad;
        pitch *= degToRad;
        roll *= degToRad;

        // Now call the existing setFromEulerYXZ with the radians
        setFromEulerYPRRadiansInternal(yaw, pitch, roll, signedAngles, false);
    }
    
    void setFromEulerYPRNormalized(double yaw, double pitch, double roll, bool signedAngles = true) {

        // Now call the existing setFromEulerYXZ with the radians
        setFromEulerYPRRadiansInternal(yaw, pitch, roll, signedAngles, true);
    }
    
    // getting the representation in a shape of a Yaw, Pitch, Roll in radians or in normalized values if normalized = true (0.0 to 1.0 in case of signedAngles = false, -1.0 to 1.0 in case of signedAngles = true)
    // TODO: make it follow the same pattern as the setters
    EulerAngleSet getAsEulerYPRRadians(bool signedAngles = true) const {
        return getAsEulerYPRRadiansInternal(signedAngles);
    }

    void setToZero() {
        quat = M1Quaternion(1.0, 0.0, 0.0, 0.0); // Quaternion representing no rotation
    }
    
    void resetOrientation() {
        setToZero();
    }

    M1Quaternion getAsQuaternion() const { return quat; }

    M1Orientation operator+(const M1Orientation& other) const {
        M1Quaternion resultQuat = quat * other.quat;
        resultQuat.normalize();
        return M1Orientation(resultQuat);
    }
    
    EulerAngleSet getAsEulerYPRDegrees(bool signedAngles = true) const {
        auto [yaw, pitch, roll] = getAsEulerYPRRadiansInternal(signedAngles); // returning them non normalized

        // Convert from radians to degrees
        const double radToDeg = 180.0 / M_PI;
        yaw *= radToDeg;
        pitch *= radToDeg;
        roll *= radToDeg;

        return EulerAngleSet(yaw, pitch, roll);
    }
    
    EulerAngleSet getAsEulerYPRNormalized(bool signedAngles = true) const {
        auto [yaw, pitch, roll] = getAsEulerYPRRadiansInternal(signedAngles, true); // returning them normalized
        return EulerAngleSet(yaw, pitch, roll);
    }

    M1Orientation operator-(const M1Orientation& other) const {
        // Subtracting orientations requires more considerations
        // This is a placeholder for a more complex implementation
        M1Quaternion inverseOther = M1Quaternion(other.quat.w, -other.quat.x, -other.quat.y, -other.quat.z);
        M1Quaternion resultQuat = quat * inverseOther;
        resultQuat.normalize();
        return M1Orientation(resultQuat);
    }

    bool operator==(const M1Orientation& other) const {
        // Simple comparison with a low precision threshold
        const double threshold = 1e-4;
        return std::abs(quat.w - other.quat.w) < threshold &&
               std::abs(quat.x - other.quat.x) < threshold &&
               std::abs(quat.y - other.quat.y) < threshold &&
               std::abs(quat.z - other.quat.z) < threshold;
    }

private:
    M1Quaternion quat;

    double normalizeAngle(double angle) {
        // Normalize angle to [-PI, PI]
        while (angle > M_PI) angle -= 2 * M_PI;
        while (angle < -M_PI) angle += 2 * M_PI;
        return angle;
    }

    double denormalizeAngle(double angle) const {
        // Convert from [-PI, PI] to [-180, 180]
        return angle * 180.0 / M_PI;
    }
    
    void setFromEulerYPRRadiansInternal(double yaw, double pitch, double roll, bool signedAngles = true, bool normalized = true) {
        if (!signedAngles) {
            yaw = 2.0 * (yaw - 0.5);
            pitch = 2.0 * (pitch - 0.5);
            roll = 2.0 * (roll - 0.5);
        }
        if (!normalized) {
            yaw = normalizeAngle(yaw);
            pitch = normalizeAngle(pitch);
            roll = normalizeAngle(roll);
        }

        // Convert Euler angles to quaternion
        quat = eulerToQuaternion(yaw, pitch, roll);
    }
    
    EulerAngleSet getAsEulerYPRRadiansInternal(bool signedAngles = true, bool normalized = false) const {
        double yaw, pitch, roll;
        quaternionToEuler(quat, yaw, pitch, roll); // Convert to Euler angles in radians

        if (normalized) {
            yaw = signedAngles ? yaw / M_PI : (yaw + M_PI) / (2 * M_PI);
            pitch = signedAngles ? pitch / (M_PI / 2) : (pitch + (M_PI / 2)) / M_PI;
            roll = signedAngles ? roll / M_PI : (roll + M_PI) / (2 * M_PI);
        } else {
            if (!signedAngles) {
                yaw = (yaw < 0) ? yaw + 2 * M_PI : yaw;
                pitch = (pitch < -M_PI / 2) ? pitch + M_PI : (pitch > M_PI / 2) ? pitch - M_PI : pitch;
                roll = (roll < 0) ? roll + 2 * M_PI : roll;
            }
        }

        return EulerAngleSet(yaw, pitch, roll);
    }

    M1Quaternion eulerToQuaternion(double yaw, double pitch, double roll) {
        
        double cy = cos(yaw * 0.5);
        double sy = sin(yaw * 0.5);
        double cp = cos(pitch * 0.5);
        double sp = sin(pitch * 0.5);
        double cr = cos(roll * 0.5);
        double sr = sin(roll * 0.5);

        M1Quaternion result(
            cy * cp * cr + sy * sp * sr, // w
            cy * cp * sr - sy * sp * cr, // x
            sy * cp * sr + cy * sp * cr, // y
            sy * cp * cr - cy * sp * sr  // z
        );

        return result;
    }

    void quaternionToEuler(const M1Quaternion& q, double& yaw, double& pitch, double& roll) const {
        // Extract sin(pitch)
        double sinp = 2 * (q.w * q.y - q.z * q.x);

        // Compute pitch (y-axis rotation)
        if (std::abs(sinp) >= 1)
            pitch = std::copysign(M_PI / 2, sinp); // use π/2 or -π/2 if out of range
        else
            pitch = std::asin(sinp);

        // Compute yaw (y-axis rotation) and roll (z-axis rotation)
        if (std::abs(sinp) < 0.9999) { // Avoid gimbal lock
            yaw = std::atan2(2 * (q.w * q.z + q.x * q.y), 1 - 2 * (q.y * q.y + q.z * q.z));
            roll = std::atan2(2 * (q.w * q.x + q.y * q.z), 1 - 2 * (q.x * q.x + q.y * q.y));
        } else {
            // Gimbal lock, arbitrary set yaw to 0
            yaw = 0;
            roll = std::atan2(-2 * (q.x * q.z - q.w * q.y), 2 * (q.w * q.x + q.y * q.z));
        }
    }

    M1Orientation(const M1Quaternion& quat) : quat(quat) {}
};

struct M1OrientationTrackingResult {
    M1Orientation currentOrientation;
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
