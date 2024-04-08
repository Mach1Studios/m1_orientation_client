

#pragma once

#include <sstream>
#include "MurkaView.h"
#include "TextField.h"
#include "MurkaBasicWidgets.h"
#include "M1SwitchableIconButton.h"

using namespace murka;

struct M1OrientationClientWindowDeviceSlot {
    std::string icon;
    std::string deviceName;
    bool highlighted = false;
    int index = 0;
    std::function<void(int)> onClickCallback = [](int){};
};

class M1OrientationClientWindow : public murka::View<M1OrientationClientWindow> {
public:
    M1OrientationClientWindow() {
        millisOnStart = juce::Time::currentTimeMillis();
    }

	std::string formatFloatWithLeadingZeros(float value) {
		std::ostringstream oss;
        oss.precision(2); // forces float to only use 2 floating digits
		oss << std::fixed << std::setfill('0') << std::setw(2) << value;
		return oss.str();
	}
    
    void internalDraw(Murka & m) {
        // Updating the state from the client
        
        if (orientationClient != nullptr) {
            isConnected = m1OrientationClient.isConnectedToDevice();
            showOscSettings = (m1OrientationClient.getCurrentDevice().getDeviceType() == M1OrientationManagerDeviceTypeOSC)
            showSWSettings = m1OrientationClient.getCurrentDevice().getDeviceName().find("Supperware HT IMU") != std::string::npos;
            oscSettingsChangedCallback = [&](int requested_osc_port, std::string requested_osc_msg_address) {
                m1OrientationClient.command_setAdditionalDeviceSettings("osc_add="+requested_osc_msg_address);
                m1OrientationClient.command_setAdditionalDeviceSettings("osc_p="+std::to_string(requested_osc_port));
            };
            supperwareSettingsChangedCallback = [&](bool isRightEarChirality) {
                std::string chir_cmd;
                if (isRightEarChirality) {
                    chir_cmd = "1";
                } else {
                    chir_cmd = "0";
                }
                m1OrientationClient.command_setAdditionalDeviceSettings("sw_chir="+chir_cmd);
            };
            disconnectClickedCallback = [&]() { m1OrientationClient.command_disconnect(); };
            recenterClickedCallback = [&]() { m1OrientationClient.command_recenter(); };
            yprSwitchesClickedCallback = [&](int whichone) {
                    if (whichone == 0)
                        // yaw clicked
                        monitorState->yawActive = !monitorState->yawActive;
                        m1OrientationClient.command_setTrackingYawEnabled(monitorState->yawActive);
                    if (whichone == 1)
                        // pitch clicked
                        monitorState->pitchActive = !monitorState->pitchActive;
                        m1OrientationClient.command_setTrackingPitchEnabled(monitorState->pitchActive);
                    if (whichone == 2)
                        // roll clicked
                        monitorState->rollActive = !monitorState->rollActive;
                        m1OrientationClient.command_setTrackingRollEnabled(monitorState->rollActive);
            };
            
        }
        
        
        m.setColor(70 + 40, 40, 40, 255);
        m.enableFill();
        m.drawRectangle(0, 0, shape.size.x, shape.size.y);
        
        if (!inside() && !areInteractiveChildrenHovered() && mouseDownPressed(0)) {
            onClickOutsideCallback();
        }
        
        m.enableFill();
        
        // Drawing the window itself
        
        if (isConnected) {
            m.setColor(235, 87, 87); // fancy red = connected
        } else {
            m.setColor(189, 189, 189); // disconnected white
        }
        m.prepare<M1Label>(MurkaShape(m.getSize().width()/2 - 75, 0, 150, 30)).withText("CONNECTED DEVICE").withTextAlignment(TEXT_CENTER).draw();
        
        m.drawLine(7, 7, m.getSize().width()/2 - 75, 7);
        m.drawLine(m.getSize().width()/2 + 75, 7, m.getSize().width() - 7, 7);
        m.drawLine(7, 7, 7, 28);
        m.drawLine(m.getSize().width() - 7, 7, m.getSize().width() - 7, 28);


        float additionalSettingsOffsetY;
        
        std::vector<std::string> deviceListStrings;
        
        for (int i = 0; i < deviceSlots.size(); i++) {
            deviceListStrings.push_back(deviceSlots[i].deviceName);
        }
        
        // debug device list
        
        deviceListStrings.push_back("<SELECT>");
        deviceListStrings.push_back("test device 1");
        deviceListStrings.push_back("test device 2");
        deviceListStrings.push_back("test device 3");
        deviceListStrings.push_back("test device 4");
        deviceListStrings.push_back("test device 5");
        
        auto& deviceDropdown = m.prepare<M1DropdownMenu>({7, 30, shape.size.x - 14, 120}).withOptions(deviceListStrings);
        deviceDropdown.textAlignment = TEXT_LEFT;
        deviceDropdown.optionHeight = 40;
        
        std::string deviceSelectedOption;
        if (isConnected) {
            // connected device ID
        } else {
            deviceSelectedOption = "<SELECT>";
        }

        
        if (!showDeviceSelectionDropdown) {
            auto& dropdownInit = m.prepare<M1DropdownButton>({7, 30, shape.size.x - 14, 40}).withLabel(deviceSelectedOption).withOutline(true).withBackgroundColor(MurkaColor(BACKGROUND_GREY)).withOutlineColor(MurkaColor(ENABLED_PARAM));
            dropdownInit.textAlignment = TEXT_LEFT;
            dropdownInit.heightDivisor = 3;
            dropdownInit.draw();
            
            if (dropdownInit.pressed) {
                showDeviceSelectionDropdown = true;
                deviceDropdown.open();
            }
        } else {
            deviceDropdown.draw();
//            m.addOverlay([&](){
//            }, &deviceDropdown);
            if (deviceDropdown.changed || !deviceDropdown.opened) {                 
                // UPDATING THE DEVICE PER SELECTED OPTION
                
                showDeviceSelectionDropdown = false;
                deviceDropdown.close();
            }
        }
    
        if (showOscSettings) {
            // Extra dropdown settings for when the OSC device is active for changing the input address/port/address_patter

            int oscDivWidth = int(float(shape.size.x) / 3.0);

            m.setColor(78, 78, 78, 255);
            m.enableFill();
            m.drawRectangle(2, additionalSettingsOffsetY, shape.size.x - 4, 30);
            m.disableFill();
            
            // INPUT MSG ADDRESS PATTERN TEXTFIELD
            auto& msg_address_pattern_field = m.prepare<murka::TextField>({2, additionalSettingsOffsetY, (oscDivWidth * 2) - 4, 30}).onlyAllowNumbers(false).controlling(&requested_osc_msg_address);
            msg_address_pattern_field.drawBounds = true;
            msg_address_pattern_field.widgetBgColor = {0.3, 0.3, 0.3};
            msg_address_pattern_field.hint = "OSC ADDRESS PATTERN";
            msg_address_pattern_field.draw();

            // INPUT IP PORT TEXTFIELD
            auto& ip_port_field = m.prepare<murka::TextField>({2 + oscDivWidth * 2, additionalSettingsOffsetY, oscDivWidth - 4, 30}).onlyAllowNumbers(true).controlling(&requested_osc_port);
            ip_port_field.clampNumber = true;
            ip_port_field.minNumber = 100;
            ip_port_field.maxNumber = 65535;
            ip_port_field.drawBounds = true;
            ip_port_field.widgetBgColor = {0.3, 0.3, 0.3};
            ip_port_field.hint = "OSC PORT";
            ip_port_field.draw();
            
            if (ip_port_field.editingFinished || msg_address_pattern_field.editingFinished) {
                oscSettingsChangedCallback(requested_osc_port, requested_osc_msg_address);
            }
        }
        
        if (showSWSettings) {
            // extra dropdown settings for Supperware device
            
            int swDivWidth = int(float(shape.size.x) / 3.0);
            
            m.setColor(78, 78, 78, 255);
            m.enableFill();
            m.drawRectangle(2, additionalSettingsOffsetY, shape.size.x - 4, 30);
            m.disableFill();

            auto& sw_chirality_switch = m.prepare<M1SwitchableIconButton>({2, additionalSettingsOffsetY + 4, (swDivWidth * 2) - 4, 25});
            sw_chirality_switch.withBorders();
            sw_chirality_switch.onClick([&](M1SwitchableIconButton& b){
                // Used to indicate how the IMU is mounted on the head
                // chirality = true  | USB connector is on the right side
                // chirality = false | USB connector is on the left side
                isRightEarChirality = !isRightEarChirality;
                supperwareSettingsChangedCallback(isRightEarChirality);
            });
            if (!isRightEarChirality) {
                sw_chirality_switch.caption = "USB ON THE LEFT";
            } else {
                sw_chirality_switch.caption = "USB ON THE RIGHT";
            }
            sw_chirality_switch.fontSize = 5;
            sw_chirality_switch.withFontSize(5);
            sw_chirality_switch.draw();
        }
        
    }
    
    void startRefreshing() {
        millisWhenRefreshingStarted = juce::Time::currentTimeMillis();
        refreshing = true;
		onRefreshCallback();
    }
    
    bool refreshing = true;
    juce::int64 millisOnStart = 0;
    juce::int64 millisWhenRefreshingStarted = 0;
    

    M1OrientationClientWindow& withOrientationClient(M1OrientationClient& client) {
        orientationClient = &client;
    }
    
    M1OrientationClient* orientationClient = nullptr;
    
    bool isConnected = false;
    std::string connectedDeviceId = "";

    bool trackYaw;
    bool trackPitch;
    bool trackRoll;
    std::pair<int, int> yawRange;
    std::pair<int, int> pitchRange;
    std::pair<int, int> rollRange;
	float yaw;
	float pitch;
	float roll;

    std::function<void(int)> yprSwitchesClickedCallback;
    std::function<void()> recenterClickedCallback;
    std::function<void()> disconnectClickedCallback;
    std::vector<M1OrientationClientWindowDeviceSlot> deviceSlots;
    
	std::function<void()> onClickOutsideCallback = []() {};
	std::function<void()> onRefreshCallback = []() {};
	
    MurImage icon;
    
    bool initialized = false;
    bool showSettings = false;
    bool enabled = true;
    
    bool showDeviceSelectionDropdown = false;

    // device specific
    bool showSWSettings = false;
    bool isRightEarChirality = true;
    std::function<void(bool)> supperwareSettingsChangedCallback;

    // osc specific
    bool showOscSettings = false;
    int requested_osc_port = 9901; // default value
    std::string requested_osc_msg_address = "/orientation"; // default value
    std::function<void(int, std::string)> oscSettingsChangedCallback;
};
