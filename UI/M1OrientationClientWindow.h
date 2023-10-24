

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
        m.setColor(40, 40, 40, 255);
        m.enableFill();
        m.drawRectangle(0, 0, shape.size.x, shape.size.y);

        m.setColor(110, 110, 110, 255);
        m.disableFill();
        m.drawRectangle(0, 0, shape.size.x, shape.size.y);
        
        if (!inside() && !areInteractiveChildrenHovered() && mouseDownPressed(0)) {
            onClickOutsideCallback();
        }
        
        m.enableFill();

        // Drawing the window
        float offsetY = 5;
        float oscSettingsOffsetY;
        
        // Drawing devices
        for (int i = 0; i < deviceSlots.size(); i++) {
            m.prepare<M1SwitchableIconButton>({2, offsetY + (30 * i), shape.size.x - 4, 30}).
                withCaption(deviceSlots[i].deviceName)
                .setHighlighted(deviceSlots[i].highlighted)
                .withIconKind(deviceSlots[i].icon)
                .withFontSize(8)
                .onClick([&](M1SwitchableIconButton& b) {
                    deviceSlots[b.elementIndex].onClickCallback(b.elementIndex);
                })
                .withElementIndex(deviceSlots[i].index)
                .withBorders()
                .draw();
            if (deviceSlots[i].deviceName == "OSC Input") {
                // set where we will display the OSC settings under the OSC device
                oscSettingsOffsetY = offsetY + (30 * i) + 30;
                offsetY += 30; // offset the next slot if this happens
            }
        }
        
        // increment up the offset height by number of devices
        offsetY += 30 * deviceSlots.size(); // addds one more slot space for next element

        // Drawing settings if settings panel is enabled
        if (showSettings) {
            
            m.prepare<M1SwitchableIconButton>({2, shape.size.y - 100, shape.size.x - 4, 25})
                .withBorders()
                .withCaption("RECENTER").withFontSize(12).onClick([&](M1SwitchableIconButton& b){ recenterClickedCallback(); })
                .draw();
            
            m.prepare<M1SwitchableIconButton>({2, shape.size.y - 75, shape.size.x - 4, 25})
                .withBorders()
                .withCaption("DISCONNECT").withFontSize(12).onClick([&](M1SwitchableIconButton& b){ disconnectClickedCallback(); })
                .draw();
            
            int yprToggleWidth = int(float(shape.size.x - 10) / 3.0);
            
            m.prepare<M1SwitchableIconButton>({5, shape.size.y - 48, yprToggleWidth, 25})
                .withBorders()
                .withCaption("Y").withFontSize(8).onClick([&](M1SwitchableIconButton& b){ yprSwitchesClickedCallback(0);
                })
                .setHighlighted(trackYaw)
                .draw();

            m.prepare<M1SwitchableIconButton>({5 + yprToggleWidth * 1, shape.size.y - 48, yprToggleWidth, 25})
                .withBorders()
                .withCaption("P").withFontSize(8).onClick([&](M1SwitchableIconButton& b){ yprSwitchesClickedCallback(1);
                })
                .setHighlighted(trackPitch)
                .draw();

            m.prepare<M1SwitchableIconButton>({5 + yprToggleWidth * 2, shape.size.y - 48, yprToggleWidth, 25})
                .withBorders()
                .withCaption("R").withFontSize(8).onClick([&](M1SwitchableIconButton& b){ yprSwitchesClickedCallback(2);
                })
                .setHighlighted(trackRoll)
                .draw();

                // TODO: properly center these
                m.drawString(formatFloatWithLeadingZeros(yaw), 2 + yprToggleWidth/4 + 2, shape.size.y - 20);
                m.drawString(formatFloatWithLeadingZeros(pitch), 2 + yprToggleWidth * 1 + yprToggleWidth/4 + 2, shape.size.y - 20);
                m.drawString(formatFloatWithLeadingZeros(roll), 2 + yprToggleWidth * 2 + yprToggleWidth/4 + 2, shape.size.y - 20);
            
            if (showOscSettings) {
                // if OSC active then show UI for changing the input address/port/address_patter

                int oscDivWidth = int(float(shape.size.x) / 3.0);

                m.setColor(78, 78, 78, 255);
                m.enableFill();
                m.drawRectangle(2, oscSettingsOffsetY, shape.size.x - 4, 30);
                m.disableFill();
                
                // INPUT MSG ADDRESS PATTERN TEXTFIELD
                auto& msg_address_pattern_field = m.prepare<murka::TextField>({2, oscSettingsOffsetY, (oscDivWidth * 2) - 4, 30}).onlyAllowNumbers(false).controlling(&requested_osc_msg_address);
                msg_address_pattern_field.drawBounds = true;
                msg_address_pattern_field.widgetBgColor = {0.3, 0.3, 0.3};
                msg_address_pattern_field.hint = "OSC ADDRESS PATTERN";
                msg_address_pattern_field.draw();

                // INPUT IP PORT TEXTFIELD
                auto& ip_port_field = m.prepare<murka::TextField>({2 + oscDivWidth * 2, oscSettingsOffsetY, oscDivWidth - 4, 30}).onlyAllowNumbers(true).controlling(&requested_osc_port);
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
    
    M1OrientationClientWindow& onClickOutside(std::function<void()> callback) {
        this->onClickOutsideCallback = callback;
        return *this;
    }

    M1OrientationClientWindow& withSettingsPanelEnabled(bool showSettings) {
        this->showSettings = showSettings;
        return *this;
    }
    
    M1OrientationClientWindow& withOscSettingsEnabled(bool showOscSettings) {
        this->showOscSettings = showOscSettings;
        return *this;
    }

    M1OrientationClientWindow& withDeviceList(std::vector<M1OrientationClientWindowDeviceSlot> slots) {
        deviceSlots = slots;
        return *this;
    }
    
	M1OrientationClientWindow& onRecenterClicked(std::function<void()> callback) {
		recenterClickedCallback = callback;
		return *this;
	}

    M1OrientationClientWindow& onDisconnectClicked(std::function<void()> callback) {
        disconnectClickedCallback = callback;
        return *this;
    }

    M1OrientationClientWindow& onOscSettingsChanged(std::function<void(int, std::string)> callback) {
        oscSettingsChangedCallback = callback;
        return *this;
    }

    M1OrientationClientWindow& onYPRSwitchesClicked(std::function<void(int)> callback) {
        yprSwitchesClickedCallback = callback;
        return *this;
    }
    
    M1OrientationClientWindow& withYPRTrackingSettings(bool trackYaw, bool trackPitch, bool trackRoll, std::pair<int, int> yawRange, std::pair<int, int> pitchRange, std::pair<int, int> rollRange) {
        this->trackYaw = trackYaw;
        this->trackPitch = trackPitch;
        this->trackRoll = trackRoll;
        this->yawRange = yawRange;
        this->pitchRange = pitchRange;
        this->rollRange = rollRange;
        return *this;
    }

	M1OrientationClientWindow& withYPR(float yaw, float pitch, float roll) {
		this->yaw = yaw;
		this->pitch = pitch;
		this->roll = roll;
		return *this;
	}

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
    std::function<void(int, std::string)> oscSettingsChangedCallback;
    std::vector<M1OrientationClientWindowDeviceSlot> deviceSlots;
    
	std::function<void()> onClickOutsideCallback = []() {};
	std::function<void()> onRefreshCallback = []() {};
	
    MurImage icon;
    
    bool initialized = false;
    bool showSettings = false;
    bool enabled = true;
    // osc specific
    bool showOscSettings = false;
    int requested_osc_port = 9901; // default value
    std::string requested_osc_msg_address = "/orientation"; // default value

};
