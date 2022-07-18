#include "M1OrientationManagerOSCServerBase.h"

void M1OrientationManagerOSCServerBase::oscMessageReceived(const juce::OSCMessage& message) {
    if (message.getAddressPattern() == "/addClient") {
        // add client to clients list
        int port = message[0].getInt32();
        bool found = false;

        for (auto& client : clients) {
            if (client.port == port) {
                client.time = juce::Time::currentTimeMillis();
                found = true;
            }
        }

        if (!found) {
            M1OrientationDeviceClient client;
            client.port = port;
            client.time = juce::Time::currentTimeMillis();
            clients.push_back(client);
        }

        juce::OSCSender sender;
        if (sender.connect("127.0.0.1", port)) {
            juce::OSCMessage msg("/connectedToServer");
            sender.send(msg);
        }

        std::vector<M1OrientationDeviceClient> clients = { M1OrientationDeviceClient { port, 0 } };
        _getDevices(clients);
        _getCurrentDevice(clients);
        _getTracking(clients);
        _getTrackingYaw(clients);
        _getTrackingPitch(clients);
        _getTrackingRoll(clients);
    }
    else if (message.getAddressPattern() == "/refreshDevices") {
        refreshDevices();

        _getDevices(clients);
    }
    else if (message.getAddressPattern() == "/selectDevice") {
        std::string device = message[0].getString().toStdString();
        selectDevice(device);

        _getCurrentDevice(clients);
    }
    else if (message.getAddressPattern() == "/setTracking") {
        bool enable = message[0].getInt32();
        setTracking(enable);

        _getTracking(clients);
    }
    else if (message.getAddressPattern() == "/setTrackingYaw") {
        bool enable = message[0].getInt32();
        setTrackingYaw(enable);

        _getTrackingYaw(clients);
    }
    else if (message.getAddressPattern() == "/setTrackingPitch") {
        bool enable = message[0].getInt32();
        setTrackingPitch(enable);

        _getTrackingPitch(clients);
    }
    else if (message.getAddressPattern() == "/setTrackingRoll") {
        bool enable = message[0].getInt32();
        setTrackingRoll(enable);

        _getTrackingRoll(clients);
    }
    else {
        if (callback) {
            callback(message);
        }
    }
}

void M1OrientationManagerOSCServerBase::send(const std::vector<M1OrientationDeviceClient>& clients, std::string str) {
    juce::OSCMessage msg(str.c_str());
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::send(const std::vector<M1OrientationDeviceClient>& clients, juce::OSCMessage& msg) {
    for (auto& client : clients) {
        juce::OSCSender sender;
        if (sender.connect("127.0.0.1", client.port)) {
            sender.send(msg);
        }
    }
}

void M1OrientationManagerOSCServerBase::_getDevices(const std::vector<M1OrientationDeviceClient>& clients) {
    std::vector<std::string> devices = getDevices();

    juce::OSCMessage msg("/getDevices");
    for (auto& device : devices) {
        msg.addString(device);
    }
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::_getCurrentDevice(const std::vector<M1OrientationDeviceClient>& clients) {
    std::string currentDevice = getCurrentDevice();
    juce::OSCMessage msg("/getCurrentDevice");
    msg.addString(currentDevice);
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::_getTrackingYaw(const std::vector<M1OrientationDeviceClient>& clients) {
    bool enable = getTrackingYaw();
    juce::OSCMessage msg("/getTrackingYaw");
    msg.addInt32(enable);
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::_getTrackingPitch(const std::vector<M1OrientationDeviceClient>& clients) {
    bool enable = getTrackingPitch();
    juce::OSCMessage msg("/getTrackingPitch");
    msg.addInt32(enable);
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::_getTrackingRoll(const std::vector<M1OrientationDeviceClient>& clients) {
    bool enable = getTrackingRoll();
    juce::OSCMessage msg("/getTrackingRoll");
    msg.addInt32(enable);
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::_getTracking(const std::vector<M1OrientationDeviceClient>& clients) {
    bool enable = getTracking();
    juce::OSCMessage msg("/getTracking");
    msg.addInt32(enable);
    send(clients, msg);
}

M1OrientationManagerOSCServerBase::~M1OrientationManagerOSCServerBase() {
    close();
}

void M1OrientationManagerOSCServerBase::setOrientation(float yaw, float pitch, float roll) {
    juce::OSCMessage msg("/orientation");
    msg.addFloat32(yaw);
    msg.addFloat32(pitch);
    msg.addFloat32(roll);
    send(clients, msg);
}

int M1OrientationManagerOSCServerBase::getClientsCount() {
    return clients.size();
}

void M1OrientationManagerOSCServerBase::setCallback(std::function<void(const juce::OSCMessage& message)> callback) {
    this->callback = callback;
}

bool M1OrientationManagerOSCServerBase::init(int serverPort) {
    // check the port
    juce::DatagramSocket socket(false);
    socket.setEnablePortReuse(false);
    if (socket.bindToPort(serverPort)) {
        socket.shutdown();

        receiver.connect(serverPort);
        receiver.addListener(this);

        this->serverPort = serverPort;

        return true;
    }
    return false;
}

void M1OrientationManagerOSCServerBase::close() {
    receiver.removeListener(this);
    receiver.disconnect();
}
