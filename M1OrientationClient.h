#pragma once

#include <JuceHeader.h>

#include "M1OrientationTypes.h"
#include "M1OrientationSettings.h"

#include "libs/httplib/httplib.h"

class M1OrientationClient :
    private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>,
    public M1OrientationManagerOSCSettings
{
    std::mutex mutex;
    bool isRunning;

    juce::OSCSender helperInterface;
    int helperPort = 0;
    int serverPort = 0;

    M1OrientationDeviceInfo currentDevice;
    std::vector<M1OrientationDeviceInfo> devices;

    Orientation orientation;
    bool bTrackingYawEnabled = true;
    bool bTrackingPitchEnabled = true;
    bool bTrackingRollEnabled = true;
    
    float playerPositionInSeconds = 0;
    float playerFrameRate = 0;
    bool playerIsPlaying = false;
    int playerLastUpdate = 0;
    //int HH, MM, SS, FS;

    std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> statusCallback = nullptr;

    void oscMessageReceived(const juce::OSCMessage& message) override;
	void send(std::string path, std::string data);
    
    bool isConnectedToDevice() {
        return getCurrentDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone;
    }

public:
    ~M1OrientationClient();

    int client_id = 0;
    bool client_active = true;
    std::string clientType = ""; // Use this to specify a client with a specific behavior
    bool connectedToServer = false;

    // setup the server and watcher connections, the watcher is off by default
    bool init(int serverPort, int watcherPort) override;

    // Commands from a client to the server
    void command_startTrackingUsingDevice(M1OrientationDeviceInfo device);
    void command_disconnect();
    void command_setTrackingYawEnabled(bool enable);
    void command_setTrackingPitchEnabled(bool enable);
    void command_setTrackingRollEnabled(bool enable);
    void command_setAdditionalDeviceSettings(std::string additional_settings);
    void command_setPlayerFrameRate(float frameRate);
    void command_setPlayerPositionInSeconds(float playheadPositionInSeconds);
    void command_setPlayerIsPlaying(bool isPlaying);
    void command_recenter();
    void command_refresh();

    // Functions from the server to the clients
    std::vector<M1OrientationDeviceInfo> getDevices();
    M1OrientationDeviceInfo getCurrentDevice();
    Orientation getOrientation();
    bool getTrackingYawEnabled();
    bool getTrackingPitchEnabled();
    bool getTrackingRollEnabled();
    
    // Master Timecode and Playhead position
    float getPlayerPositionInSeconds();
    bool getPlayerIsPlaying();
    float getPlayerLastUpdate();

    // Connection handling
    bool isConnectedToServer();
    int getServerPort();
    int getHelperPort();
    std::string getClientType();
    void setClientType(std::string client_type);
    void setStatusCallback(std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> callback);
    void close();
};
