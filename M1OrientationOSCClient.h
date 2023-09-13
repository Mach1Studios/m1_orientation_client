#pragma once

#include <JuceHeader.h>

#include "M1OrientationTypes.h"
#include "M1OrientationSettings.h"

class M1OrientationOSCClient :
    private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>,
    private juce::Timer,
    public M1OrientationManagerOSCSettings
{
    juce::OSCReceiver receiver;
    int serverPort = 0;
    int clientPort = 0;
    bool connectedToServer = false;
    void startOrientationManager();
    void killProcessByName(const char *name);
    int activeClientCounter = 1; // default with 1 so we do not auto shutdown on launch
    juce::int64 watchDogPingTime = 0;
    // run process m1-orientationmanager.exe from the same folder
    juce::ChildProcess orientationManagerProcess;

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

    // setup the server and watcher connections, the watcher is off by default
    bool init(int serverPort) override;

    // Commands from a client to the server
    void command_refreshDevices();
    void command_startTrackingUsingDevice(M1OrientationDeviceInfo device);
    void command_disconnect();
    void command_setTrackingYawEnabled(bool enable);
    void command_setTrackingPitchEnabled(bool enable);
    void command_setTrackingRollEnabled(bool enable);
    void command_updateOscDevice(int port, std::string addr_pttrn);
    void command_setMonitorYPR(int mode, float yaw, float pitch, float roll);
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
    
    void timerCallback() override {
        // TODO: use service to analyze the std--out//std--err on the daemon and remove this watchdog

        juce::int64 currentTime = juce::Time::currentTimeMillis();

        // TIMER 0 = m1-orientationmanager ping timer
        // this is used to blindly check if the m1-orientationmanager has crashed and attempt to relaunch it
        
        // pings and keeps m1-orientationmanager to restart orientation server if there is an issue
        DBG("TIMER[0]: " + std::to_string(currentTime - watchDogPingTime));
        if (currentTime > watchDogPingTime && currentTime - watchDogPingTime > 1000) {
            watchDogPingTime = juce::Time::currentTimeMillis() + 15000; // push time check for 10 seconds to wait for service restart
            // restart the server
            killProcessByName("m1-orientationmanager"); // TODO: pass which orientation manager we are using
            startOrientationManager();
        }
    }
};
