#pragma once

#include <JuceHeader.h>
#include "M1OrientationManagerOSCSettings.h"

class M1OrientationManagerOSCClient : 
    private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>, 
    public M1OrientationManagerOSCSettings
{
    juce::OSCReceiver receiver;
    int serverPort = 0;
    int clientPort = 0;
    bool connectedToServer = false;

    std::vector<std::string> devices;

    float yaw = 0;
    float pitch = 0;
    float roll = 0;

    bool bTrackingYaw = true;
    bool bTrackingPitch = true;
    bool bTrackingRoll = true;

    bool bTracking = false;

    std::string currentDevice;

    std::function<void(const juce::OSCMessage& message)> callback = nullptr;

    void oscMessageReceived(const juce::OSCMessage& message) override;

    bool send(std::string str);
    bool send(juce::OSCMessage& msg);


public:

    ~M1OrientationManagerOSCClient();

    bool init(int serverPort) override;

    float getYaw();
    float getPitch();
    float getRoll();

    void setTrackingYaw(bool enable);
    void setTrackingPitch(bool enable);
    void setTrackingRoll(bool enable);

    bool getTrackingYaw();
    bool getTrackingPitch();
    bool getTrackingRoll();

    bool isConnectedToServer();

    void setCallback(std::function<void(const juce::OSCMessage& message)> callback);
    
    void refreshDevices();
    std::vector<std::string> getDevices();
    
    std::string getCurrentDevice();
    void selectDevice(std::string device);

    void setTracking(bool enable);
    bool getTracking();

    void close();
};
