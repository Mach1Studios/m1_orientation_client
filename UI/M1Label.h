#pragma once

#include "MurkaView.h"
#include "MurkaBasicWidgets.h"

using namespace murka;

class M1Label : public murka::View<M1Label> {
public:
    void internalDraw(Murka & m) {
        
        auto font = m.getCurrentFont();
        if (customFont) {
            font = font;
        }
        
        bool hovered = isHovered();
        
        // Click
        bool clicked = false;
        if (hovered && mouseDownPressed(0)) {
            onClickCallback();
            clicked = true;
        }
        
        MurkaColor bgColor = backgroundColor;
        m.enableFill();
        if ((bgColor.getNormalisedAlpha() != 0.0) && bgFill) {
            if (backgroundColorHoverEnable) {
                m.setColor(hovered ? backgroundColor : backgroundColorUnhovered);
            } else {
                m.setColor(bgColor);
            }
            m.drawRectangle(0, 0, shape.size.x, shape.size.y);
        }
        
        if (clicked && onClickFlash) {
            m.setColor(MurkaColor(220, 220, 220));
            m.drawRectangle(0, 0, shape.size.x, shape.size.y);
        }
        
        if (strokeBorder) {
            if (strokeBorkerHoverEnable) {
                m.setColor(hovered ? strokeBorderColor : strokeBorderColorUnhovered);
            } else {
                m.setColor(strokeBorderColor);
            }
            m.disableFill();
            m.drawLine(0, 0, shape.size.x, 0);
            m.drawLine(0, 0, 0, shape.size.y);
            m.drawLine(shape.size.x, shape.size.y, shape.size.x, 0);
            m.drawLine(shape.size.x, shape.size.y, 0, shape.size.y);
            m.enableFill();
        }
        
        // text color
        MurkaColor fgColor = customColor ? color : m.getColor();
        float anim = enabled ? 40  * A(highlighted) : 0.0;
        fgColor.setRed(fgColor.getRed() + anim - fgColor.getRed() * 0.5 * !enabled);
        fgColor.setGreen(fgColor.getGreen() + anim - fgColor.getGreen() * 0.5 * !enabled);
        fgColor.setBlue(fgColor.getBlue() + anim - fgColor.getBlue() * 0.5 * !enabled);
        m.setColor(fgColor);
        
        if (labelVerticalCentering) {
            // add to the labelPadding_y the text height halved subtracted from the height halved
            label_y_center = shape.size.y/2 - font->getStringBoundingBox(label, 0, 0).height/2;
        }
        
        if (alignment == TEXT_LEFT) {
            font->drawString(label, labelPadding_x, labelPadding_y + label_y_center);
        }
        if (alignment == TEXT_CENTER) {
            float textX = (shape.size.x / 2) - (font->getStringBoundingBox(label, 0, 0).width / 2);
            font->drawString(label, textX, labelPadding_y + label_y_center);
        }
        if (alignment == TEXT_RIGHT) {
            float textX = (shape.size.x - labelPadding_x) - font->getStringBoundingBox(label, 0, 0).width;
            font->drawString(label, textX, labelPadding_y + label_y_center);
        }
    }
    
    // Here go parameters and any parameter convenience constructors. 

    TextAlignment alignment = TEXT_LEFT;
    int labelPadding_x = 5;
    int labelPadding_y = 0;
    float label_y_center = 0;
    bool labelVerticalCentering = false;
    
    M1Label& withTextAlignment(TextAlignment a) {
        alignment = a;
        return *this;
    }
    
    M1Label& withText(std::string text) {
        label = text;
        return *this;
    }
    
    M1Label& withForegroundColor(MurkaColor fgColor) {
        customColor = true;
        color = fgColor;
        return *this;
    }
    
    M1Label& withBackgroundFill(MurkaColor bgFillColor, MurkaColor bgFillColorUnhovered = MurkaColor(0,0,0, 0)) {
        backgroundColor = bgFillColor;
        backgroundColorUnhovered = bgFillColorUnhovered;
        if (bgFillColorUnhovered.getAlpha() > 0) {
            backgroundColorHoverEnable = true;
        } else {
            backgroundColorHoverEnable = false;
        }
        
        bgFill = true;
        return *this;
    }
    
    M1Label& withStrokeBorder(MurkaColor sbColor, MurkaColor sbColorUnhovered = MurkaColor(0,0,0, -1.0)) {
        strokeBorderColor = sbColor;
        if (sbColorUnhovered.getAlpha() > 0) {
            strokeBorderColorUnhovered = sbColorUnhovered;
            strokeBorkerHoverEnable = true;
        } else {
            strokeBorkerHoverEnable = false;
        }
        strokeBorder = true;
        return *this;
    }
    
    M1Label& withVerticalTextOffset(float offset) {
        labelPadding_y = offset;
        return *this;
    }
    
    M1Label& withVerticalTextCentering(bool withVertCenter) {
        labelVerticalCentering = withVertCenter;
        return *this;
    }
    
    M1Label& withOnClickCallback(std::function<void()> callback) {
        onClickCallback = callback;
        return *this;
    }
    
    M1Label& withOnClickFlash() {
        onClickFlash = true;
        return *this;
    }
    
    std::function<void()> onClickCallback = []() {};
    
    bool bgFill = false;
    bool strokeBorder = false;
    bool onClickFlash = false;

    MurkaColor color = { 0.98, 0.98, 0.98 };
    MurkaColor strokeBorderColor = { 0., 0., 0., 0. };
    MurkaColor strokeBorderColorUnhovered = { 0., 0., 0., 0. };
    bool strokeBorkerHoverEnable = false;
    MurkaColor backgroundColor = { 0., 0., 0., 0. };
    MurkaColor backgroundColorUnhovered = { 0., 0., 0., 0. };
    bool backgroundColorHoverEnable = false;

    FontObject* font;

    bool customColor = false;
    bool customFont = false;
    bool enabled = true;
    bool highlighted = false;
    
    std::string label = "";

	// The results type, you also need to define it even if it's nothing.
	bool result;

	virtual bool wantsClicks() override { return false; } // override this if you want to signal that you don't want clicks
};
