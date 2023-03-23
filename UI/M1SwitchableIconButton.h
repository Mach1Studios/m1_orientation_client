
#pragma once

#include "MurkaView.h"
#include "MurkaBasicWidgets.h"
#include "M1Label.h"
#include "BinaryData.h"

using namespace murka;

class M1SwitchableIconButton : public murka::View<M1SwitchableIconButton> {
public:
    M1SwitchableIconButton() {
    }
    
    void internalDraw(Murka & m) {
        MurkaContext& ctx = m.currentContext;

        bool inside = ctx.isHovered() * !areInteractiveChildrenHovered(ctx) * hasMouseFocus(m);
        
        m.setColor(40 + 40 * highlighted + 15 * inside,
                   40 + 40 * highlighted + 15 * inside,
                   40 + 40 * highlighted + 15 * inside, 255);
        m.enableFill();
        m.drawRectangle(0, 0, shape.size.x, shape.size.y);

        m.setColor(110 + 200 * inside,
                   110 + 200 * inside,
                   110 + 200 * inside, 255);
        m.disableFill();
        if (drawBorders)
            m.drawRectangle(0, 0, shape.size.x, shape.size.y);
        
        m.setColor(140, 255);
        m.enableFill();
        if (iconKind != "") {
            if (!iconLoaded) {
                if (iconKind == "bt") {
                    icon.loadFromRawData(BinaryData::bluetooth_icon_png, BinaryData::bluetooth_icon_pngSize);
                    iconLoaded = true;
                } else
                if (iconKind == "wifi") {
                    icon.loadFromRawData(BinaryData::wifi_icon_png, BinaryData::wifi_icon_pngSize);
                    iconLoaded = true;
                } else {
                    std::cout << "UNKNOWN ICON TYPE";
                }
            }
            m.drawImage(icon, 5, 5, shape.size.y - 10, shape.size.y - 10);
            m.prepare<M1Label>({shape.size.y, 5, shape.size.x - shape.size.y - 10, shape.size.y - 10}).withText(caption).withTextAlignment(TEXT_CENTER).commit();
        } else {
            m.prepare<M1Label>({5, 5, shape.size.x - 10, shape.size.y - 10}).withText(caption).withTextAlignment(TEXT_CENTER).commit();
        }
        
        if (inside && ctx.mouseDownPressed[0]) {
            onClickCallback(*this);
        }
    }
    
    M1SwitchableIconButton& onClick(std::function<void(M1SwitchableIconButton&)> callback) {
        onClickCallback = callback;
        return *this;
    }
    
    std::function<void(M1SwitchableIconButton&)> onClickCallback = [](M1SwitchableIconButton& b){};
    
    M1SwitchableIconButton& withIconKind(std::string iconKind_) {
        iconKind = iconKind_;
        return *this;
    }

    M1SwitchableIconButton& withCaption(std::string caption_) {
        caption = caption_;
        return *this;
    }

    M1SwitchableIconButton& withBorders() {
        drawBorders = true;
        return *this;
    }

    M1SwitchableIconButton& withFontSize(int fs) {
        fontSize = fs;
        return *this;
    }

    M1SwitchableIconButton& withElementIndex(int index) {
        elementIndex = index;
        return *this;
    }

    M1SwitchableIconButton& setHighlighted(bool highlighted_) {
        highlighted = highlighted_;
        return *this;
    }

    int fontSize = 10; 
    bool drawBorders = false;
    int elementIndex = -1;
    bool highlighted = false;
    std::string iconKind = "";
    std::string caption = "default";
    
    MurImage icon;
    
    bool iconLoaded = false;
    
    bool enabled = true;
};
