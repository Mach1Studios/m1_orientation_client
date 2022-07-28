#include "M1OrientationOSCClient.h"

void M1OrientationOSCClient::oscMessageReceived(const juce::OSCMessage& message) {
    if (message.getAddressPattern() == "/connectedToServer") {
        connectedToServer = true;
    }
    else if (message.getAddressPattern() == "/getDevices") {
        devices.clear();
        for (int i = 0; i < message.size(); i+=2) {
            devices.push_back({ message[i + 0].getString().toStdString(), (enum M1OrientationDeviceType)message[i + 1].getInt32() });
        }
    }
    else if (message.getAddressPattern() == "/getCurrentDevice") {
        currentDevice = { message[0].getString().toStdString(), (enum M1OrientationDeviceType)message[1].getInt32() };
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
    else if (callback) {
        callback(message);
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

M1GlobalOrientation M1OrientationOSCClient::getOrientation() {
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

bool M1OrientationOSCClient::isConnectedToServer() {
    return connectedToServer;
}

void M1OrientationOSCClient::setCallback(std::function<void(const juce::OSCMessage& message)> callback) {
    this->callback = callback;
}

bool M1OrientationOSCClient::init(int serverPort) {
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

void M1OrientationOSCClient::command_refreshDevices()
{
    send("/refreshDevices");
}

std::vector<M1OrientationDevice> M1OrientationOSCClient::getDevices() {
    return devices;
}

M1OrientationDevice M1OrientationOSCClient::getCurrentDevice() {
    return currentDevice;
}

void M1OrientationOSCClient::command_startTrackingUsingDevice(M1OrientationDevice device) {
    juce::OSCMessage msg("/startTrackingUsingDevice");
    msg.addString(device.name);
    msg.addInt32(device.type);
    send(msg);
}

void M1OrientationOSCClient::close() {
    receiver.removeListener(this);
    receiver.disconnect();
}
