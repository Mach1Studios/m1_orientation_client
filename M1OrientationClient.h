#pragma once

#include <JuceHeader.h>

#include "M1OrientationTypes.h"
#include "M1OrientationSettings.h"

// requires WIN32_LEAN_AND_MEAN
#include "httplib.h"

class M1OrientationClient :
    private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>,
    public M1OrientationManagerOSCSettings
{
	std::mutex mutex;
	bool isRunning = false;

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
	void send(std::string path, std::string data);

public:
    ~M1OrientationClient();

    int client_id = 0;
    bool client_active = true;
    std::string clientType = ""; // Use this to specify a client with a specific behavior
	bool connectedToServer = false;

    // setup the server and watcher connections, the watcher is off by default
    bool init(int serverPort, int watcherPort, bool useWatcher) override;

    // Commands from a client to the server
    void command_startTrackingUsingDevice(M1OrientationDeviceInfo device);
    void command_disconnect();
    void command_setTrackingYawEnabled(bool enable);
    void command_setTrackingPitchEnabled(bool enable);
    void command_setTrackingRollEnabled(bool enable);
    void command_setOscDevice(int port, std::string addr_pttrn);
    void command_setMonitoringMode(int mode);
    void command_setOffsetYPR(int client_id, float yaw, float pitch, float roll);
    void command_setMasterYPR(float yaw, float pitch, float roll);
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
    int getServerPort();
    std::string getClientType();
    void setClientType(std::string client_type);
    void setStatusCallback(std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> callback);
    void close();
};
