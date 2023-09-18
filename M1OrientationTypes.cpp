#include <JuceHeader.h>
#include "M1OrientationTypes.h"

std::map<enum M1OrientationDeviceType, std::string> M1OrientationDeviceTypeName = {
    { M1OrientationManagerDeviceTypeNone, "Unknown"},
    { M1OrientationManagerDeviceTypeSerial, "Serial"},
    { M1OrientationManagerDeviceTypeBLE, "BLE"},
    { M1OrientationManagerDeviceTypeOSC, "OSC"},
	{ M1OrientationManagerDeviceTypeCamera, "Camera"},
	{ M1OrientationManagerDeviceTypeEmulator, "Emulator"},
    { M1OrientationManagerDeviceTypeFusion, "Fusion"}, // TODO: Add Camera + [any] type
};

std::map<enum M1OrientationStatusType, std::string> M1OrientationStatusTypeName = {
    { M1OrientationManagerStatusTypeError, "Error"},
    { M1OrientationManagerStatusTypeNotConnectable, "Not Connectable"},
    { M1OrientationManagerStatusTypeConnectable, "Connectable"},
    { M1OrientationManagerStatusTypeConnected, "Connected"},
};

void Orientation::setYPR(M1OrientationYPR orientation) {
    // prevent using angleType's other than UNSIGNED_NORMALLED
    // without first setting the min/max values so they can be
    // transformed into UNSIGNED_NORMALLED
    // Please set the angleType and appropriate min/max values!

    if ((int)orientation.angleType == (int)M1OrientationYPR::DEGREES) {
        // normalize
        if (orientation.yaw_min == -180.0f) {
            if (orientation.yaw <= -180.0f) {
                // check we are a remainder of 180 and shift the remainder to the negative half
                orientation.yaw = std::fmod(orientation.yaw, -180.0f);
            }
            if (orientation.yaw >= 0.0f) {
                orientation.yaw = std::fmod(orientation.yaw, 180.0f);
            }
        } else if (orientation.yaw_max == 360.0f) {
            // check we are a remainder of 360 and shift to -180 -> 180
            orientation.yaw = std::fmod(orientation.yaw, 360.0f) - 180.0f;
        } else {
            // assume min/max were not set
            if ((orientation.yaw >= 360. || orientation.yaw < 0) || orientation.yaw_max == 360.) {
                // check we are a remainder of 180 and shift the remainder to the negative half
                orientation.yaw = std::fmod(orientation.yaw, 180.0f) - 180.0f;
            }
        }
        orientationYPR.yaw = (float)juce::jmap(orientation.yaw, -180.0f, 180.0f, -1.0f, 1.0f);
        orientationYPR.pitch = (float)juce::jmap(orientation.pitch, -180.0f, 180.0f, -1.0f, 1.0f);
        orientationYPR.roll = (float)juce::jmap(orientation.roll, -180.0f, 180.0f, -1.0f, 1.0f);
        
    } else if ((int)orientation.angleType == (int)M1OrientationYPR::RADIANS) {
        if ((orientation.yaw >= juce::MathConstants<float>::twoPi || orientation.yaw < 0) || orientation.yaw_max == juce::MathConstants<float>::twoPi) {
            // check we are a remainder of PI and shift the remainder to the negative half
            orientation.yaw = std::fmod(orientation.yaw, juce::MathConstants<float>::pi) - juce::MathConstants<float>::pi;
        }
        orientationYPR.yaw = (float)juce::jmap(orientation.yaw, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, -1.0f, 1.0f);
        orientationYPR.pitch = (float)juce::jmap(orientation.pitch, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, -1.0f, 1.0f);
        orientationYPR.roll = (float)juce::jmap(orientation.roll, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, -1.0f, 1.0f);
        
    } else if ((int)orientation.angleType == (int)M1OrientationYPR::SIGNED_NORMALLED) {        // TODO: test this
        orientationYPR = orientation;
        
    } else if ((int)orientation.angleType == (int)M1OrientationYPR::UNSIGNED_NORMALLED) {
        orientationYPR.yaw = (float)juce::jmap(orientation.yaw, 0.0f, 1.0f, -1.0f, 1.0f);
        orientationYPR.pitch = (float)juce::jmap(orientation.pitch, 0.0f, 1.0f, -1.0f, 1.0f);
        orientationYPR.roll = (float)juce::jmap(orientation.roll, 0.0f, 1.0f, -1.0f, 1.0f);
        
    } else {
        // TODO: error for not defining angle type or default to DEGREES
        DBG("ERROR: no type discovered when setting YPR!");
    }
    
    // update the struct now that the value is normalized
    orientationYPR.angleType = M1OrientationYPR::SIGNED_NORMALLED;
    orientationYPR.yaw_min = -1.0f, orientationYPR.pitch_min = -1.0f, orientationYPR.roll_min = -1.0f;
    orientationYPR.yaw_max = 1.0f, orientationYPR.pitch_max = 1.0f, orientationYPR.roll_max = 1.0f;

    // Below converts YPR SIGNED NORMALLED -> YPR RADIANS -> Quat
    // It is better to avoid this function and stick to updating quat and calculating best YPR
    // Note:
    //   Skipping the setQuat() call and calculating the output directly
    orientationQuat.w = cos((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2) + sin((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2);
    orientationQuat.x = sin((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2) - cos((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2);
    orientationQuat.y = cos((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2) + sin((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2);
    orientationQuat.z = cos((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2) - sin((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2);
}

void Orientation::offsetYPR(M1OrientationYPR offset_delta) {
    // this function assumes an orientation offset delta and apply it safely
    // if you hit this assert its because you are trying to apply an offset that isnt
    // unsigned normalled and potentially not a calculated angle delta

    if ((int)offset_delta.angleType == (int)M1OrientationYPR::SIGNED_NORMALLED) {
        // continue to process
    } else if (offset_delta.angleType == M1OrientationYPR::DEGREES || offset_delta.angleType == M1OrientationYPR::RADIANS) {
        offset_delta.yaw = (float)juce::jmap(offset_delta.yaw, offset_delta.yaw_min, offset_delta.yaw_max, -1.0f, 1.0f);
        offset_delta.pitch = (float)juce::jmap(offset_delta.pitch, offset_delta.pitch_min, offset_delta.pitch_max, -1.0f, 1.0f);
        offset_delta.roll = (float)juce::jmap(offset_delta.roll, offset_delta.roll_min, offset_delta.roll_max, -1.0f, 1.0f);
        offset_delta.angleType = M1OrientationYPR::SIGNED_NORMALLED;
        offset_delta.yaw_min = -1.0f, offset_delta.pitch_min = -1.0f, offset_delta.roll_min = -1.0f;
        offset_delta.yaw_max =  1.0f, offset_delta.pitch_max =  1.0f, offset_delta.roll_max =  1.0f;
    }
    
    if ((int)offset_delta.angleType != (int)M1OrientationYPR::UNSIGNED_NORMALLED) {
        // process if we are not unsigned
        M1OrientationYPR currentOrientation = getYPRasSignedNormalled();
        
        M1OrientationYPR resultingOrientation; // constructs as unsigned normalled
        resultingOrientation = getUnsignedNormalled(currentOrientation + offset_delta);
        resultingOrientation.angleType = M1OrientationYPR::UNSIGNED_NORMALLED;
        resultingOrientation.yaw = std::fmod(resultingOrientation.yaw, 1.0f);
        resultingOrientation.pitch = std::fmod(resultingOrientation.pitch, 1.0f);
        resultingOrientation.roll = std::fmod(resultingOrientation.roll, 1.0f);
        setYPR(resultingOrientation);
    } else {
        DBG("Warning: Tried to offset with unsigned values!");
    }
}

void Orientation::setQuat(M1OrientationQuat orientation) {
    
    //normalize
    auto temp = getNormalled(orientation);
    orientationQuat.wIn = temp.w;
    orientationQuat.xIn = temp.x;
    orientationQuat.yIn = temp.y;
    orientationQuat.zIn = temp.z;

    orientationQuat.w = orientation.wb * orientation.wIn + orientation.xb * orientation.xIn + orientation.yb * orientation.yIn + orientation.zb * orientation.zIn;
    orientationQuat.x = orientation.wb * orientation.xIn - orientation.xb * orientation.wIn - orientation.yb * orientation.zIn + orientation.zb * orientation.yIn;
    orientationQuat.y = orientation.wb * orientation.yIn + orientation.xb * orientation.zIn - orientation.yb * orientation.wIn - orientation.zb * orientation.xIn;
    orientationQuat.z = orientation.wb * orientation.zIn - orientation.xb * orientation.yIn + orientation.yb * orientation.xIn - orientation.zb * orientation.wIn;
    
    //TODO: add logic for reordering and inversing

    float test = orientationQuat.x * orientationQuat.z + orientationQuat.y * orientationQuat.w;
    if (test > 0.499999) {
        // singularity at north pole
        orientationYPR.yaw = 2 * atan2(orientationQuat.x, orientationQuat.w);
        orientationYPR.pitch = juce::MathConstants<double>::pi / 2;
        orientationYPR.roll = 0;
        return;
    } else if (test < -0.499999) {
        // singularity at south pole
        orientationYPR.yaw = -2 * atan2(orientationQuat.x, orientationQuat.w);
        orientationYPR.pitch = -juce::MathConstants<double>::pi / 2;
        orientationYPR.roll = 0;
        return;
    } else {
        float sqx = orientationQuat.x * orientationQuat.x;
        float sqy = orientationQuat.z * orientationQuat.z;
        float sqz = orientationQuat.y * orientationQuat.y;

        float y = atan2(2 * orientationQuat.z * orientationQuat.w - 2 * orientationQuat.x * orientationQuat.y, 1 - 2 * sqy - 2 * sqz);
        float p = asin(2 * test);
        float r = atan2(2 * orientationQuat.x * orientationQuat.w - 2 * orientationQuat.z * orientationQuat.y, 1 - 2 * sqx - 2 * sqz);

        y *= -1.0f;

        // output from above is expected as -PI -> PI
        // converting to signed normalled   -1  -> 1
        orientationYPR.yaw = y / juce::MathConstants<float>::pi;
        orientationYPR.pitch = p / juce::MathConstants<float>::pi;
        orientationYPR.roll = r / juce::MathConstants<float>::pi;
        orientationYPR.angleType = M1OrientationYPR::SIGNED_NORMALLED;
        orientationYPR.yaw_min = -1.0f, orientationYPR.pitch_min = -1.0f, orientationYPR.roll_min = -1.0f;
        orientationYPR.yaw_max = 1.0f, orientationYPR.pitch_max = 1.0f, orientationYPR.roll_max = 1.0f;
    }
}

void Orientation::offsetQuat(M1OrientationQuat offset_delta) {
    // this function assumes an orientation offset delta is calculated and applies it safely
    
    M1OrientationQuat currentOrientation = getQuat();
    
    M1OrientationQuat resultingOrientation;
    resultingOrientation.wIn = currentOrientation.w * offset_delta.w;
    resultingOrientation.xIn = currentOrientation.x * offset_delta.x;
    resultingOrientation.yIn = currentOrientation.y * offset_delta.y;
    resultingOrientation.zIn = currentOrientation.z * offset_delta.z;
    setQuat(resultingOrientation);
}

M1OrientationYPR Orientation::getYPRasUnsignedNormalled() {
    M1OrientationYPR tempOrientation;
    if (orientationYPR.angleType == M1OrientationYPR::UNSIGNED_NORMALLED) {
        // TODO: test this
        tempOrientation = orientationYPR;
    } else {
        // shift yaw negative range to make 0deg = 0norm
        tempOrientation.yaw = (float)juce::jmap(orientationYPR.yaw + std::abs(orientationYPR.yaw_min), orientationYPR.yaw_min, orientationYPR.yaw_max, 0.0f, 1.0f);
        tempOrientation.pitch = (float)juce::jmap(orientationYPR.pitch, orientationYPR.pitch_min, orientationYPR.pitch_max, 0.0f, 1.0f);
        tempOrientation.roll = (float)juce::jmap(orientationYPR.roll, orientationYPR.roll_min, orientationYPR.roll_max, 0.0f, 1.0f);
        // skip setting min/max values since we will discard offset
    }
    return tempOrientation;
}

M1OrientationYPR Orientation::getYPRasSignedNormalled() {
    M1OrientationYPR tempOrientation;
    if (orientationYPR.angleType == M1OrientationYPR::SIGNED_NORMALLED) {
        // TODO: test this
        tempOrientation = orientationYPR;
    } else {
        tempOrientation.yaw = (float)juce::jmap(orientationYPR.yaw, orientationYPR.yaw_min, orientationYPR.yaw_max, -1.0f, 1.0f);
        tempOrientation.pitch = (float)juce::jmap(orientationYPR.pitch, orientationYPR.pitch_min, orientationYPR.pitch_max, -1.0f, 1.0f);
        tempOrientation.roll = (float)juce::jmap(orientationYPR.roll, orientationYPR.roll_min, orientationYPR.roll_max, -1.0f, 1.0f);
        // skip setting min/max values since we will discard offset
    }
    return tempOrientation;
}

M1OrientationYPR Orientation::getYPRinDegrees() {
    M1OrientationYPR tempOrientation;
    if (orientationYPR.angleType == M1OrientationYPR::SIGNED_NORMALLED) {
        // TODO: test this
        tempOrientation = orientationYPR;
    } else {
        tempOrientation = getYPRasSignedNormalled(); // sets and converts back to signed normalled
    }
    tempOrientation.yaw = (float)juce::jmap(tempOrientation.yaw, -1.0f, 1.0f, -180.0f, 180.0f);
    tempOrientation.pitch = (float)juce::jmap(tempOrientation.pitch, -1.0f, 1.0f, -180.0f, 180.0f);
    tempOrientation.roll = (float)juce::jmap(tempOrientation.roll, -1.0f, 1.0f, -180.0f, 180.0f);
    return tempOrientation;
}

M1OrientationYPR Orientation::getYPRinRadians() {
    M1OrientationYPR tempOrientation;
    if (orientationYPR.angleType == M1OrientationYPR::SIGNED_NORMALLED) {
        // TODO: test this
        tempOrientation = orientationYPR;
    } else {
        tempOrientation = getYPRasSignedNormalled(); // sets and converts back to signed normalled
    }
    tempOrientation.yaw = (float)juce::jmap(orientationYPR.yaw, -1.0f, 1.0f, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi);
    tempOrientation.pitch = (float)juce::jmap(orientationYPR.pitch, -1.0f, 1.0f, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi);
    tempOrientation.roll = (float)juce::jmap(orientationYPR.roll, -1.0f, 1.0f, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi);
    return tempOrientation;
}

M1OrientationQuat Orientation::getQuat() {
    return orientationQuat;
}

void Orientation::resetOrientation() {
    //M1OrientationYPR zeroed;
    //setYPR(zeroed);
    
    M1OrientationQuat quat = getQuat();
    
    quat.wb = quat.wIn;
    quat.xb = quat.xIn;
    quat.yb = quat.yIn;
    quat.zb = quat.zIn;
    
    setQuat(quat);
}

M1OrientationYPR getUnsignedNormalled(M1OrientationYPR orientation) {
    M1OrientationYPR normalised_orientation;
    normalised_orientation.yaw = (float)juce::jmap(orientation.yaw, orientation.yaw_min , orientation.yaw_max, 0.0f, 1.0f);
    normalised_orientation.pitch = (float)juce::jmap(orientation.pitch, orientation.pitch_min , orientation.pitch_max, 0.0f, 1.0f);
    normalised_orientation.roll = (float)juce::jmap(orientation.roll, orientation.roll_min , orientation.roll_max, 0.0f, 1.0f);
    normalised_orientation.angleType = M1OrientationYPR::UNSIGNED_NORMALLED;
    normalised_orientation.yaw_min = 0.0f, normalised_orientation.pitch_min = 0.0f, normalised_orientation.roll_min = 0.0f;
    normalised_orientation.yaw_max = 1.0f, normalised_orientation.pitch_max = 1.0f, normalised_orientation.roll_max = 1.0f;
    return normalised_orientation;
}

M1OrientationQuat getNormalled(M1OrientationQuat orientation) {
    // if set with `In` values, use them instead
    if (orientation.w != orientation.wIn) orientation.w = orientation.wIn;
    if (orientation.x != orientation.wIn) orientation.x = orientation.xIn;
    if (orientation.y != orientation.wIn) orientation.y = orientation.yIn;
    if (orientation.z != orientation.wIn) orientation.z = orientation.zIn;

    double magnitude = sqrt(orientation.w * orientation.w + orientation.x * orientation.x + orientation.y * orientation.y + orientation.z * orientation.z);
    orientation.w /= magnitude;
    orientation.x /= magnitude;
    orientation.y /= magnitude;
    orientation.z /= magnitude;
    return orientation;
}
