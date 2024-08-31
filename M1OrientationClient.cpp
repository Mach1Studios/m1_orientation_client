#include "M1OrientationClient.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "libs/json/single_include/nlohmann/json.hpp"

void M1OrientationClient::oscMessageReceived(const juce::OSCMessage& message) {
}

void M1OrientationClient::send(std::string path, std::string data)
{
    httplib::Client client("localhost", serverPort);
    time_t usec = 10000; // 10ms
    client.set_connection_timeout(0, usec);
    client.set_read_timeout(0, usec);
    client.set_write_timeout(0, usec);
    client.Post(path, data, "text/plain");
}
    
void M1OrientationClient::command_setTrackingYawEnabled(bool enable) {
    send("/setTrackingYawEnabled", nlohmann::json({ enable }).dump());
}

void M1OrientationClient::command_setTrackingPitchEnabled(bool enable) {
    send("/setTrackingPitchEnabled", nlohmann::json({ enable }).dump());
}

void M1OrientationClient::command_setTrackingRollEnabled(bool enable) {
    send("/setTrackingRollEnabled", nlohmann::json({ enable }).dump());
}

void M1OrientationClient::command_setTrackingYawInverted(bool invert) {
    send("/setTrackingYawInverted", nlohmann::json({ invert }).dump());
}

void M1OrientationClient::command_setTrackingPitchInverted(bool invert) {
    send("/setTrackingPitchInverted", nlohmann::json({ invert }).dump());
}

void M1OrientationClient::command_setTrackingRollInverted(bool invert) {
    send("/setTrackingRollInverted", nlohmann::json({ invert }).dump());
}

void M1OrientationClient::command_setAdditionalDeviceSettings(std::string additional_settings) {
    send("/setDeviceSettings", nlohmann::json({ additional_settings }).dump());
}

void M1OrientationClient::command_setPlayerFrameRate(float playerFrameRate) {
    send("/setPlayerFrameRate", nlohmann::json({ playerFrameRate }).dump());
}

void M1OrientationClient::command_setPlayerPositionInSeconds(float playerPlayheadPositionInSeconds) {
    send("/setPlayerPosition", nlohmann::json({ playerPlayheadPositionInSeconds }).dump());
}

void M1OrientationClient::command_setPlayerIsPlaying(bool playerIsPlaying) {
    send("/setPlayerIsPlaying", nlohmann::json({ playerIsPlaying }).dump());
}

void M1OrientationClient::command_recenter() {
    send("/recenter", "");
}

Mach1::Orientation M1OrientationClient::getOrientation() {
    return m_orientation;
}

bool M1OrientationClient::getTrackingYawEnabled() {
    return bTrackingYawEnabled;
}

bool M1OrientationClient::getTrackingPitchEnabled() {
    return bTrackingPitchEnabled;
}

bool M1OrientationClient::getTrackingRollEnabled() {
    return bTrackingRollEnabled;
}

bool M1OrientationClient::getTrackingYawInverted() {
    return bTrackingYawInverted;
}

bool M1OrientationClient::getTrackingPitchInverted() {
    return bTrackingPitchInverted;
}

bool M1OrientationClient::getTrackingRollInverted() {
    return bTrackingRollInverted;
}

float M1OrientationClient::getPlayerPositionInSeconds() {
    return playerPositionInSeconds;
}

bool M1OrientationClient::getPlayerIsPlaying() {
    return playerIsPlaying;
}

float M1OrientationClient::getPlayerLastUpdate() {
    return playerLastUpdate;
}

bool M1OrientationClient::isConnectedToServer() {
    return connectedToServer;
}

int M1OrientationClient::getServerPort() {
    return serverPort;
}

int M1OrientationClient::getHelperPort() {
    return helperPort;
}

void M1OrientationClient::setClientType(std::string client_type = "") {
    // sets the client type for unique client behaviors
    // Warning: Must be set before the init() call
    clientType = client_type;
}

std::string M1OrientationClient::getClientType() {
    return clientType;
}

void M1OrientationClient::setStatusCallback(std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> callback)
{
    this->statusCallback = callback;
}

bool M1OrientationClient::init(int serverPort, int helperPort) {
    // TODO: Add UI feedback for this process to stop user from selecting another device during connection
    
    // Using `currentApplicationFile` to be safe for both plugins and apps on all OS targets
    //juce::File pluginExe = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    //juce::File appDirectory = pluginExe.getParentDirectory();
    
    // Using common support files installation location
    juce::File m1SupportDirectory = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory);
    
    juce::File settingsFile;
    if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
        // test for any mac OS
        settingsFile = m1SupportDirectory.getChildFile("Application Support").getChildFile("Mach1");
    } else if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::Windows) != 0) {
        // test for any windows OS
        settingsFile = m1SupportDirectory.getChildFile("Mach1");
    } else {
        settingsFile = m1SupportDirectory.getChildFile("Mach1");
    }
    settingsFile = settingsFile.getChildFile("settings.json");
    
    DBG("Opening settings file: " + settingsFile.getFullPathName().quoted());
    if (settingsFile.exists()) {
        // Found the settings.json
        juce::var mainVar = juce::JSON::parse(juce::File(settingsFile));
        this->serverPort = mainVar["serverPort"];
        this->helperPort = mainVar["helperPort"];
    } else {
        if (!settingsFile.exists()) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::NoIcon,
                "Warning",
                "Mach1: settings.json file doesn't exist in Mach1's Application Support directory, please reinstall the Spatial System",
                "",
                nullptr,
                juce::ModalCallbackFunction::create(([&](int result) {
                   //juce::JUCEApplicationBase::quit();
                }))
            );
            return false;
        }
    }
    
    // This is for a service handling the orientation manager if the helper port is discovered
    if (this->helperPort != 0) {
        helperInterface.connect("127.0.0.1", this->helperPort);
    }
    
    isRunning = true;

    std::thread([&, this]() {
        httplib::Client client("localhost", this->serverPort);
        time_t usec = 10000; // 10ms
        client.set_connection_timeout(0, usec);
        client.set_read_timeout(0, usec);
        client.set_write_timeout(0, usec);

        while (isRunning) {
            auto res = client.Get("/ping");
            if (res) {
                std::string body = res->body;
                if (body != "") {
                    auto j = nlohmann::json::parse(body);

                    std::vector<M1OrientationDeviceInfo> devices;
                    for (int i = 0; i < j["devices"].size(); i++) {
                        auto m = j["devices"][i];

                        std::string deviceName = m.at(0);
                        enum M1OrientationDeviceType deviceType = (enum M1OrientationDeviceType)m.at(1);
                        std::string deviceAddress = m.at(2);
                        bool hasStrength = m.at(3);
                        int deviceStrength = m.at(4);

                        devices.push_back(M1OrientationDeviceInfo(deviceName, deviceType, deviceAddress, hasStrength ? deviceStrength : false));
                    }

                    mutex.lock();
                    this->devices = devices;
                    int currentDeviceIdx = j["currentDeviceIdx"];
                    if (currentDeviceIdx >= 0) {
                        currentDevice = devices[currentDeviceIdx];
                    }
                    else {
                        currentDevice = M1OrientationDeviceInfo();
                    }
                    mutex.unlock();

					if (j["orientation"].size() == 3) {
                        auto raw_rot = j["orientation"];
                        Mach1::Float3 incomingRot = {raw_rot[1], raw_rot[0], raw_rot[2]};
						m_orientation.SetRotation(incomingRot.Map(-1, 1, -PI, PI));
					}
					else if (j["orientation"].size() == 4) {
						// quat input
                        auto raw_quat = j["orientation"];
						m_orientation.SetRotation({ raw_quat[0], raw_quat[1], raw_quat[2], raw_quat[3] });
					}

                    bTrackingYawEnabled = j["trackingEnabled"][0];
                    bTrackingPitchEnabled = j["trackingEnabled"][1];
                    bTrackingRollEnabled = j["trackingEnabled"][2];
                    bTrackingYawInverted = j["trackingInverted"][0];
                    bTrackingPitchInverted = j["trackingInverted"][1];
                    bTrackingRollInverted = j["trackingInverted"][2];

                    playerFrameRate = j["player"]["frameRate"];
                    playerPositionInSeconds = j["player"]["positionInSeconds"];
                    playerIsPlaying = j["player"]["isPlaying"];
                    playerLastUpdate = j["player"]["lastUpdate"];

                    connectedToServer = true;
                }
            }
            else {
                connectedToServer = false;
            }
            
            if (this->helperPort != 0) {
                if (!connectedToServer) {
                    juce::OSCMessage clientRequestsServerMessage = juce::OSCMessage(juce::OSCAddressPattern("/clientRequestsServer"));
                    helperInterface.send(clientRequestsServerMessage);
                }

                juce::OSCMessage clientExistsMessage = juce::OSCMessage(juce::OSCAddressPattern("/clientExists"));
                helperInterface.send(clientExistsMessage);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }

    }).detach();

    return true;
}

void M1OrientationClient::command_refresh()
{
    send("/devicesrefresh", "");
}

std::vector<M1OrientationDeviceInfo> M1OrientationClient::getDevices() {
    mutex.lock();
    std::vector<M1OrientationDeviceInfo> devices = this->devices;
    mutex.unlock();
    return devices;
}

M1OrientationDeviceInfo M1OrientationClient::getCurrentDevice() {
    mutex.lock();
    M1OrientationDeviceInfo currentDevice = this->currentDevice;
    mutex.unlock();
    return currentDevice;
}

void M1OrientationClient::command_startTrackingUsingDevice(M1OrientationDeviceInfo device) {
    mutex.lock();
    if (currentDevice != device) {
        send("/startTrackingUsingDevice", nlohmann::json({ device.getDeviceName(), (int)device.getDeviceType(), device.getDeviceAddress() }).dump());
    }
    mutex.unlock();
}

void M1OrientationClient::command_disconnect()
{
    send("/disconnect", "");
}

void M1OrientationClient::close() {
    isRunning = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

M1OrientationClient::~M1OrientationClient() {
    close();
}
