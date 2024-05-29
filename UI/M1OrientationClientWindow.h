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
            isConnected = (deviceSelectedOption != "<SELECT DEVICE>");
            
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
            disconnectClickedCallback = [&]() {
                orientationClient->command_disconnect();
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

        if (isConnected) {
            m.setColor(ORIENTATION_ACTIVE_COLOR);
        } else {
            m.setColor(ENABLED_PARAM); // disconnected white
        }
        m.prepare<M1Label>(MurkaShape(m.getSize().width()/2 - 75, 0, 150, 30)).withText("CONNECTED DEVICE").withTextAlignment(TEXT_CENTER).draw();
        
        m.drawLine(7, 7, m.getSize().width()/2 - 75, 7);
        m.drawLine(m.getSize().width()/2 + 75, 7, m.getSize().width() - 7, 7);
        m.drawLine(7, 7, 7, 16);
        m.drawLine(m.getSize().width() - 7, 7, m.getSize().width() - 7, 16);

        float additionalSettingsOffsetY = 140; // dropdown starts at 30 y and is 120 pix long in height
        
        if (isConnected) {
            // XYZ buttons / tracking enablers
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 0 + 2,
                                          additionalSettingsOffsetY,
                                          m.getSize().width()/3, 30))
            .withText("YAW").withTextAlignment(TEXT_CENTER).draw();
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 1 + 2,
                                          additionalSettingsOffsetY,
                                          m.getSize().width()/3, 30))
            .withText("PITCH").withTextAlignment(TEXT_CENTER).draw();
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 2 + 0,
                                          additionalSettingsOffsetY,
                                          m.getSize().width()/3, 30))
            .withText("ROLL").withTextAlignment(TEXT_CENTER).draw();
                        
            // Yaw value display & Enable button
            std::stringstream ytmp;
            ytmp << std::fixed << std::setprecision(3) << orientationClient->getOrientation().GetGlobalRotationAsEulerDegrees().GetYaw() + 0.0;
            std::string yawValue = ytmp.str();
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 0 + 6,
                                          additionalSettingsOffsetY + 22,
                                          m.getSize().width()/3 - 6, 30))
            .withText((orientationClient->getCurrentDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone) ? yawValue : "0.000").withTextAlignment(TEXT_CENTER).withVerticalTextOffset(8)
            .withBackgroundFill(MurkaColor(DISABLED_PARAM), MurkaColor(BACKGROUND_GREY))
            .withStrokeBorder(MurkaColor(ORIENTATION_ACTIVE_COLOR))
            .withOnClickCallback([&](){
                orientationClient->command_setTrackingYawEnabled(!orientationClient->getTrackingYawEnabled());
            })
            .draw();
            
            // Pitch value display & Enable button
            std::stringstream ptmp;
            ptmp << std::fixed << std::setprecision(3) << orientationClient->getOrientation().GetGlobalRotationAsEulerDegrees().GetPitch() + 0.0;
            std::string pitchValue = ptmp.str();
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 1  + 4,
                                          additionalSettingsOffsetY + 22,
                                          m.getSize().width()/3 - 8, 30))
            .withText((orientationClient->getCurrentDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone) ? pitchValue : "0.000").withTextAlignment(TEXT_CENTER).withVerticalTextOffset(8)
            .withBackgroundFill(MurkaColor(DISABLED_PARAM), MurkaColor(BACKGROUND_GREY))
            .withStrokeBorder(MurkaColor(ORIENTATION_ACTIVE_COLOR))
            .withOnClickCallback([&](){
                orientationClient->command_setTrackingPitchEnabled(!orientationClient->getTrackingPitchEnabled());
            })
            .draw();
            
            // Roll value display & Enable button
            std::stringstream rtmp;
            rtmp << std::fixed << std::setprecision(3) << orientationClient->getOrientation().GetGlobalRotationAsEulerDegrees().GetRoll() + 0.0;
            std::string rollValue = rtmp.str();
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/3 * 2 + 0,
                                          additionalSettingsOffsetY + 22,
                                          m.getSize().width()/3 - 6, 30))
            .withText((orientationClient->getCurrentDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone) ? rollValue : "0.000").withTextAlignment(TEXT_CENTER).withVerticalTextOffset(8)
            .withBackgroundFill(MurkaColor(DISABLED_PARAM), MurkaColor(BACKGROUND_GREY))
            .withStrokeBorder(MurkaColor(ORIENTATION_ACTIVE_COLOR))
            .withOnClickCallback([&](){
                orientationClient->command_setTrackingRollEnabled(!orientationClient->getTrackingRollEnabled());
            })
            .draw();
            
            // Recenter and disconnect
            
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/2 * 0 + 6,
                                          additionalSettingsOffsetY + 60,
                                          m.getSize().width()/2 - 8, 30))
            .withText("RECENTER").withTextAlignment(TEXT_CENTER).withVerticalTextOffset(8)
            .withOnClickFlash()
            .withBackgroundFill(MurkaColor(DISABLED_PARAM), MurkaColor(BACKGROUND_GREY))
            .withStrokeBorder(MurkaColor(ORIENTATION_ACTIVE_COLOR))
            .withOnClickCallback([&](){
                orientationClient->command_recenter();
            })
            .draw();
            
            m.prepare<M1Label>(MurkaShape(m.getSize().width()/2 * 1 + 2,
                                          additionalSettingsOffsetY + 60,
                                          m.getSize().width()/2 - 8, 30))
            .withBackgroundFill(MurkaColor(DISABLED_PARAM), MurkaColor(BACKGROUND_GREY))
            .withOnClickFlash()
            .withText("DISCONNECT").withTextAlignment(TEXT_CENTER).withVerticalTextOffset(8)
            .withStrokeBorder(MurkaColor(ORIENTATION_ACTIVE_COLOR))
            .withOnClickCallback([&](){
                orientationClient->command_disconnect();
            })
            .draw();
            
            float additionalOptionY = 80;
            
            if (deviceSelectedOption == "SUPPERWARE HT IMU") {
                m.prepare<M1Label>(MurkaShape(6, additionalOptionY, shape.size.x  - 8, 30))
                .withBackgroundFill(MurkaColor(DISABLED_PARAM), MurkaColor(BACKGROUND_GREY))
                .withText(supperwareChirality).withTextAlignment(TEXT_CENTER).withVerticalTextOffset(8)
                .withOnClickCallback([&](){
                    if (supperwareChirality == "USB ON THE LEFT") {
                        supperwareChirality = "USB ON THE RIGHT";
                    } else {
                        supperwareChirality = "USB ON THE LEFT";
                    }
                })
                .withOnClickFlash()
                .withStrokeBorder(MurkaColor(ORIENTATION_ACTIVE_COLOR))
                .draw();
            }
            
            if (deviceSelectedOption == "OSC Input" || deviceSelectedOption == "OSC Device") {
                // INPUT MSG ADDRESS PATTERN TEXTFIELD
                auto& msg_address_pattern_field = m.prepare<murka::TextField>({8, additionalOptionY, shape.size.x * 0.7 - 10, 30}).onlyAllowNumbers(false).controlling(&requested_osc_msg_address);
                msg_address_pattern_field.drawBounds = false;
                msg_address_pattern_field.hint = "OSC ADDRESS PATTERN";
                msg_address_pattern_field.widgetBgColor = MurkaColor(BACKGROUND_GREY);
                msg_address_pattern_field.widgetFgColor = MurkaColor(ORIENTATION_ACTIVE_COLOR);
                msg_address_pattern_field.draw();

                m.disableFill();
                m.setColor(ORIENTATION_ACTIVE_COLOR);
                m.drawRectangle(msg_address_pattern_field.shape);
                m.enableFill();

                // INPUT IP PORT TEXTFIELD
                auto& ip_port_field = m.prepare<murka::TextField>({shape.size.x * 0.7 + 6, additionalOptionY, shape.size.x * 0.3 - 14, 30}).onlyAllowNumbers(true).controlling(&requested_osc_port);
                ip_port_field.clampNumber = true;
                ip_port_field.minNumber = 100;
                ip_port_field.maxNumber = 65535;
                ip_port_field.drawBounds = false;
                ip_port_field.widgetBgColor = MurkaColor(BACKGROUND_GREY);
                ip_port_field.widgetFgColor = MurkaColor(ORIENTATION_ACTIVE_COLOR);
                ip_port_field.hint = "OSC PORT";
                ip_port_field.draw();

                m.disableFill();
                m.setColor(ORIENTATION_ACTIVE_COLOR);
                m.drawRectangle(ip_port_field.shape);
                m.enableFill();
                
                if (ip_port_field.editingFinished || msg_address_pattern_field.editingFinished) {
                    oscSettingsChangedCallback(requested_osc_port, requested_osc_msg_address);
                }
            }
        }
        
        std::vector<std::string> deviceListStrings;
        if (deviceSlots.size() > 0) {
            deviceListStrings.push_back("<SELECT DEVICE>");
        } else {
            deviceListStrings.push_back("<NOT AVAILABLE>");
        }

        for (int i = 0; i < deviceSlots.size(); i++) {
            deviceListStrings.push_back(deviceSlots[i].deviceName);
        }

        float deviceDropdownY = 28;
        
        auto& deviceDropdown = m.prepare<M1DropdownMenu>({7, deviceDropdownY, shape.size.x - 14, 120}).withOptions(deviceListStrings);
        deviceDropdown.withLabelColor(MurkaColor(LABEL_TEXT_COLOR));
        deviceDropdown.withSelectedLabelColor(MurkaColor(ORIENTATION_ACTIVE_COLOR));
        deviceDropdown.withHighlightLabelColor(MurkaColor(BACKGROUND_GREY));
        deviceDropdown.textAlignment = TEXT_LEFT;
        deviceDropdown.optionHeight = 40;
        
        if (!showDeviceSelectionDropdown) {
            MurkaShape dropdownInitShape = MurkaShape(7, deviceDropdownY, shape.size.x - 14, 40);

            m.setColor(DISABLED_PARAM);
            m.enableFill();
            m.drawRectangle(dropdownInitShape);
            
            // `withOutlineColor()` sets the triangle
            // TODO: Make isConnected test include checking the device name
            auto& dropdownInit = m.prepare<M1DropdownButton>(dropdownInitShape).withLabel(deviceSelectedOption).withLabelColor(isConnected ? MurkaColor(ORIENTATION_ACTIVE_COLOR) : MurkaColor(LABEL_TEXT_COLOR)).withOutline(false).withOutlineColor(isConnected ? MurkaColor(ORIENTATION_ACTIVE_COLOR) : MurkaColor(ENABLED_PARAM)).withBackgroundColor(MurkaColor(BACKGROUND_GREY))
                .withTriangle(true);
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
                
                std::vector<M1OrientationDeviceInfo> sourceDevices = orientationClient->getDevices();
                bool foundDevice = false;
                for (int i = 0; i < sourceDevices.size(); i++) {
                    if (sourceDevices[i].getDeviceName() == deviceSelectedOption) {
                        orientationClient->command_startTrackingUsingDevice(sourceDevices[i]);
                        foundDevice = true;
                    }
                }
                
                if (!foundDevice) {
                    orientationClient->command_disconnect();
                }
                
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
    
    std::string deviceSelectedOption = "<SELECT DEVICE>";
    
    bool refreshing = true;
    juce::int64 millisOnStart = 0;
    juce::int64 millisWhenRefreshingStarted = 0;

    M1OrientationClientWindow& withOrientationClient(M1OrientationClient& client) {
        orientationClient = &client;
        return *this;
    }
    
    M1OrientationClientWindow& withDeviceSlots(std::vector<M1OrientationClientWindowDeviceSlot> slots) {
        deviceSlots = slots;
        return *this;
    }
    
    std::string supperwareChirality = "USB ON THE LEFT";

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
    // TODO: GETTER for textfields active
    bool showOscSettings = false;
    int requested_osc_port = 9901; // default value
    std::string requested_osc_msg_address = "/orientation"; // default value
    std::function<void(int, std::string)> oscSettingsChangedCallback;
};
