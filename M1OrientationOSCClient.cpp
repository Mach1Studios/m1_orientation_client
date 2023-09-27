#include "M1OrientationOSCClient.h"

#include <iostream>
#include <thread>
#include <chrono>

void M1OrientationOSCClient::oscMessageReceived(const juce::OSCMessage& message) {
    
    // watch dog
    if (message.getAddressPattern() == "/Mach1/ActiveClients") {
        // restart the ping timer because we received a ping
        watchDogPingTime = juce::Time::currentTimeMillis();

        if (message.size() > 0) {
            activeClientCounter = message[0].getInt32();
            DBG("Received message from " + message.getAddressPattern().toString() + ", with " + std::to_string(message[0].getInt32()) + " active clients");
        } else {
            DBG("Received message from " + message.getAddressPattern().toString() + ", with 0 active clients");
        }
    }
    
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
        if (message.size() == 3){
            /// WARNING: ONLY SEND SIGNED NORMALLED ORIENTATION VALUES
            /// -1.0 -> 1.0
            M1OrientationYPR incomingOrientation;
            incomingOrientation.angleType = M1OrientationYPR::SIGNED_NORMALLED;
            incomingOrientation.yaw = message[0].getFloat32();
            incomingOrientation.pitch = message[1].getFloat32();
            incomingOrientation.roll = message[2].getFloat32();
            orientation.setYPR(incomingOrientation);
        } else if (message.size() == 4){
        // quat input
            orientation.setQuat({ message[0].getFloat32(), message[1].getFloat32(), message[2].getFloat32(), message[3].getFloat32() });
        } else {
            // error we have an undefined number of values in the message
            DBG("Error: Undefined orientation message size:"+std::to_string(message.size()));
        }
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
    if ((serverPort > 100 && serverPort < 65535) && (serverPort > 100 && serverPort < 65535)) {
        if (sender.connect("127.0.0.1", serverPort)) {
            sender.send(msg);
            return true;
        } else {
            // if this send returns false, check for reconnection state
            // TODO: This is an error, if we are sending messages but missing the server we should try to reconnect here
        }
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

void M1OrientationOSCClient::command_updateOscDevice(int new_osc_port, std::string new_osc_addr_pattrn) {
    juce::OSCMessage msg("/setOscDeviceSettings");
    msg.addInt32(new_osc_port);
    msg.addString(new_osc_addr_pattrn);
    send(msg);
}

void M1OrientationOSCClient::command_setMonitorYPR(int mode = 0, float yaw = 0, float pitch = 0, float roll = 0) {
    // It is expected to send the orientation to the monitor, let the monitor process its orientation and return it here for reporting to other plugin instances
    juce::OSCMessage msg("/setMonitorYPR");
    msg.addInt32(mode);
    msg.addFloat32(yaw);
    msg.addFloat32(pitch);
    msg.addFloat32(roll);
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
    orientation.resetOrientation();
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

bool M1OrientationOSCClient::init(int serverPort) {
    // TODO: Add UI feedback for this process to stop user from selecting another device during connection
    
    // Using `currentApplicationFile` to be safe for both plugins and apps on all OS targets
    //juce::File pluginExe = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    //juce::File appDirectory = pluginExe.getParentDirectory();
    
    // If a sibling OrientationManager is found then we skip past the services section as this overrides for easy local usage
    juce::File siblingOrientationManager;
    if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::Windows) != 0) {
        siblingOrientationManager = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getSiblingFile("m1-orientationmanager.exe");
    } else {
        siblingOrientationManager = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getSiblingFile("m1-orientationmanager");
    }

    if (siblingOrientationManager.exists()) {
        // directly launching orientation manager because we are in local usage or debug mode
        juce::ChildProcess exeProcess;
        juce::StringArray arguments;
        arguments.add(siblingOrientationManager.getFullPathName().quoted());
        arguments.add("--no-gui");
        DBG("Starting m1-orientationmanager: " + siblingOrientationManager.getFullPathName().quoted());
        if (exeProcess.start(arguments)) {
            DBG("Started m1-orientationmanager");
        } else {
            // Failed to start the process
            DBG("Failed to start m1-orientationmanager");
            exit(1);
        }
    }
    
    // Using common support files installation location
    juce::File m1SupportDirectory = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory);

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
                    // test to skip this loop due to improper parsing of port number
                    // this likely is just a debugger catching the wrong instance
                    if ((this->clientPort > 100 && this->clientPort < 65535) && (this->serverPort > 100 && this->serverPort < 65535)) {
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
                }
            }).detach();
            
            // start the watch dog and m1-orientationmanager app
            startOrientationManager();
            watchDogPingTime = juce::Time::currentTimeMillis();
            // starts the m1-orientationmanager ping timer
            startTimer(500);
            
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

// m1-orientationmanager related functions

void M1OrientationOSCClient::startOrientationManager() {
    
    // Create a DatagramSocket to check the availability of port serverPort
    juce::DatagramSocket socket(false);
    socket.setEnablePortReuse(false);
    if (socket.bindToPort(serverPort)) {
        socket.shutdown();
        
        // If a sibling OrientationManager is found then we skip past the services section as this overrides for easy local usage
        juce::File siblingOrientationManager;
        if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::Windows) != 0) {
            siblingOrientationManager = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getSiblingFile("m1-orientationmanager.exe");
        } else {
            siblingOrientationManager = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getSiblingFile("m1-orientationmanager");
        }

        if (siblingOrientationManager.exists()) {
            /// MANAGED BY LOCAL FOUND EXE

            // directly launching orientation manager because we are in local usage or debug mode
            juce::ChildProcess exeProcess;
            juce::StringArray arguments;
            arguments.add(siblingOrientationManager.getFullPathName().quoted());
            arguments.add("--no-gui");
            DBG("Starting m1-orientationmanager: " + siblingOrientationManager.getFullPathName().quoted());
            if (exeProcess.start(arguments)) {
                DBG("Started m1-orientationmanager");
            } else {
                // Failed to start the process
                DBG("Failed to start m1-orientationmanager");
                exit(1);
            }
        } else {
            /// MANAGED BY SERVICES
            
            // We will assume the folders are properly created during the installation step
            // Using common support files installation location
            juce::File m1SupportDirectory = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory);
            juce::File orientationManagerExe; // for linux and win
            
            if ((juce::SystemStats::getOperatingSystemType() == juce::SystemStats::MacOSX_10_7) ||
                (juce::SystemStats::getOperatingSystemType() == juce::SystemStats::MacOSX_10_8) ||
                (juce::SystemStats::getOperatingSystemType() == juce::SystemStats::MacOSX_10_9)) {
                // MacOS 10.7-10.9, launchd v1.0
                // load process m1-orientationmanager
                std::string load_command = "launchctl load -w /Library/LaunchDaemons/com.mach1.spatial.orientationmanager.plist";
                DBG("Executing: " + load_command);
                system(load_command.c_str());
                // start process m1-orientationmanager
                std::string command = "launchctl start com.mach1.spatial.orientationmanager";
                DBG("Executing: " + command);
                system(command.c_str());
                
            } else if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
                // All newer MacOS, launchd v2.0
                // load process m1-orientationmanager
                std::string load_command = "launchctl bootstrap gui/$UID /Library/LaunchDaemons/com.mach1.spatial.orientationmanager.plist";
                DBG("Executing: " + load_command);
                system(load_command.c_str());
                // start process m1-orientationmanager
                std::string command = "launchctl kickstart -p gui/$UID/com.mach1.spatial.orientationmanager";
                DBG("Executing: " + command);
                system(command.c_str());
                
            } else if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::Windows) != 0) {
                // Any windows OS
                // TODO: migrate to Windows Service Manager
                orientationManagerExe = m1SupportDirectory.getChildFile("Mach1").getChildFile("m1-orientationmanager.exe");
                juce::StringArray arguments;
                arguments.add(orientationManagerExe.getFullPathName().quoted());
                arguments.add("--no-gui");
                DBG("Starting m1-orientationmanager: " + orientationManagerExe.getFullPathName());
                if (orientationManagerProcess.start(arguments)) {
                    DBG("Started m1-orientationmanager server");
                } else {
                    // Failed to start the process
                    DBG("Failed to start the m1-orientationmanager");
                    exit(1);
                }
                
            } else if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::Linux)) {
                // TODO: factor out linux using systemd service

            } else {
                orientationManagerExe = m1SupportDirectory.getChildFile("Mach1").getChildFile("m1-orientationmanager");
                juce::StringArray arguments;
                arguments.add(orientationManagerExe.getFullPathName().quoted());
                arguments.add("--no-gui");
                DBG("Starting m1-orientationmanager: " + orientationManagerExe.getFullPathName());
                if (orientationManagerProcess.start(arguments)) {
                    DBG("Started m1-orientationmanager server");
                } else {
                    // Failed to start the process
                    DBG("Failed to start the m1-orientationmanager");
                    exit(1);
                }
            }
        }
    }
}

void M1OrientationOSCClient::killProcessByName(const char *name)
{
    std::string service_name;
    if (std::string(name) == "m1-orientationmanager") {
        // for launchctl we need to use the service name instead of the executable name
        service_name = "com.mach1.spatial.orientationmanager";
    }
    
    DBG("Killing "+std::string(name)+"...");
    
    std::string command;
    if ((juce::SystemStats::getOperatingSystemType() == juce::SystemStats::MacOSX_10_7) ||
        (juce::SystemStats::getOperatingSystemType() == juce::SystemStats::MacOSX_10_8) ||
        (juce::SystemStats::getOperatingSystemType() == juce::SystemStats::MacOSX_10_9)) {
        // MacOS 10.7-10.9, launchd v1.0
        command =  "launchctl unload -w /Library/LaunchDaemons/"+service_name+".plist";
    } else if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
        // All newer MacOS, launchd v2.0
        command = "launchctl kill 9 gui/$UID/"+service_name;
    } else if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::Windows) != 0) {
        command = "taskkill /IM "+std::string(name)+" /F";
    } else {
        command = "pkill "+std::string(name);
    }
    DBG("Executing: " + command);
    system(command.c_str());
}
