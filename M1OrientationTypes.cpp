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
    //assert(orientation.angleType != M1OrientationYPR::AngleType::UNSIGNED_NORMALLED &&
    //       (orientation.yaw_min == 0.0f && orientation.yaw_max == 1.0f) ||
    //       (orientation.pitch_min == 0.0f && orientation.pitch_max == 1.0f) ||
    //       (orientation.roll_min == 0.0f && orientation.roll_max == 1.0f));
    
    if (orientation.angleType == M1OrientationYPR::AngleType::DEGREES) {
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
        orientationYPR.yaw = (float)juce::jmap(orientation.yaw, (float)-180.0, (float)180.0, (float)0.0, (float)1.0);
        orientationYPR.pitch = (float)juce::jmap(orientation.pitch, (float)-180.0, (float)180.0, (float)0.0, (float)1.0);
        orientationYPR.roll = (float)juce::jmap(orientation.roll, (float)-180.0, (float)180.0, (float)0.0, (float)1.0);
        
    } else if (orientation.angleType == M1OrientationYPR::AngleType::RADIANS) {
        if ((orientation.yaw >= juce::MathConstants<float>::twoPi || orientation.yaw < 0) || orientation.yaw_max == juce::MathConstants<float>::twoPi) {
            // check we are a remainder of PI and shift the remainder to the negative half
            orientation.yaw = std::fmod(orientation.yaw, juce::MathConstants<float>::pi) - juce::MathConstants<float>::pi;
        }
        orientationYPR.yaw = (float)juce::jmap(orientation.yaw, (float)-juce::MathConstants<float>::pi, (float)juce::MathConstants<float>::pi, (float)0.0, (float)1.0);
        orientationYPR.pitch = (float)juce::jmap(orientation.pitch, (float)-juce::MathConstants<float>::pi, (float)juce::MathConstants<float>::pi, (float)0.0, (float)1.0);
        orientationYPR.roll = (float)juce::jmap(orientation.roll, (float)-juce::MathConstants<float>::pi, (float)juce::MathConstants<float>::pi, (float)0.0, (float)1.0);
        
    } else if (orientation.angleType == M1OrientationYPR::AngleType::SIGNED_NORMALLED) {
        orientationYPR.yaw = (float)juce::jmap(orientation.yaw, (float)-1.0, (float)1.0, (float)0.0, (float)1.0);
        orientationYPR.pitch = (float)juce::jmap(orientation.pitch, (float)-1.0, (float)1.0, (float)0.0, (float)1.0);
        orientationYPR.roll = (float)juce::jmap(orientation.roll, (float)-1.0, (float)1.0, (float)0.0, (float)1.0);
        
    } else if (orientation.angleType == M1OrientationYPR::AngleType::UNSIGNED_NORMALLED) {
        // TODO: test this
        orientationYPR = orientation;
        
    } else {
        // TODO: error for not defining angle type or default to DEGREES
        DBG("ERROR: no type discovered when setting YPR!");
    }
    
    // update the struct now that the value is normalized
    orientationYPR.angleType == M1OrientationYPR::UNSIGNED_NORMALLED;
    orientationYPR.yaw_min = 0.0f, orientationYPR.pitch_min = 0.0f, orientationYPR.roll_min = 0.0f;
    orientationYPR.yaw_max = 1.0f, orientationYPR.pitch_max = 1.0f, orientationYPR.roll_max = 1.0f;

    // Below converts YPR UNSIGNED NORMALLED -> SIGNED YPR RADIANS -> Quat
    // It is better to avoid this function and stick to updating quat and calculating best YPR
    // Note:
    //   Skipping the setQuat() call and calculating the output directly
    
    float yaw = (juce::MathConstants<float>::twoPi * orientationYPR.yaw) - juce::MathConstants<float>::pi;
    float pitch = (juce::MathConstants<float>::twoPi * orientationYPR.pitch) - juce::MathConstants<float>::pi;
    float roll = (juce::MathConstants<float>::twoPi * orientationYPR.roll) - juce::MathConstants<float>::pi;
    
    orientationQuat.w = cos((roll) / 2) * cos((pitch) / 2) * cos((yaw) / 2) + sin((roll) / 2) * sin((pitch) / 2) * sin((yaw) / 2);
    orientationQuat.x = sin((roll) / 2) * cos((pitch) / 2) * cos((yaw) / 2) - cos((roll) / 2) * sin((pitch) / 2) * sin((yaw) / 2);
    orientationQuat.y = cos((roll) / 2) * sin((pitch) / 2) * cos((yaw) / 2) + sin((roll) / 2) * cos((pitch) / 2) * sin((yaw) / 2);
    orientationQuat.z = cos((roll) / 2) * cos((pitch) / 2) * sin((yaw) / 2) - sin((roll) / 2) * sin((pitch) / 2) * cos((yaw) / 2);
}

void Orientation::offsetYPR(M1OrientationYPR offset_delta) {
    // this function assumes an orientation offset delta and apply it safely
    // if you hit this assert its because you are trying to apply an offset that isnt
    // unsigned normalled and potentially not a calculated angle delta
    //assert(offset_delta.angleType != M1OrientationYPR::AngleType::UNSIGNED_NORMALLED);
    
    M1OrientationYPR currentOrientation = getYPRasUnsignedNormalled();
    
    M1OrientationYPR resultingOrientation;
    resultingOrientation.angleType = M1OrientationYPR::UNSIGNED_NORMALLED;
    orientationYPR.yaw_min = 0.0f, orientationYPR.pitch_min = 0.0f, orientationYPR.roll_min = 0.0f;
    orientationYPR.yaw_max = 1.0f, orientationYPR.pitch_max = 1.0f, orientationYPR.roll_max = 1.0f;
    resultingOrientation.yaw = std::fmod(currentOrientation.yaw + offset_delta.yaw, 1.0f);
    resultingOrientation.pitch = std::fmod(currentOrientation.yaw + offset_delta.pitch, 1.0f);
    resultingOrientation.roll = std::fmod(currentOrientation.yaw + offset_delta.roll, 1.0f);
    setYPR(resultingOrientation);
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
        orientationYPR.pitch = juce::MathConstants<float>::pi / 2;
        orientationYPR.roll = 0;
        return;
    } else if (test < -0.499999) {
        // singularity at south pole
        orientationYPR.yaw = -2 * atan2(orientationQuat.x, orientationQuat.w);
        orientationYPR.pitch = -juce::MathConstants<float>::pi / 2;
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
    }
    // shift to unsigned normalled 0->1
    orientationYPR.yaw = (orientationYPR.yaw + 1.0f) / 2;
    orientationYPR.pitch =  (orientationYPR.pitch + 1.0f) / 2;
    orientationYPR.roll =  (orientationYPR.roll + 1.0f) / 2;
    orientationYPR.angleType = M1OrientationYPR::AngleType::UNSIGNED_NORMALLED;
    orientationYPR.yaw_min = 0.0f, orientationYPR.pitch_min = 0.0f, orientationYPR.roll_min = 0.0f;
    orientationYPR.yaw_max = 1.0f, orientationYPR.pitch_max = 1.0f, orientationYPR.roll_max = 1.0f;
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
    if (orientationYPR.angleType == M1OrientationYPR::AngleType::UNSIGNED_NORMALLED) {
        // TODO: test this
        tempOrientation = orientationYPR;
    } else {
        // shift yaw negative range to make 0deg = 0norm
        tempOrientation.yaw = (float)juce::jmap(orientationYPR.yaw + std::abs((float)orientationYPR.yaw_min), (float)orientationYPR.yaw_min, (float)orientationYPR.yaw_max, (float)0.0, (float)1.0);
        tempOrientation.pitch = (float)juce::jmap(orientationYPR.pitch, (float)orientationYPR.pitch_min, (float)orientationYPR.pitch_max, (float)0.0, (float)1.0);
        tempOrientation.roll = (float)juce::jmap(orientationYPR.roll, (float)orientationYPR.roll_min, (float)orientationYPR.roll_max, (float)0.0, (float)1.0);
        // skip setting min/max values since we will discard offset
    }
    return tempOrientation;
}

M1OrientationYPR Orientation::getYPRasSignedNormalled() {
    M1OrientationYPR tempOrientation;
    if (orientationYPR.angleType == M1OrientationYPR::AngleType::SIGNED_NORMALLED) {
        // TODO: test this
        tempOrientation = orientationYPR;
    } else {
        tempOrientation.yaw = (float)juce::jmap(orientationYPR.yaw, (float)orientationYPR.yaw_min, (float)orientationYPR.yaw_max, (float)-1.0, (float)1.0);
        tempOrientation.pitch = (float)juce::jmap(orientationYPR.pitch, (float)orientationYPR.pitch_min, (float)orientationYPR.pitch_max, (float)-1.0, (float)1.0);
        tempOrientation.roll = (float)juce::jmap(orientationYPR.roll, (float)orientationYPR.roll_min, (float)orientationYPR.roll_max, (float)-1.0, (float)1.0);
        // skip setting min/max values since we will discard offset
    }
    return tempOrientation;
}

M1OrientationYPR Orientation::getYPRinDegrees() {
    M1OrientationYPR tempOrientation;
    if (orientationYPR.angleType == M1OrientationYPR::AngleType::SIGNED_NORMALLED) {
        // TODO: test this
    } else {
        tempOrientation = getYPRasSignedNormalled(); // sets and converts back to signed normalled
    }
    tempOrientation.yaw = (float)juce::jmap(orientationYPR.yaw, (float)-1.0, (float)1.0, (float)-180.0, (float)180.0);
    tempOrientation.pitch = (float)juce::jmap(orientationYPR.pitch, (float)-1.0, (float)1.0, (float)-180.0, (float)180.0);
    tempOrientation.roll = (float)juce::jmap(orientationYPR.roll, (float)-1.0, (float)1.0, (float)-180.0, (float)180.0);
    return tempOrientation;
}

M1OrientationYPR Orientation::getYPRinRadians() {
    M1OrientationYPR tempOrientation;
    if (orientationYPR.angleType == M1OrientationYPR::AngleType::SIGNED_NORMALLED) {
        // TODO: test this
    } else {
        tempOrientation = getYPRasSignedNormalled(); // sets and converts back to signed normalled
    }
    tempOrientation.yaw = (float)juce::jmap(orientationYPR.yaw, (float)-1.0, (float)1.0, (float)-juce::MathConstants<float>::pi, (float)juce::MathConstants<float>::pi);
    tempOrientation.pitch = (float)juce::jmap(orientationYPR.pitch, (float)-1.0, (float)1.0, (float)-juce::MathConstants<float>::pi, (float)juce::MathConstants<float>::pi);
    tempOrientation.roll = (float)juce::jmap(orientationYPR.roll, (float)-1.0, (float)1.0, (float)-juce::MathConstants<float>::pi, (float)juce::MathConstants<float>::pi);
    return tempOrientation;
}

M1OrientationYPR Orientation::getUnsignedNormalled(M1OrientationYPR orientation) {
    M1OrientationYPR normalised_orientation;
    normalised_orientation.angleType = M1OrientationYPR::UNSIGNED_NORMALLED;
    normalised_orientation.yaw = (float)juce::jmap(orientation.yaw, (float)orientation.yaw_min , (float)orientation.yaw_max, (float)0.0, (float)1.0);
    normalised_orientation.pitch = (float)juce::jmap(orientation.pitch, (float)orientation.pitch_min , (float)orientation.pitch_max, (float)0.0, (float)1.0);
    normalised_orientation.roll = (float)juce::jmap(orientation.roll, (float)orientation.roll_min , (float)orientation.roll_max, (float)0.0, (float)1.0);
    return normalised_orientation;
}

M1OrientationQuat Orientation::getNormalled(M1OrientationQuat orientation) {
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

M1OrientationQuat Orientation::getQuat() {
    return orientationQuat;
}

void Orientation::resetOrientation() {
    M1OrientationQuat quat = getQuat();
    
    quat.wb = quat.w;
    quat.xb = quat.x;
    quat.yb = quat.y;
    quat.zb = quat.z;
    
    setQuat(quat);
}
