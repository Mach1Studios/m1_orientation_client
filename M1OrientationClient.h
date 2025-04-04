#pragma once

#include <JuceHeader.h>

#include "M1OrientationTypes.h"
#include "M1OrientationSettings.h"

#include "libs/httplib/httplib.h"
#include "m1_mathematics/Orientation.h"

#ifndef PI
#define PI       3.14159265358979323846
#endif

class M1OrientationClient :
    private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>,
    public M1OrientationManagerOSCSettings
{
    std::mutex mutex;
    std::mutex connectionMutex;
    bool isRunning;
    bool connectedToServer = false;

    juce::OSCSender helperInterface;
    int helperPort = 0;
    int serverPort = 0;

    M1OrientationDeviceInfo currentDevice;
    std::vector<M1OrientationDeviceInfo> devices;

    Mach1::Orientation m_orientation;
    bool bTrackingYawEnabled = true;
    bool bTrackingPitchEnabled = true;
    bool bTrackingRollEnabled = true;
    bool bTrackingYawInverted = false;
    bool bTrackingPitchInverted = false;
    bool bTrackingRollInverted = false;

    std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> statusCallback = nullptr;

    int failedRequestCount = 0;
    static const int MAX_FAILED_REQUESTS = 3; // Adjust this value as needed

    void oscMessageReceived(const juce::OSCMessage& message) override;
	void send(std::string path, std::string data);
    
public:
    ~M1OrientationClient();

    int client_id = 0;
    bool client_active = true;
    std::string clientType = ""; // Use this to specify a client with a specific behavior

    // setup the server and watcher connections, the watcher is off by default
    bool init(int serverPort, int watcherPort) override;

    // Commands from a client to the server
    void command_startTrackingUsingDevice(M1OrientationDeviceInfo device);
    void command_disconnect();
    void command_setTrackingYawEnabled(bool enable);
    void command_setTrackingPitchEnabled(bool enable);
    void command_setTrackingRollEnabled(bool enable);
    void command_setTrackingYawInverted(bool invert);
    void command_setTrackingPitchInverted(bool invert);
    void command_setTrackingRollInverted(bool invert);
    void command_setAdditionalDeviceSettings(std::string additional_settings);
    void command_recenter();
    void command_refresh();

    // Functions from the server to the clients
    std::vector<M1OrientationDeviceInfo> getDevices();
    M1OrientationDeviceInfo getCurrentDevice();
    Mach1::Orientation getOrientation();
    bool getTrackingYawEnabled();
    bool getTrackingPitchEnabled();
    bool getTrackingRollEnabled();
    bool getTrackingYawInverted();
    bool getTrackingPitchInverted();
    bool getTrackingRollInverted();

    // Connection handling
    int getServerPort();
    int getHelperPort();
    std::string getClientType();
    void setClientType(std::string client_type);
    void setStatusCallback(std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> callback);
    void close();
    
    bool isConnectedToDevice() {
        return getCurrentDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone;
    }

    void setConnectedToServer(bool connected);
    bool isConnectedToServer();
};
