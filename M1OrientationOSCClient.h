#pragma once

#include <JuceHeader.h>
#include "M1OrientationTypes.h"
#include "M1OrientationSettings.h"

class M1OrientationOSCClient : 
    private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>, 
    public M1OrientationManagerOSCSettings
{
    juce::OSCReceiver receiver;
    int serverPort = 0;
    int clientPort = 0;
    bool connectedToServer = false;

    M1OrientationDevice currentDevice;
    std::vector<M1OrientationDevice> devices;

    Orientation orientation;

    bool bTrackingYawEnabled = true;
    bool bTrackingPitchEnabled = true;
    bool bTrackingRollEnabled = true;

    bool bTracking = false;


    std::function<void(bool success, std::string message)> statusCallback = nullptr;

    void oscMessageReceived(const juce::OSCMessage& message) override;

    bool send(std::string str);
    bool send(juce::OSCMessage& msg);

public:

    ~M1OrientationOSCClient();

    bool init(int serverPort) override;

    void command_refreshDevices();
    void command_startTrackingUsingDevice(M1OrientationDevice device);
    void command_setTrackingYawEnabled(bool enable);
    void command_setTrackingPitchEnabled(bool enable);
    void command_setTrackingRollEnabled(bool enable);

    Orientation getOrientation();

    bool getTrackingYawEnabled();
    bool getTrackingPitchEnabled();
    bool getTrackingRollEnabled();

    bool isConnectedToServer();

    void setStatusCallback(std::function<void(bool success, std::string message)> callback);

    std::vector<M1OrientationDevice> getDevices();
    M1OrientationDevice getCurrentDevice();

    void close();
};
