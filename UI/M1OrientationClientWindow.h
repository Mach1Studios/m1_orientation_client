

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
        if (refreshing) {
            auto secondsPassed = float(juce::Time::currentTimeMillis() - millisOnStart) / 1000.0;
            for (int i = 0; i < 3; i++) {
                m.drawCircle(shape.size.x / 2 + (shape.size.x / 4) * cos(cos(secondsPassed + 3.14 * 0.5 * float(i)) * 10),
                             shape.size.y / 2 + (shape.size.x / 4) * sin(cos(secondsPassed + 3.14 * 0.5 * float(i)) * 10), 5);
            }
            
            // Fake refresh
            
            auto secondsPassedSinceRefreshStarted = float(juce::Time::currentTimeMillis() - millisWhenRefreshingStarted) / 1000.0;
            if (secondsPassedSinceRefreshStarted > 2) {
                refreshing = false;
                //std::cout << "Refreshing stopped" << std::endl;
            }
        } else {
            // Drawing the window
            float offsetY = 5;
            
            m.prepare<M1SwitchableIconButton>({5, offsetY, shape.size.x - 10, 25}).
                withCaption("REFRESH").withBorders().onClick(
                        [&](M1SwitchableIconButton& b) {
                            startRefreshing();
                        }).withFontSize(12).draw();
            
            offsetY += 35;
            
            // Drawing devices
            for (int i = 0; i < deviceSlots.size(); i++) {
                m.prepare<M1SwitchableIconButton>({2, offsetY, shape.size.x - 4, 30}).
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
                
                offsetY += 30 * i;
            }
            
            // Drawing settings if settings panel is enabled
            if (showSettings) {
                
                if (showOscSettings) {
                    // if OSC active then show UI for changing the input address/port/address_patter
                    
                    offsetY += 30;
                    
                    // IP & PORT BOX
                    m.setColor(40, 40, 40, 255);
                    m.enableFill();
                    m.drawRectangle(5, offsetY, shape.size.x - 10, 25);
                    m.disableFill();
                                        
                    // INPUT IP PORT TEXTFIELD
                    m.setColor(110, 110, 110, 255);
                    auto& ip_port_field = m.prepare<murka::TextField>({shape.size.x - 10 + 5, offsetY, (shape.size.x * 0.33) - 10, 25}).onlyAllowNumbers(true).controlling(&requested_osc_port);
                    ip_port_field.widgetBgColor.a = 0;
                    ip_port_field.drawBounds = false;
                    ip_port_field.hint = "OSC PORT";
                    ip_port_field.draw();
                        
                    offsetY += 30;

                    // INPUT MSG ADDRESS PATTERN TEXTFIELD
                    m.setColor(40, 40, 40, 255);
                    m.enableFill();
                    m.drawRectangle(5, offsetY, shape.size.x - 10, 25);
                    m.disableFill();
                    
                    m.setColor(110, 110, 110, 255);
                    auto& msg_address_pattern_field = m.prepare<murka::TextField>({5, offsetY, shape.size.x - 10, 25}).onlyAllowNumbers(false).controlling(&requested_osc_msg_address);
                    msg_address_pattern_field.widgetBgColor.a = 0;
                    msg_address_pattern_field.drawBounds = false;
                    msg_address_pattern_field.hint = "OSC ADDRESS PATTERN";
                    msg_address_pattern_field.draw();
                    
                    if (ip_port_field.editingFinished || msg_address_pattern_field.editingFinished) {
                        oscSettingsChangedCallback(requested_osc_port, requested_osc_msg_address);
                    }
                }
                
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
    
	M1OrientationClientWindow& onRefreshClicked(std::function<void()> callback) {
		onRefreshCallback = callback;
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
