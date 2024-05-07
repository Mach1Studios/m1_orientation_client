

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
            isConnected = orientationClient->isConnectedToDevice();
            
            // Debug
            isConnected = (deviceSelectedOption != "<SELECT>");
            
            showOscSettings = (orientationClient->getCurrentDevice().getDeviceType() == M1OrientationManagerDeviceTypeOSC);
            showSWSettings = orientationClient->getCurrentDevice().getDeviceName().find("Supperware HT IMU") != std::string::npos;
            oscSettingsChangedCallback = [&](int requested_osc_port, std::string requested_osc_msg_address) {
                orientationClient->command_setAdditionalDeviceSettings("osc_add="+requested_osc_msg_address);
                orientationClient->command_setAdditionalDeviceSettings("osc_p="+std::to_string(requested_osc_port));
            };
            supperwareSettingsChangedCallback = [&](bool isRightEarChirality) {
                std::string chir_cmd;
                if (isRightEarChirality) {
                    chir_cmd = "1";
                } else {
                    chir_cmd = "0";
                }
                orientationClient->command_setAdditionalDeviceSettings("sw_chir="+chir_cmd);
            };
            disconnectClickedCallback = [&]() { orientationClient->command_disconnect();
            };
            recenterClickedCallback = [&]() {
                orientationClient->command_recenter();
            };
            yprSwitchesClickedCallback = [&](int whichone) {
                if (whichone == 0)
                    // yaw clicked
                    orientationClient->command_setTrackingYawEnabled(!orientationClient->getTrackingYawEnabled());
                if (whichone == 1)
                    // pitch clicked
                    orientationClient->command_setTrackingPitchEnabled(!orientationClient->getTrackingPitchEnabled());
                if (whichone == 2)
                    // roll clicked
                    orientationClient->command_setTrackingRollEnabled(!orientationClient->getTrackingRollEnabled());
            };
        }
        
        if (!inside() && !areInteractiveChildrenHovered() && mouseDownPressed(0)) {
            onClickOutsideCallback();
        }
        
        m.enableFill();
        
        // Drawing the window itself
        
        MurkaColor connectedRed = MurkaColor(255.0 / 255.0, 72 / 255.0, 80 / 255.0);
        MurkaColor backgroundHover = MurkaColor(0.25, 0.25, 0.25);
        
        if (isConnected) {
            m.setColor(connectedRed); // fancy red = connected
        } else {
            m.setColor(189, 189, 189); // disconnected white
        }
        m.prepare<M1Label>(MurkaShape(m.getSize().width()/2 - 75, 0, 150, 30)).withText("CONNECTED DEVICE").withTextAlignment(TEXT_CENTER).draw();
        
        m.drawLine(7, 7, m.getSize().width()/2 - 75, 7);
        m.drawLine(m.getSize().width()/2 + 75, 7, m.getSize().width() - 7, 7);
        m.drawLine(7, 7, 7, 16);
        m.drawLine(m.getSize().width() - 7, 7, m.getSize().width() - 7, 16);

        float additionalSettingsOffsetY = 140; // dropdown starts at 30 y and is 120 pix long in height
        
        if (isConnected) {
            // XYZ buttons / tracking enablers
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 0,
                                          additionalSettingsOffsetY,
                                          m.getSize().width()/3, 30))
            .withText("YAW").withTextAlignment(TEXT_CENTER).draw();
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 1,
                                          additionalSettingsOffsetY,
                                          m.getSize().width()/3, 30))
            .withText("PITCH").withTextAlignment(TEXT_CENTER).draw();
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 2,
                                          additionalSettingsOffsetY,
                                          m.getSize().width()/3, 30))
            .withText("ROLL").withTextAlignment(TEXT_CENTER).draw();
            
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 0 + 2,
                                          additionalSettingsOffsetY + 22,
                                          m.getSize().width()/3 - 4, 30))
            .withText("100").withTextAlignment(TEXT_CENTER).withVerticalTextOffset(10)
            .withStrokeBorder(connectedRed).
            draw();
            
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 1 + 2,
                                          additionalSettingsOffsetY + 22,
                                          m.getSize().width()/3 - 4, 30))
            .withText("0").withTextAlignment(TEXT_CENTER).withVerticalTextOffset(10)
            .withStrokeBorder(connectedRed).draw();
            
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 2 + 2,
                                          additionalSettingsOffsetY + 22,
                                          m.getSize().width()/3 - 4, 30))
            .withText("25").withTextAlignment(TEXT_CENTER).withVerticalTextOffset(10)
            .withStrokeBorder(connectedRed).draw();
            
            // Re-center and disconnect
            
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/2 * 0 + 6,
                                          additionalSettingsOffsetY + 60,
                                          m.getSize().width()/2 - 8, 30))
            .withText("RE-CENTER").withTextAlignment(TEXT_CENTER).withVerticalTextOffset(10)
            .withOnClickFlash()
            .withBackgroundFill(backgroundHover, MurkaColor(26.0 / 255.0, 26.0 / 255.0, 26.0 / 255.0))
            .withStrokeBorder(connectedRed).draw();
            
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/2 * 1 + 2,
                                          additionalSettingsOffsetY + 60,
                                          m.getSize().width()/2 - 8, 30))
            .withBackgroundFill(backgroundHover, MurkaColor(26.0 / 255.0, 26.0 / 255.0, 26.0 / 255.0))
            .withOnClickFlash()
            .withText("DISCONNECT").withTextAlignment(TEXT_CENTER).withVerticalTextOffset(10)
            .withStrokeBorder(connectedRed).draw();
            
        }
        
        float additionalOptionY = 80;
        
        if (deviceSelectedOption == "SUPPERWARE HT IMU") {
            m.prepare<M1Label>(MurkaShape(6, additionalOptionY, shape.size.x  - 8, 30))
            .withBackgroundFill(backgroundHover, MurkaColor(26.0 / 255.0, 26.0 / 255.0, 26.0 / 255.0))
            .withText(debugSupperwareChirality).withTextAlignment(TEXT_CENTER).withVerticalTextOffset(10)
            .withOnClickCallback([&](){
                if (debugSupperwareChirality == "USB ON THE LEFT") {
                    debugSupperwareChirality = "USB ON THE RIGHT";
                } else {
                    debugSupperwareChirality = "USB ON THE LEFT";
                }
            })
            .withOnClickFlash()
            .withStrokeBorder(connectedRed).draw();
        }
        
        if (deviceSelectedOption == "OSC Device") {
            
            // INPUT MSG ADDRESS PATTERN TEXTFIELD
            auto& msg_address_pattern_field = m.prepare<murka::TextField>({2, additionalOptionY, shape.size.x * 0.7 - 4, 30}).onlyAllowNumbers(false).controlling(&requested_osc_msg_address);
            msg_address_pattern_field.drawBounds = false;
            msg_address_pattern_field.hint = "OSC ADDRESS PATTERN";
            msg_address_pattern_field.widgetBgColor = BACKGROUND_GREY;
            msg_address_pattern_field.widgetFgColor = connectedRed;
            msg_address_pattern_field.draw();
            
//            m.pushStyle();
            m.disableFill();
            m.setColor(connectedRed);
            m.drawRectangle(msg_address_pattern_field.shape);
            m.enableFill();
//            m.popStyle();

            // INPUT IP PORT TEXTFIELD
            auto& ip_port_field = m.prepare<murka::TextField>({shape.size.x * 0.7 + 2, additionalOptionY, shape.size.x * 0.3 - 2, 30}).onlyAllowNumbers(true).controlling(&requested_osc_port);
            ip_port_field.clampNumber = true;
            ip_port_field.minNumber = 100;
            ip_port_field.maxNumber = 65535;
            ip_port_field.drawBounds = false;
            ip_port_field.widgetBgColor = BACKGROUND_GREY;
            ip_port_field.widgetFgColor = connectedRed;
            ip_port_field.hint = "OSC PORT";
            ip_port_field.draw();

//            m.pushStyle();
            m.disableFill();
            m.setColor(connectedRed);
            m.drawRectangle(ip_port_field.shape);
            m.enableFill();
//            m.popStyle();

        }

        std::vector<std::string> deviceListStrings;
        if (deviceSlots.size() > 0) {
            deviceListStrings.push_back("<SELECT>");
        } else {
            deviceListStrings.push_back("<NOT AVAILABLE>");
        }

        for (int i = 0; i < deviceSlots.size(); i++) {
            deviceListStrings.push_back(deviceSlots[i].deviceName);
        }
        
        // !!! debug device list !!!
        /*
        deviceListStrings.push_back("<SELECT>");
        deviceListStrings.push_back("OSC Device");
        deviceListStrings.push_back("SUPPERWARE HT IMU");
        deviceListStrings.push_back("test device 3");
        deviceListStrings.push_back("test device 4");
        deviceListStrings.push_back("test device 5");
        */
         
        float deviceDropdownY = 28;
        
        auto& deviceDropdown = m.prepare<M1DropdownMenu>({7, deviceDropdownY, shape.size.x - 14, 120}).withOptions(deviceListStrings);
        deviceDropdown.textAlignment = TEXT_LEFT;
        deviceDropdown.optionHeight = 40;
        
        if (!showDeviceSelectionDropdown) {
            MurkaShape dropdownInitShape = MurkaShape(7, deviceDropdownY, shape.size.x - 14, 40);

            m.setColor(MurkaColor(0.25, 0.25, 0.25));
            m.enableFill();
            m.drawRectangle(dropdownInitShape);
            
            auto& dropdownInit = m.prepare<M1DropdownButton>(dropdownInitShape).withLabel(deviceSelectedOption).withOutline(true).withBackgroundColor(MurkaColor(BACKGROUND_GREY))
                .withTriangle(true).withOutlineColor(isConnected ? connectedRed : MurkaColor(ENABLED_PARAM));
            dropdownInit.textAlignment = TEXT_LEFT;
            dropdownInit.heightDivisor = 3;
            dropdownInit.draw();
            
            if (dropdownInit.pressed) {
                showDeviceSelectionDropdown = true;
                deviceDropdown.open();
            }
        } else {
            deviceDropdown.draw();
            if (deviceDropdown.changed || !deviceDropdown.opened) {
                // UPDATING THE DEVICE PER SELECTED OPTION
                deviceSelectedOption = deviceListStrings[deviceDropdown.selectedOption];
                showDeviceSelectionDropdown = false;
                deviceDropdown.close();
            }
        }
    }
    
    void startRefreshing() {
        millisWhenRefreshingStarted = juce::Time::currentTimeMillis();
        refreshing = true;
		onRefreshCallback();
    }
    
    std::string deviceSelectedOption = "<SELECT>";
    
    bool refreshing = true;
    juce::int64 millisOnStart = 0;
    juce::int64 millisWhenRefreshingStarted = 0;

    M1OrientationClientWindow& withOrientationClient(M1OrientationClient& client) {
        orientationClient = &client;
        return *this;
    }
    
    std::string debugSupperwareChirality = "USB ON THE LEFT";

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
