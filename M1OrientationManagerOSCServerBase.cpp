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
            M1OrientationManagerClientConnection client;
            client.port = port;
            client.time = juce::Time::currentTimeMillis();
            clients.push_back(client);
        }

        juce::OSCSender sender;
        if (sender.connect("127.0.0.1", port)) {
            juce::OSCMessage msg("/connectedToServer");
            sender.send(msg);
        }

        std::vector<M1OrientationManagerClientConnection> clients = { M1OrientationManagerClientConnection { port, 0 } };
        send_getDevices(clients);
        send_getCurrentDevice(clients);
        send_getTracking(clients);
        send_getTrackingYaw(clients);
        send_getTrackingPitch(clients);
        send_getTrackingRoll(clients);
    }
    else if (message.getAddressPattern() == "/refreshDevices") {
        refreshDevices();
    }
    else if (message.getAddressPattern() == "/selectDevice") {
        std::string device = message[0].getString().toStdString();
        command_selectDevice(device);
    }
    else if (message.getAddressPattern() == "/setTracking") {
        bool enable = message[0].getInt32();
        command_setTracking(enable);
    }
    else if (message.getAddressPattern() == "/setTrackingYaw") {
        bool enable = message[0].getInt32();
        command_setTrackingYaw(enable);
    }
    else if (message.getAddressPattern() == "/setTrackingPitch") {
        bool enable = message[0].getInt32();
        command_setTrackingPitch(enable);
    }
    else if (message.getAddressPattern() == "/setTrackingRoll") {
        bool enable = message[0].getInt32();
        command_setTrackingRoll(enable);
    }
    else {
        if (callback) {
            callback(message);
        }
    }
}

void M1OrientationManagerOSCServerBase::send(const std::vector<M1OrientationManagerClientConnection>& clients, std::string str) {
    juce::OSCMessage msg(str.c_str());
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::send(const std::vector<M1OrientationManagerClientConnection>& clients, juce::OSCMessage& msg) {
    for (auto& client : clients) {
        juce::OSCSender sender;
        if (sender.connect("127.0.0.1", client.port)) {
            sender.send(msg);
        }
    }
}

void M1OrientationManagerOSCServerBase::send_getDevices(const std::vector<M1OrientationManagerClientConnection>& clients) {
    std::vector<std::string> devices = getDevices();

    juce::OSCMessage msg("/getDevices");
    for (auto& device : devices) {
        msg.addString(device);
    }
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::send_getCurrentDevice(const std::vector<M1OrientationManagerClientConnection>& clients) {
    std::string currentDevice = getCurrentDevice();
    juce::OSCMessage msg("/getCurrentDevice");
    msg.addString(currentDevice);
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::send_getTrackingYaw(const std::vector<M1OrientationManagerClientConnection>& clients) {
    bool enable = getTrackingYaw();
    juce::OSCMessage msg("/getTrackingYaw");
    msg.addInt32(enable);
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::send_getTrackingPitch(const std::vector<M1OrientationManagerClientConnection>& clients) {
    bool enable = getTrackingPitch();
    juce::OSCMessage msg("/getTrackingPitch");
    msg.addInt32(enable);
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::send_getTrackingRoll(const std::vector<M1OrientationManagerClientConnection>& clients) {
    bool enable = getTrackingRoll();
    juce::OSCMessage msg("/getTrackingRoll");
    msg.addInt32(enable);
    send(clients, msg);
}

void M1OrientationManagerOSCServerBase::send_getTracking(const std::vector<M1OrientationManagerClientConnection>& clients) {
    bool enable = getTracking();
    juce::OSCMessage msg("/getTracking");
    msg.addInt32(enable);
    send(clients, msg);
}

M1OrientationManagerOSCServerBase::~M1OrientationManagerOSCServerBase() {
    close();
}

void M1OrientationManagerOSCServerBase::setOrientation(float yaw, float pitch, float roll) {
    if (getTracking()) {
        if (getTrackingYaw()) {
            juce::OSCMessage msg("/yaw");
            msg.addFloat32(yaw);
            send(clients, msg);
        }
        if (getTrackingPitch()) {
            juce::OSCMessage msg("/pitch");
            msg.addFloat32(pitch);
            send(clients, msg);
        }
        if (getTrackingRoll()) {
            juce::OSCMessage msg("/roll");
            msg.addFloat32(roll);
            send(clients, msg);
        }
    }
}

std::vector<M1OrientationManagerClientConnection> M1OrientationManagerOSCServerBase::getClients() {
    return clients;
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

void M1OrientationManagerOSCServerBase::command_refreshDevices() {
    
    refreshDevices();
    send_getDevices(clients);
}

void M1OrientationManagerOSCServerBase::command_selectDevice(std::string device) {
    selectDevice(device);

    send_getCurrentDevice(clients);
}

void M1OrientationManagerOSCServerBase::command_setTracking(bool enable) {
    setTracking(enable);

    send_getTracking(clients);
}

void M1OrientationManagerOSCServerBase::command_setTrackingYaw(bool enable) {
    setTrackingYaw(enable);

    send_getTrackingYaw(clients);
}

void M1OrientationManagerOSCServerBase::command_setTrackingPitch(bool enable) {
    setTrackingPitch(enable);

    send_getTrackingPitch(clients);
}

void M1OrientationManagerOSCServerBase::command_setTrackingRoll(bool enable) {
    setTrackingRoll(enable);

    send_getTrackingRoll(clients);
}
