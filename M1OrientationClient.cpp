#include "M1OrientationClient.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "nlohmann/json.hpp"

void M1OrientationClient::oscMessageReceived(const juce::OSCMessage& message) {
	/*
    else if (message.getAddressPattern() == "/getStatus") {
        bool success = message[0].getInt32();
        std::string text = message[1].getString().toStdString();
        std::string connectedDeviceName = message[2].getString().toStdString();
        int connectedDeviceType = message[3].getInt32();
        std::string connectedDeviceAddress = message[4].getString().toStdString();

        // TODO: Should device be found from vector of devices instead of this?
        if (statusCallback) {
            statusCallback(success, text, connectedDeviceName, connectedDeviceType, connectedDeviceAddress);
            currentDevice = { connectedDeviceName, (enum M1OrientationDeviceType)connectedDeviceType, connectedDeviceAddress };
        }
    }
    else if (message.getAddressPattern() == "/client-active") {
        // used to mark a client as active and expose it to the object initializing this client
        client_active = (bool)message[0].getInt32();
    }
    else if (message.getAddressPattern() == "/getTimecode") {
        // Retrieve current playhead position as timecode
        if (message.size() == 4) {
            HH = message[0].getInt32();
            MM = message[1].getInt32();
            SS = message[2].getInt32();
            FS = message[3].getInt32();
            currentPlayheadPositionInSeconds = (HH * 3600) + (MM * 60) + SS + (frameRate > 0 ? FS / frameRate : 0);
        } else {
            currentPlayheadPositionInSeconds = message[0].getFloat32();
        }
    } else {
        // TODO: error handling for false returns
    }
	*/
}

void M1OrientationClient::send(std::string path, std::string data)
{
	httplib::Client("localhost", serverPort).Post(path, data, "text/plain");
}

M1OrientationClient::~M1OrientationClient() {
    close();
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

void M1OrientationClient::command_setOscDevice(int new_osc_port, std::string new_osc_addr_pattrn) {
	send("/setOscDeviceSettings", nlohmann::json({ new_osc_port, new_osc_addr_pattrn }).dump());
}

// TODO: refactor this out
void M1OrientationClient::command_setMonitoringMode(int mode = 0) {
    // It is expected to send the orientation to the monitor, let the monitor process its orientation and return it here for reporting to other plugin instances
	send("/setMonitoringMode", nlohmann::json({ mode }).dump());
}

void M1OrientationClient::command_setOffsetYPR(int client_id = 0, float yaw = 0, float pitch = 0, float roll = 0) {
    // Use this to instruct a client to add its orientation for calculation in another client
    // Master orientation of all clients should be calculated externally
	send("/setOffsetYPR", nlohmann::json({ client_id, yaw, pitch ,roll }).dump());
}

void M1OrientationClient::command_setMasterYPR(float yaw = 0, float pitch = 0, float roll = 0) {
    // Use this to set the final calculated YPR that can be used for registered plugins GUI systems
    // Note: Expects an absolute YPR orientation
	send("/setMasterYPR", nlohmann::json({ yaw, pitch ,roll }).dump());
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

Orientation M1OrientationClient::getOrientation() {
    return orientation;
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

bool M1OrientationClient::init(int serverPort, int watcherPort, bool useWatcher = false) {
    // TODO: Add UI feedback for this process to stop user from selecting another device during connection
    
    // Using `currentApplicationFile` to be safe for both plugins and apps on all OS targets
    //juce::File pluginExe = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    //juce::File appDirectory = pluginExe.getParentDirectory();
    
    // Using common support files installation location
    juce::File m1SupportDirectory = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory);

    // check server is running
    if (useWatcher) {
        juce::DatagramSocket socket(false);
        socket.setEnablePortReuse(false);
        if (socket.bindToPort(watcherPort)) {
            socket.shutdown();
            
            // TODO: Send signal to watcher here
        }
    }
 
	this->serverPort = serverPort;

	isRunning = true;

	std::thread([&]() {
		httplib::Client client("localhost", serverPort);

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
					mutex.unlock();

					if (j["orientation"].size() == 3) {
						M1OrientationYPR incomingOrientation;
						incomingOrientation.angleType = M1OrientationYPR::SIGNED_NORMALLED;
						incomingOrientation.yaw = j["orientation"][0];
						incomingOrientation.pitch = j["orientation"][1];
						incomingOrientation.roll = j["orientation"][2];
						orientation.setYPR(incomingOrientation);
					}
					else if (j["orientation"].size() == 4) {
						// quat input
						orientation.setQuat({ j["orientation"][0], j["orientation"][1], j["orientation"][2], j["orientation"][3] });
					}

					bTrackingYawEnabled = j["trackingEnabled"][0];
					bTrackingPitchEnabled = j["trackingEnabled"][1];
					bTrackingRollEnabled = j["trackingEnabled"][2];

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
            
            // TODO: inform watcher that we're here
            
            if (!connectedToServer) {
                // TODO: send message to watcher that we need manager
            }

			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		}

	}).detach();

    return true;
}

void M1OrientationClient::command_disconnect()
{
	send("/disconnect", "");
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

void M1OrientationClient::close() {
    // Send a message to remove the client from server list
	/*
    juce::OSCMessage msg("/removeClient");
    msg.addInt32(this->clientPort);
    send(msg);
    */

	isRunning = false;

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

    receiver.removeListener(this);
    receiver.disconnect();
}
