#pragma once

#include <JuceHeader.h>
#include "M1OrientationManagerOSCSettings.h"

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
