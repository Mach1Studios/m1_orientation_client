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

    M1OrientationDeviceInfo currentDevice;
    std::vector<M1OrientationDeviceInfo> devices;
    int oscDevicePort;

    Orientation orientation;
    bool bTrackingYawEnabled = true;
    bool bTrackingPitchEnabled = true;
    bool bTrackingRollEnabled = true;
    bool bTracking = false;
    
    float currentPlayheadPositionInSeconds;
    float frameRate;
    int HH, MM, SS, FS;

    std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> statusCallback = nullptr;

    void oscMessageReceived(const juce::OSCMessage& message) override;

    bool send(std::string str);
    bool send(juce::OSCMessage& msg);

public:

    ~M1OrientationOSCClient();

    bool init(int serverPort) override;

	void command_refreshDevices();
	void command_startTrackingUsingDevice(M1OrientationDeviceInfo device);
	void command_disconnect();
	void command_setTrackingYawEnabled(bool enable);
    void command_setTrackingPitchEnabled(bool enable);
    void command_setTrackingRollEnabled(bool enable);
    void command_setFrameRate(float frameRate);
    void command_setPlayheadPositionInSeconds(float playheadPositionInSeconds);
    void command_setOscDevicePort(int port);

    std::vector<M1OrientationDeviceInfo> getDevices();
    M1OrientationDeviceInfo getCurrentDevice();
    
    Orientation getOrientation();
    bool getTrackingYawEnabled();
    bool getTrackingPitchEnabled();
    bool getTrackingRollEnabled();

    // Master Timecode and Playhead position
    float getPlayheadPositionInSeconds();
    
    bool isConnectedToServer();
    void setStatusCallback(std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> callback);

    void close();
};
