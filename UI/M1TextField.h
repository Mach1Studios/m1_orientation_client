#pragma once

#include "MurkaView.h"
#include "MurkaBasicWidgets.h"
#include "TextField.h"

using namespace murka;

class M1TextField : public murka::View<M1TextField> {
public:
    void internalDraw(Murka & m) {
        int* data = dataToControl;

        bool isInside = inside() *
//        Had to temporary remove the areInteractiveChildrenHovered because of the bug in Murka with the non-deleting children widgets. TODO: fix this
//        !areInteractiveChildrenHovered(ctx) *
//            hasMouseFocus(m) *
             (!editingTextNow);
        
        changed = false;
        hovered = isInside; // for external views to be highlighted too if needed
        bool hoveredLocal = hovered + externalHover; // shouldn't propel hoveredLocal outside so it doesn't feedback

        if (!enabled) {
            hoveredLocal = false;
        }
        
        m.setColor(100 + 110 * enabled,
                    100 + 110 * enabled,
                    100 + 110 * enabled, 200);
        m.pushStyle();
        m.enableFill();
        
        float width = 1.0;
        
        m.drawRectangle(-width * (4 + A(1 * inside())), 0, width * (8 + A(2 * inside())), shape.size.x * (0.25 + A(0.02 * inside())));
        
        // A white rectangle inside a grey colored one
        m.setColor(100 + 110 * enabled + A(30 * hoveredLocal) * enabled, 255);
        float w = A(width * (1 + 1 * hoveredLocal));
        m.drawRectangle(-w, 0, w * 2, shape.size.x * 0.26);
        
        m.popMatrix();
        m.popStyle();
        
        m.setColor(100 + 110 * enabled + A(30 * hoveredLocal), 255);
        auto labelPositionY = shape.size.x * 0.8 / width + 10;
        std::string displayString = int_to_string(*data);

        std::function<void()> deleteTheTextField = [&]() {
            // Temporary solution to delete the TextField:
            // Searching for an id to delete the text field widget.
            // To be redone after the UI library refactoring.
            
            imIdentifier idToDelete;
            for (auto childTuple: imChildren) {
                auto childIdTuple = childTuple.first;
                if (std::get<1>(childIdTuple) == typeid(TextField).name()) {
                    idToDelete = childIdTuple;
                }
            }
            imChildren.erase(idToDelete);
        };
         
        std::string valueText = displayString;
        auto font = m.getCurrentFont();
        auto valueTextBbox = font->getStringBoundingBox(valueText, 0, 0);

        MurkaShape valueTextShape = { shape.size.x / 2 - valueTextBbox.width / 2 - 5,
                                     shape.size.y * 0.8 / width + 10,
                                     valueTextBbox.width + 10,
                                     valueTextBbox.height };
        
        if (editingTextNow) {
            auto& textFieldObject =
                m.prepare<TextField>({ valueTextShape.x() - 5, valueTextShape.y() - 5,
                    valueTextShape.width() + 10, valueTextShape.height() + 10 })
                .controlling(data)
                .withPrecision(2)
                .forcingEditorToSelectAll(shouldForceEditorToSelectAll)
                .onlyAllowNumbers(true)
                .draw();
            
            auto textFieldEditingFinished = textFieldObject.editingFinished;
            
            if (shouldForceEditorToSelectAll) {
                // We force selection by sending the value to text editor field
                shouldForceEditorToSelectAll = false;
            }
            
            if (!textFieldEditingFinished) {
                textFieldObject.activated = true;
                //ctx.claimKeyboardFocus(&textFieldObject); // TODO: is this still needed?
            }
            
            if (textFieldEditingFinished) {
                editingTextNow = false;
                changed = true;
                deleteTheTextField();
            }
            
        } else {
            m.prepare<murka::Label>({0, shape.size.y * 0.8 / width + 10,
                shape.size.x / width, shape.size.y * 0.5})
                .withAlignment(TEXT_CENTER).text(valueText)
                .draw();
        }
        
        bool hoveredValueText = false;
        if (valueTextShape.inside(mousePosition()) && !editingTextNow && enabled) {
            m.drawRectangle(valueTextShape.x() - 2,
                             valueTextShape.y(),
                             2,
                             2);
            m.drawRectangle(valueTextShape.x() + valueTextShape.width() + 2,
                             valueTextShape.y(),
                             2,
                             2);
            m.drawRectangle(valueTextShape.x() - 2,
                             valueTextShape.y() + valueTextShape.height(),
                             2,
                             2);
            m.drawRectangle(valueTextShape.x() + valueTextShape.width() + 2,
                             valueTextShape.y() + valueTextShape.height(),
                             2,
                             2);
            hoveredValueText = true;
        }
        
        // Action

        if ((mouseDownPressed(0)) && (!isHovered()) && (editingTextNow)) {
            // Pressed outside the knob widget while editing text. Aborting the text edit
            editingTextNow = false;
            deleteTheTextField();
        }
        
        if ((hoveredValueText) && (doubleClick()) && (enabled)) {
            editingTextNow = true;
            shouldForceEditorToSelectAll = true;
        }
    }
    
    std::stringstream converterStringStream;
    std::string int_to_string(int input) {
        converterStringStream.str(std::string());
        converterStringStream << std::fixed << input;
        return (converterStringStream.str());
    }
    
    bool editingTextNow = false;
    bool shouldForceEditorToSelectAll = false;
    
    bool enabled = true;
    bool externalHover = false; // for Pitch wheel to control the knob
    int defaultValue = 0;
    int* dataToControl = nullptr;
    bool changed = false;
    bool hovered = false;

    M1TextField & withParameters(int defaultValue_ = 0,
                                 bool enabled_ = true,
                                 bool externalHover_ = false) {
        defaultValue = defaultValue_;
        enabled = enabled_;
        externalHover = externalHover_;
        return *this;
    }
    
    M1TextField & controlling(int* dataPointer) {
        dataToControl = dataPointer;
        return *this;
    }
};
