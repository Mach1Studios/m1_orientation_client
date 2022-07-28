#pragma once

#include <JuceHeader.h>
#include "M1OrientationManagerOSCSettings.h"

/// Class for orientation and orientation utilities
/// Designed to aggregate all orientation handling to a single collection point
struct GlobalOrientation {
    struct YPR {
        float yaw, pitch, roll = 0.0f;
        float yaw_min, pitch_min, roll_min = -180.0f;
        float yaw_max, pitch_max, roll_max = 180.0f;
        float custom_output_yaw, custom_output_pitch, custom_output_roll = 0.0f;
    };
    
    struct Quaternion {
        double qw, qx, qy, qz = 0;
    };

    YPR ypr;
    Quaternion quat;
    
    YPR getYPR() {
        double qW, qX, qY, qZ;
        float y, p, r;
        const juce::Array<float> quats = { quat.qw, quat.qx, quat.qy, quat.qz };

        qW = quats[0];
        qX = quats[2];
        qY = quats[1];
        qZ = quats[3];

        double test = qX * qZ + qY * qW;
        if (test > 0.499999) {
            // singularity at north pole
            ypr.yaw = 2 * atan2(qX, qW);
            ypr.pitch = juce::MathConstants<double>::pi / 2;
            ypr.roll = 0;
            ypr.custom_output_yaw = (float)juce::jmap(ypr.yaw, (float)-180, (float)180, ypr.yaw_min, ypr.yaw_max);
            ypr.custom_output_pitch = (float)juce::jmap(ypr.pitch, (float)-180, (float)180, ypr.pitch_min, ypr.pitch_max);
            ypr.custom_output_roll = (float)juce::jmap(ypr.roll, (float)-180, (float)180, ypr.roll_min, ypr.roll_max);
            return ypr;
        }
        if (test < -0.499999) {
            // singularity at south pole
            ypr.yaw = -2 * atan2(qX, qW);
            ypr.pitch = -juce::MathConstants<double>::pi / 2;
            ypr.roll = 0;
            ypr.custom_output_yaw = (float)juce::jmap(ypr.yaw, (float)-180, (float)180, ypr.yaw_min, ypr.yaw_max);
            ypr.custom_output_pitch = (float)juce::jmap(ypr.pitch, (float)-180, (float)180, ypr.pitch_min, ypr.pitch_max);
            ypr.custom_output_roll = (float)juce::jmap(ypr.roll, (float)-180, (float)180, ypr.roll_min, ypr.roll_max);
            return ypr;
        }
        double sqx = qX * qX;
        double sqy = qZ * qZ;
        double sqz = qY * qY;

        y = atan2(2 * qZ*qW - 2 * qX*qY, 1 - 2 * sqy - 2 * sqz);
        p = asin(2 * test);
        r = atan2(2 * qX*qW - 2 * qZ*qY, 1 - 2 * sqx - 2 * sqz);

        y *= -1.0f;

        ypr.yaw = juce::radiansToDegrees(y);
        ypr.pitch = juce::radiansToDegrees(p);
        ypr.roll = juce::radiansToDegrees(r);
        
        // remap output ypr
        ypr.custom_output_yaw = (float)juce::jmap(ypr.yaw, (float)-180, (float)180, ypr.yaw_min, ypr.yaw_max);
        ypr.custom_output_pitch = (float)juce::jmap(ypr.pitch, (float)-180, (float)180, ypr.pitch_min, ypr.pitch_max);
        ypr.custom_output_roll = (float)juce::jmap(ypr.roll, (float)-180, (float)180, ypr.roll_min, ypr.roll_max);

        return ypr;
    };
    
    Quaternion getQuaternion() {
        /// It is better to avoid this function and stick to updating quat and calculating best YPR
        quat.qx = sin(ypr.roll/2) * cos(ypr.pitch/2) * cos(ypr.yaw/2) - cos(ypr.roll/2) * sin(ypr.pitch/2) * sin(ypr.yaw/2);
        quat.qy = cos(ypr.roll/2) * sin(ypr.pitch/2) * cos(ypr.yaw/2) + sin(ypr.roll/2) * cos(ypr.pitch/2) * sin(ypr.yaw/2);
        quat.qz = cos(ypr.roll/2) * cos(ypr.pitch/2) * sin(ypr.yaw/2) - sin(ypr.roll/2) * sin(ypr.pitch/2) * cos(ypr.yaw/2);
        quat.qw = cos(ypr.roll/2) * cos(ypr.pitch/2) * cos(ypr.yaw/2) + sin(ypr.roll/2) * sin(ypr.pitch/2) * sin(ypr.yaw/2);
        return quat;
    };
    
    void resetOrientation() {
        // TODO: setup reset logic here
        // Ideally use last quat values
    };
};

/// Class for collecting devices
class SerialDeviceInfo {
public:
    std::string devicePath;
    std::string deviceName;
    // index for device, connection state for device
    int deviceID, deviceState;
    
    SerialDeviceInfo(std::string devicePathIn, std::string deviceNameIn, int deviceIDIn, int deviceStateIn){
        devicePath = devicePathIn;
        deviceName = deviceNameIn;
        deviceID = deviceIDIn;
        deviceState = deviceStateIn;
    }
    SerialDeviceInfo(){
        deviceName = "device undefined";
        deviceID = -1;
        deviceState = -1;
    }
    std::string getDevicePath(){
        return devicePath;
    }
    std::string getDeviceName(){
        return deviceName;
    }
    int getDeviceID(){
        return deviceID;
    }
    int getDeviceState(){
        return deviceState;
    }
};

struct M1OrientationManagerClientConnection {
    int port;
    juce::int64 time;
};

class M1OrientationManagerOSCServerBase : 
    private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>, 
    public M1OrientationManagerOSCSettings
{
    juce::OSCReceiver receiver;
    std::vector<M1OrientationManagerClientConnection> clients;
    int serverPort = 0;

    std::function<void(const juce::OSCMessage& message)> callback = nullptr;

    void oscMessageReceived(const juce::OSCMessage& message) override;
    void send(const std::vector<M1OrientationManagerClientConnection>& clients, std::string str);
    void send(const std::vector<M1OrientationManagerClientConnection>& clients, juce::OSCMessage& msg);

    void send_getDevices(const std::vector<M1OrientationManagerClientConnection>& clients);
    void send_getCurrentDevice(const std::vector<M1OrientationManagerClientConnection>& clients);
    void send_getTrackingYaw(const std::vector<M1OrientationManagerClientConnection>& clients);
    void send_getTrackingPitch(const std::vector<M1OrientationManagerClientConnection>& clients);
    void send_getTrackingRoll(const std::vector<M1OrientationManagerClientConnection>& clients);
    void send_getTracking(const std::vector<M1OrientationManagerClientConnection>& clients);

public:
    
    virtual ~M1OrientationManagerOSCServerBase();

    bool init(int serverPort);

    void setOrientation(float yaw, float pitch, float roll);

    std::vector<M1OrientationManagerClientConnection> getClients();

    // need to override
    virtual void update() = 0;
 
    virtual void setTrackingYaw(bool enable) = 0;
    virtual void setTrackingPitch(bool enable) = 0;
    virtual void setTrackingRoll(bool enable) = 0;

    virtual bool getTrackingYaw() = 0;
    virtual bool getTrackingPitch() = 0;
    virtual bool getTrackingRoll() = 0;

    virtual void refreshDevices() = 0;
    // TODO: make these functions work by unique ID and string?
    virtual std::vector<std::string> getDevices() = 0;
    virtual std::string getCurrentDevice() = 0;
    virtual void selectDevice(std::string device) = 0;

    virtual void setTracking(bool enable) = 0;
    virtual bool getTracking() = 0;

    void command_refreshDevices();
    void command_selectDevice(std::string device);
    void command_setTracking(bool enable);
    void command_setTrackingYaw(bool enable);
    void command_setTrackingPitch(bool enable);
    void command_setTrackingRoll(bool enable);

    void setCallback(std::function<void(const juce::OSCMessage& message)> callback);
    void close();
};
