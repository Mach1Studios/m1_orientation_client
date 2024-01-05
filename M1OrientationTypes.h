#pragma once

#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <cmath>
#include <stdexcept>


class Quaternion {
public:
    double w, x, y, z;

    Quaternion(double w = 1.0, double x = 0.0, double y = 0.0, double z = 0.0)
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
    Quaternion operator*(const Quaternion& other) const {
        return Quaternion(
            w * other.w - x * other.x - y * other.y - z * other.z,
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w);
    }
};

class M1Orientation {
public:
    M1Orientation() : quat() {}
    
    M1Orientation(double w, double x, double y, double z) : quat(w, x, y, z) {}

    
    // Set orientation directly from a Quaternion object
    void setFromQuaternion(const Quaternion& q) {
        quat = q;
        quat.normalize();
    }

    // Set orientation directly from quaternion values
    void setFromQuaternion(double w, double x, double y, double z) {
        quat = Quaternion(w, x, y, z);
        quat.normalize();
    }

    // Setting the internal representation from incoming yaw, pitch and roll in radians
    void setFromEulerYXZ(double yaw, double pitch, double roll, bool signedAngles = true, bool normalized = true) {
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
        yaw *= M_PI;
        pitch *= M_PI;
        roll *= M_PI;
        quat = eulerToQuaternion(yaw, pitch, roll);
    }
    
    // Setting the internal representation from incoming yaw, pitch and roll in degrees
    void setFromEulerYXZDegrees(double yaw, double pitch, double roll, bool signedAngles = true, bool normalized = true) {
        const double degToRad = M_PI / 180.0;

        // Convert from degrees to radians
        yaw *= degToRad;
        pitch *= degToRad;
        roll *= degToRad;

        // Now call the existing setFromEulerYXZ with the radians
        setFromEulerYXZ(yaw, pitch, roll, signedAngles, normalized);
    }
    
    // getting the representation in a shape of a Yaw, Pitch, Roll in radians
    std::tuple<double, double, double> getAsEulerYXZ(bool signedAngles = true, bool normalized = true) const {
        double yaw, pitch, roll;
        quaternionToEuler(quat, yaw, pitch, roll); // Convert to Euler angles in radians

        if (normalized) {
            // Normalize to -1 to 1 (signed) or 0 to 1 (unsigned)
            yaw = signedAngles ? yaw / M_PI : (yaw + M_PI) / (2 * M_PI);
            pitch = signedAngles ? pitch / (M_PI / 2) : (pitch + (M_PI / 2)) / M_PI;
            roll = signedAngles ? roll / M_PI : (roll + M_PI) / (2 * M_PI);
        } else {
            if (!signedAngles) {
                // Adjust to 0 to π (yaw) and -π/2 to π/2 (pitch and roll) for unsigned angles
                yaw = (yaw < 0) ? yaw + 2 * M_PI : yaw;
                pitch = (pitch < -M_PI / 2) ? pitch + M_PI : (pitch > M_PI / 2) ? pitch - M_PI : pitch;
                roll = (roll < 0) ? roll + 2 * M_PI : roll;
            }
        }

        return std::make_tuple(yaw, pitch, roll);
    }

    void setToZero() {
        quat = Quaternion(1.0, 0.0, 0.0, 0.0); // Quaternion representing no rotation
    }

    Quaternion getAsQuaternion() const { return quat; }

    M1Orientation operator+(const M1Orientation& other) const {
        Quaternion resultQuat = quat * other.quat;
        resultQuat.normalize();
        return M1Orientation(resultQuat);
    }
    
    std::tuple<double, double, double> getAsEulerYXZDegrees(bool signedAngles = true, bool normalized = true) const {
        auto [yaw, pitch, roll] = getAsEulerYXZ(signedAngles, normalized);

        if (!normalized) {
            // Convert from radians to degrees for non-normalized output
            const double radToDeg = 180.0 / M_PI;
            yaw *= radToDeg;
            pitch *= radToDeg;
            roll *= radToDeg;
        }

        return std::make_tuple(yaw, pitch, roll);
    }

    M1Orientation operator-(const M1Orientation& other) const {
        // Subtracting orientations requires more considerations
        // This is a placeholder for a more complex implementation
        Quaternion inverseOther = Quaternion(other.quat.w, -other.quat.x, -other.quat.y, -other.quat.z);
        Quaternion resultQuat = quat * inverseOther;
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
    Quaternion quat;

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

    Quaternion eulerToQuaternion(double yaw, double pitch, double roll) {
        // Convert Euler angles (in radians) to quaternion
        // Assuming YXZ order (yaw, pitch, roll)
        double cy = cos(yaw * 0.5);
        double sy = sin(yaw * 0.5);
        double cp = cos(pitch * 0.5);
        double sp = sin(pitch * 0.5);
        double cr = cos(roll * 0.5);
        double sr = sin(roll * 0.5);

        return Quaternion(
            cy * cp * cr + sy * sp * sr,
            cy * cp * sr - sy * sp * cr,
            sy * cp * sr + cy * sp * cr,
            sy * cp * cr - cy * sp * sr);
    }

    void quaternionToEuler(const Quaternion& q, double& yaw, double& pitch, double& roll) const {
        // Convert quaternion to Euler angles
        // Assuming YXZ order (yaw, pitch, roll)
        double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
        double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
        roll = std::atan2(sinr_cosp, cosr_cosp);

        double sinp = 2 * (q.w * q.y - q.z * q.x);
        if (std::abs(sinp) >= 1)
            pitch = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
        else
            pitch = std::asin(sinp);

        double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
        double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
        yaw = std::atan2(siny_cosp, cosy_cosp);
    }

    M1Orientation(const Quaternion& quat) : quat(quat) {}
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
