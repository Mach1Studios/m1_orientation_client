#include "M1OrientationOSCClient.h"

#include <iostream>
#include <thread>
#include <chrono>

void M1OrientationOSCClient::oscMessageReceived(const juce::OSCMessage& message) {
    if (message.getAddressPattern() == "/connectedToServer") {
        connectedToServer = true;
    }
    else if (message.getAddressPattern() == "/getDevices") {
        devices.clear();

        int i = 0;
        while(i < message.size()) {
            std::string deviceName = message[i++].getString().toStdString();
            enum M1OrientationDeviceType deviceType = (enum M1OrientationDeviceType)message[i++].getInt32();
            std::string deviceAddress = message[i++].getString().toStdString();
            bool hasStrength = (message[i++].getInt32() == 1);
            int deviceStrength = deviceStrength = message[i++].getInt32();

            devices.push_back(M1OrientationDeviceInfo(deviceName, deviceType, deviceAddress, hasStrength ? deviceStrength : false));
        }
    }
    else if (message.getAddressPattern() == "/getCurrentDevice") {
        // TODO: Should device be found from vector of devices instead of this?
        currentDevice = { message[0].getString().toStdString(), (enum M1OrientationDeviceType)message[1].getInt32(), message[2].getString().toStdString() };
    }
    else if (message.getAddressPattern() == "/getOrientation") {
        orientation.setYPR({ message[0].getFloat32(), message[1].getFloat32(), message[2].getFloat32() });
    }
    else if (message.getAddressPattern() == "/getTrackingYawEnabled") {
        bTrackingYawEnabled = message[0].getInt32();
    }
    else if (message.getAddressPattern() == "/getTrackingPitchEnabled") {
        bTrackingPitchEnabled = message[0].getInt32();
    }
    else if (message.getAddressPattern() == "/getTrackingRollEnabled") {
        bTrackingRollEnabled = message[0].getInt32();
    }
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
    // Playhead Timecode
    else if (message.getAddressPattern() == "/getFramerate") {
        frameRate = message[0].getFloat32();
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
}

bool M1OrientationOSCClient::send(std::string str) {
    juce::OSCMessage msg(str.c_str());
    return send(msg);
}

bool M1OrientationOSCClient::send(juce::OSCMessage& msg) {
    juce::OSCSender sender;
    if (sender.connect("127.0.0.1", serverPort)) {
        sender.send(msg);
        return true;
    } else {
        // if this send returns false, check for reconnection state
        // TODO: This is an error, if we are sending messages but missing the server we should try to reconnect here

    }

    if (!connectedToServer) {
        // TODO: This is an error, if we are sending messages but missing the server we should try to reconnect here
    }
    return false;
}

M1OrientationOSCClient::~M1OrientationOSCClient() {
    close();
}

void M1OrientationOSCClient::command_setTrackingYawEnabled(bool enable) {
    juce::OSCMessage msg("/setTrackingYawEnabled");
    msg.addInt32(enable);
    send(msg);
}

void M1OrientationOSCClient::command_setTrackingPitchEnabled(bool enable) {
    juce::OSCMessage msg("/setTrackingPitchEnabled");
    msg.addInt32(enable);
    send(msg);
}

void M1OrientationOSCClient::command_setTrackingRollEnabled(bool enable) {
    juce::OSCMessage msg("/setTrackingRollEnabled");
    msg.addInt32(enable);
    send(msg);
}

void M1OrientationOSCClient::command_setFrameRate(float frameRate) {
    juce::OSCMessage msg("/setFrameRate");
    msg.addFloat32(frameRate);
    send(msg);
}

void M1OrientationOSCClient::command_setPlayheadPositionInSeconds(float playheadPositionInSeconds) {
    juce::OSCMessage msg("/setPlayheadPosition");
    msg.addFloat32(playheadPositionInSeconds);
    send(msg);
}

void M1OrientationOSCClient::command_recenter() {
    send("/disconnect");
}

Orientation M1OrientationOSCClient::getOrientation() {
    return orientation;
}

bool M1OrientationOSCClient::getTrackingYawEnabled() {
    return bTrackingYawEnabled;
}

bool M1OrientationOSCClient::getTrackingPitchEnabled() {
    return bTrackingPitchEnabled;
}

bool M1OrientationOSCClient::getTrackingRollEnabled() {
    return bTrackingRollEnabled;
}

float M1OrientationOSCClient::getPlayheadPositionInSeconds() {
    return currentPlayheadPositionInSeconds;
}

bool M1OrientationOSCClient::isConnectedToServer() {
    return connectedToServer;
}

void M1OrientationOSCClient::setStatusCallback(std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> callback)
{
    this->statusCallback = callback;
}

bool M1OrientationOSCClient::init(int serverPort, int watcherPort) {
    // TODO: Add UI feedback for this process to stop user from selecting another device during connection
	// check server is running
	{
		juce::DatagramSocket socket(false);
		socket.setEnablePortReuse(false);
		if (socket.bindToPort(watcherPort)) {
			socket.shutdown();
            // run process M1-OrientationManager-Wacther from the same folder
            juce::File executableFile = juce::File::getSpecialLocation(juce::File::invokedExecutableFile);

#ifdef JUCE_WINDOWS
            juce::File appDirectory = executableFile.getParentDirectory();
            juce::File exeFile = appDirectory.getChildFile("M1-OrientationManager-Watcher.exe");
#else
            juce::File appDirectory = executableFile.getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory();
            juce::File exeFile = appDirectory.getChildFile("M1-OrientationManager-Watcher");
#endif
            DBG("Starting M1-OrientationManager-Watcher...");
            DBG(exeFile.getFullPathName());
            watcherProcess.start(exeFile.getFullPathName());
		}
	}

    // choose random port
    int port = 4000;
    for (int i = 0; i < 100; i++) {
        juce::DatagramSocket socket(false);
        socket.setEnablePortReuse(false);
        if (socket.bindToPort(port + i)) {
            socket.shutdown();

            receiver.connect(port + i);
            receiver.addListener(this);

            this->clientPort = port + i;
            this->serverPort = serverPort;

            std::thread([&]() {
                while (true) {
                    if (!connectedToServer) {
                        // add client to server
                        juce::OSCMessage msg("/addClient");
                        msg.addInt32(this->clientPort);
                        send(msg);
                    } else {
                        // check connection
                        juce::DatagramSocket socket(false);
                        socket.setEnablePortReuse(false);
                        if (socket.bindToPort(this->serverPort)) {
                            socket.shutdown();
                            connectedToServer = false;
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }).detach();
            return true;
        }
    }
    return false;
}

void M1OrientationOSCClient::command_refreshDevices()
{
    if (connectedToServer) {
        send("/refreshDevices");
    } else {
        // TODO: figure out reconnecting to server?
        //m1OrientationOSCClient.init(6345);
    }
}

void M1OrientationOSCClient::command_disconnect()
{
    send("/disconnect");
}

std::vector<M1OrientationDeviceInfo> M1OrientationOSCClient::getDevices() {
    return devices;
}

M1OrientationDeviceInfo M1OrientationOSCClient::getCurrentDevice() {
    return currentDevice;
}

void M1OrientationOSCClient::command_startTrackingUsingDevice(M1OrientationDeviceInfo device) {
    if (currentDevice != device) {
        juce::OSCMessage msg("/startTrackingUsingDevice");
        msg.addString(device.getDeviceName());
        msg.addInt32(device.getDeviceType());
        msg.addString(device.getDeviceAddress());
        if (send(msg)) {
            currentDevice = device;
        } else {
            // TODO: ERROR: did not finish message to server
        }
    } else {
        // already connected to this device
    }
}

void M1OrientationOSCClient::close() {
    // Send a message to remove the client from server list
    juce::OSCMessage msg("/removeClient");
    msg.addInt32(this->clientPort);
    send(msg);
    
    receiver.removeListener(this);
    receiver.disconnect();
}
