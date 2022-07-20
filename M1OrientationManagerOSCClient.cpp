#include "M1OrientationManagerOSCClient.h"

void M1OrientationManagerOSCClient::oscMessageReceived(const juce::OSCMessage& message) {
    if (message.getAddressPattern() == "/connectedToServer") {
        connectedToServer = true;
    }
    else if (message.getAddressPattern() == "/getDevices") {
        devices.clear();
        for (int i = 0; i < message.size(); i++) {
            devices.push_back(message[i].getString().toStdString());
        }
    }
    else if (message.getAddressPattern() == "/getCurrentDevice") {
        currentDevice = message[0].getString().toStdString();
    }
    else if (message.getAddressPattern() == "/yaw") {
        yaw = message[0].getFloat32();
    }
    else if (message.getAddressPattern() == "/pitch") {
        pitch = message[0].getFloat32();
    }
    else if (message.getAddressPattern() == "/roll") {
        roll = message[0].getFloat32();
    }
    else if (message.getAddressPattern() == "/getTrackingYaw") {
        bTrackingYaw = message[0].getInt32();
    }
    else if (message.getAddressPattern() == "/getTrackingPitch") {
        bTrackingPitch = message[0].getInt32();
    }
    else if (message.getAddressPattern() == "/getTrackingRoll") {
        bTrackingRoll = message[0].getInt32();
    }
    else if (message.getAddressPattern() == "/getTracking") {
        bTracking = message[0].getInt32();
    }
    else if (callback) {
        callback(message);
    }
}

bool M1OrientationManagerOSCClient::send(std::string str) {
    juce::OSCMessage msg(str.c_str());
    return send(msg);
}

bool M1OrientationManagerOSCClient::send(juce::OSCMessage& msg) {
    juce::OSCSender sender;
    if (sender.connect("127.0.0.1", serverPort)) {
        sender.send(msg);
        return true;
    }
    return false;
}

M1OrientationManagerOSCClient::~M1OrientationManagerOSCClient() {
    close();
}

float M1OrientationManagerOSCClient::getYaw() {
    return yaw;
}

float M1OrientationManagerOSCClient::getPitch() {
    return pitch;
}

float M1OrientationManagerOSCClient::getRoll() {
    return roll;
}

void M1OrientationManagerOSCClient::setTrackingYaw(bool enable) { 
    juce::OSCMessage msg("/setTrackingYaw");
    msg.addInt32(enable);
    send(msg);
}

void M1OrientationManagerOSCClient::setTrackingPitch(bool enable) {
    juce::OSCMessage msg("/setTrackingPitch");
    msg.addInt32(enable);
    send(msg);
}

void M1OrientationManagerOSCClient::setTrackingRoll(bool enable) {
    juce::OSCMessage msg("/setTrackingRoll");
    msg.addInt32(enable);
    send(msg);
}

bool M1OrientationManagerOSCClient::getTrackingYaw() { 
    return bTrackingYaw;
}

bool M1OrientationManagerOSCClient::getTrackingPitch() {
    return bTrackingPitch;
}

bool M1OrientationManagerOSCClient::getTrackingRoll() {
    return bTrackingRoll;
}

bool M1OrientationManagerOSCClient::isConnectedToServer() {
    return connectedToServer;
}

void M1OrientationManagerOSCClient::setCallback(std::function<void(const juce::OSCMessage& message)> callback) {
    this->callback = callback;
}

bool M1OrientationManagerOSCClient::init(int serverPort) {
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

            // check server is running
            juce::DatagramSocket socket(false);
            socket.setEnablePortReuse(false);
            if (socket.bindToPort(serverPort)) {
                socket.shutdown();

                // start server 
                // juce::ChildProcess().start("C:/windows/system32/notepad.exe");
            }

            // add client to server
            juce::OSCMessage msg("/addClient");
            msg.addInt32(clientPort);
            send(msg);

            return true;
        }
    }

    return false;
}

void M1OrientationManagerOSCClient::refreshDevices()
{
    send("/refreshDevices");
}

std::vector<std::string> M1OrientationManagerOSCClient::getDevices() {
    return devices;
}

std::string M1OrientationManagerOSCClient::getCurrentDevice()
{
    return currentDevice;
}

void M1OrientationManagerOSCClient::selectDevice(std::string device) {
    juce::OSCMessage msg("/selectDevice");
    msg.addString(device);
    send(msg);
}

void M1OrientationManagerOSCClient::setTracking(bool enable)
{
    juce::OSCMessage msg("/setTracking");
    msg.addInt32(enable);
    send(msg);
}

bool M1OrientationManagerOSCClient::getTracking()
{
    return bTracking;
}

void M1OrientationManagerOSCClient::close() {
    receiver.removeListener(this);
    receiver.disconnect();
}
