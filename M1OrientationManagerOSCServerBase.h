#pragma once

#include <JuceHeader.h>
#include "M1OrientationManagerOSCSettings.h"

class M1OrientationManagerOSCServerBase : 
    private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>, 
    public M1OrientationManagerOSCSettings
{
    struct M1OrientationDeviceClient {
        int port;
        juce::int64 time;
    };

    juce::OSCReceiver receiver;
    std::vector<M1OrientationDeviceClient> clients;
    int serverPort = 0;

    std::function<void(const juce::OSCMessage& message)> callback = nullptr;

    void oscMessageReceived(const juce::OSCMessage& message) override;
    void send(const std::vector<M1OrientationDeviceClient>& clients, std::string str);
    void send(const std::vector<M1OrientationDeviceClient>& clients, juce::OSCMessage& msg);

    void _getDevices(const std::vector<M1OrientationDeviceClient>& clients);
    void _getCurrentDevice(const std::vector<M1OrientationDeviceClient>& clients);
    void _getTrackingYaw(const std::vector<M1OrientationDeviceClient>& clients);
    void _getTrackingPitch(const std::vector<M1OrientationDeviceClient>& clients);
    void _getTrackingRoll(const std::vector<M1OrientationDeviceClient>& clients);
    void _getTracking(const std::vector<M1OrientationDeviceClient>& clients);

public:
    virtual ~M1OrientationManagerOSCServerBase();

    bool init(int serverPort);

    void setOrientation(float yaw, float pitch, float roll);

    int getClientsCount();

    // need to override
    virtual void update() = 0;
 
    virtual void setTrackingYaw(bool enable) = 0;
    virtual void setTrackingPitch(bool enable) = 0;
    virtual void setTrackingRoll(bool enable) = 0;

    virtual bool getTrackingYaw() = 0;
    virtual bool getTrackingPitch() = 0;
    virtual bool getTrackingRoll() = 0;

    virtual void refreshDevices() = 0;
    virtual std::vector<std::string> getDevices() = 0;

    virtual std::string getCurrentDevice() = 0;
    virtual void selectDevice(std::string device) = 0;

    virtual void setTracking(bool enable) = 0;
    virtual bool getTracking() = 0;

    void setCallback(std::function<void(const juce::OSCMessage& message)> callback);
    void close();
};
