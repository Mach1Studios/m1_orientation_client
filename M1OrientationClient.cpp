#include "M1OrientationClient.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "libs/json/single_include/nlohmann/json.hpp"

void M1OrientationClient::oscMessageReceived(const juce::OSCMessage& message) {
    if (message.getAddressPattern() == "/m1-helper-port-changed") {
        if (message.size() >= 1 && message[0].isInt32()) {
            int newHelperPort = message[0].getInt32();
            DBG("[M1OrientationClient] Helper port changed to: " + std::to_string(newHelperPort));
            
            // Update our stored helper port
            helperPort = newHelperPort;
            
            // Reconnect with the new port
            helperInterface.disconnect();
            helperInterface.connect("127.0.0.1", helperPort);
        }
    }
}

bool M1OrientationClient::send(std::string path, std::string data)
{
    httplib::Client client("localhost", serverPort);
    // The previous 10ms timeouts made commands such as /startTrackingUsingDevice
    // fail silently whenever the orientation manager took more than 10ms to
    // accept a connection (e.g. while busy connecting to a device).
    client.set_connection_timeout(0, 250000); // 250ms
    client.set_read_timeout(0, 500000); // 500ms
    client.set_write_timeout(0, 500000); // 500ms

    auto res = client.Post(path, data, "text/plain");
    if (res)
        return true;

    // A read timeout means the server accepted the command but is still busy
    // processing it (e.g. a slow device connect); only report real delivery
    // failures so the UI is not shown spurious errors.
    const auto err = res.error();
    const bool deliveryFailed = (err == httplib::Error::Connection
                                 || err == httplib::Error::ConnectionTimeout
                                 || err == httplib::Error::Write);
    if (deliveryFailed)
    {
        DBG("[M1OrientationClient] Failed to deliver command: " + path);
        if (statusCallback)
        {
            statusCallback(false, "Could not reach the orientation manager (" + path + ")", "", 0, "");
        }
        return false;
    }
    return true;
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
            // Hiding UI error by default
            // TODO: handle this more elegantly
//            juce::AlertWindow::showMessageBoxAsync(
//                juce::AlertWindow::NoIcon,
//                "Warning",
//                "Mach1: settings.json file doesn't exist in Mach1's Application Support directory, please reinstall the Spatial System",
//                "",
//                nullptr,
//                juce::ModalCallbackFunction::create(([&](int result) {
//                   //juce::JUCEApplicationBase::quit();
//                }))
//            );
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
        // Keep the ping tolerant of a busy server. With the previous 10ms
        // timeouts and 3-strike limit, ~90ms of server busyness was enough to
        // flip clients to "disconnected", which in turn made them ask the
        // helper to (re)start the orientation manager - dropping any
        // connected device in the process.
        client.set_connection_timeout(0, 250000); // 250ms
        client.set_read_timeout(0, 250000); // 250ms
        client.set_write_timeout(0, 250000); // 250ms

        int failedRequestCount = 0;

        while (isRunning) {
            auto res = client.Get("/ping");
            if (res && res->body != "") {
                failedRequestCount = 0;  // Reset counter on successful request
                setConnectedToServer(true);
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
                        Mach1::Float3 incomingRot = {raw_rot[0], raw_rot[1], raw_rot[2]};
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
                }
            }
            else {
                failedRequestCount++;
                if (failedRequestCount >= MAX_FAILED_REQUESTS) {
                    setConnectedToServer(false);
                }
            }
            
            if (this->helperPort != 0) {
                if (!isConnectedToServer()) {
                    juce::OSCMessage clientRequestsServerMessage = juce::OSCMessage(juce::OSCAddressPattern("/m1-clientRequestsServer"));
                    helperInterface.send(clientRequestsServerMessage);
                }

                juce::OSCMessage clientExistsMessage = juce::OSCMessage(juce::OSCAddressPattern("/m1-clientExists"));
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
    // Always send the request, even if this device already appears to be the
    // current one. The previous currentDevice guard made it impossible to
    // reconnect to a device the server still believed was connected (e.g.
    // after the physical connection dropped); the server owns reconnect
    // semantics now.
    send("/startTrackingUsingDevice", nlohmann::json({ device.getDeviceName(), (int)device.getDeviceType(), device.getDeviceAddress() }).dump());
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

void M1OrientationClient::setConnectedToServer(bool connected) {
    std::lock_guard<std::mutex> lock(connectionMutex);
    connectedToServer = connected;
}

bool M1OrientationClient::isConnectedToServer() {
    std::lock_guard<std::mutex> lock(connectionMutex);
    return connectedToServer;
}
