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

    M1OrientationDeviceInfo currentDevice;
    std::vector<M1OrientationDeviceInfo> devices;

    Orientation orientation;
    bool bTrackingYawEnabled = true;
    bool bTrackingPitchEnabled = true;
    bool bTrackingRollEnabled = true;
    
    float currentPlayheadPositionInSeconds;
    float frameRate;
    int HH, MM, SS, FS;

    std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> statusCallback = nullptr;

    void oscMessageReceived(const juce::OSCMessage& message) override;
    bool send(std::string str);
    bool send(juce::OSCMessage& msg);

public:
    ~M1OrientationOSCClient();

    int client_id = 0;
    bool connectedToServer = false;

    // setup the server and watcher connections, the watcher is off by default
    bool init(int serverPort, int watcherPort, bool useWatcher) override;

    // Commands from a client to the server
    void command_refreshDevices();
    void command_startTrackingUsingDevice(M1OrientationDeviceInfo device);
    void command_disconnect();
    void command_setTrackingYawEnabled(bool enable);
    void command_setTrackingPitchEnabled(bool enable);
    void command_setTrackingRollEnabled(bool enable);
    void command_updateOscDevice(int port, std::string addr_pttrn);
    void command_setMonitoringMode(int mode);
    void command_setOffsetYPR(int client_id, float yaw, float pitch, float roll);
    void command_setAbsMasterYPR(float yaw, float pitch, float roll);
    void command_setFrameRate(float frameRate);
    void command_setPlayheadPositionInSeconds(float playheadPositionInSeconds);
    void command_recenter();

    // Functions from the server to the clients
    std::vector<M1OrientationDeviceInfo> getDevices();
    M1OrientationDeviceInfo getCurrentDevice();
    Orientation getOrientation();
    bool getTrackingYawEnabled();
    bool getTrackingPitchEnabled();
    bool getTrackingRollEnabled();
    
    // Master Timecode and Playhead position
    float getPlayheadPositionInSeconds();
    
    // Connection handling
    bool isConnectedToServer();
    void setStatusCallback(std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> callback);
    void close();
};
