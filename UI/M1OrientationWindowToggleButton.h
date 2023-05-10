
#pragma once

#include "MurkaView.h"
#include "MurkaBasicWidgets.h"
#include "BinaryData.h"

using namespace murka;

class M1OrientationWindowToggleButton : public murka::View<M1OrientationWindowToggleButton> {
public:
    float degToRad(double deg) {
        return deg * (juce::MathConstants<float>::pi / 180);
    }
    
    M1OrientationWindowToggleButton() {
        icon.loadFromRawData(BinaryData::tech_support_icon_png, BinaryData::tech_support_icon_pngSize);
    }
    
    void internalDraw(Murka & m) {
        hovered = inside();
        
        m.setColor(40 + 15 * inside(),
                   40 + 15 * inside(),
                   40 + 15 * inside(), 255);
        m.enableFill();
        m.drawRectangle(0, 0, shape.size.x, shape.size.y);

        m.setColor(110 + 200 * inside(),
                   110 + 200 * inside(),
                   110 + 200 * inside(), 255);
        m.disableFill();
        m.drawRectangle(0, 0, shape.size.x, shape.size.y);
        
        m.setColor(140, 255);
        m.enableFill();
        if (!showGimmick) {
            m.drawImage(icon, 5, 5, shape.size.x - 10, shape.size.y - 10);
        } else {
            // TODO: show monitor yaw iconology?
            m.setColor(220, 220, 220, 255);
            m.drawCircle(shape.size.x / 2 + cos(degToRad(gimmickAngleDegrees)) * (shape.size.x - 5) / 2,
                         shape.size.y / 2 + sin(degToRad(gimmickAngleDegrees)) * (shape.size.y - 5) / 2,
                         2);
        }
        
        if (inside() && mouseDownPressed(0)) {
            onClickCallback(*this);
        }
    }
    
    M1OrientationWindowToggleButton& onClick(std::function<void(M1OrientationWindowToggleButton&)> callback) {
        onClickCallback = callback;
        return *this;
    }

    M1OrientationWindowToggleButton& withInteractiveOrientationGimmick(bool showGimmick, double gimmickAngleDegrees) {
        this->gimmickAngleDegrees = gimmickAngleDegrees;
        this->showGimmick = showGimmick;
        return *this;
    }

    std::function<void(M1OrientationWindowToggleButton&)> onClickCallback = [](M1OrientationWindowToggleButton& b){};
    
    MurImage icon;
    
    bool showGimmick = false;
    double gimmickAngleDegrees = 0;
    bool initialized = false;
    
    bool enabled = true;
    bool hovered = false;
};
