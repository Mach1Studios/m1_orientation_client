#include <JuceHeader.h>
#include "M1OrientationTypes.h"

std::map<enum M1OrientationDeviceType, std::string> M1OrientationDeviceTypeName = {
    { M1OrientationManagerDeviceTypeNone, "Unknown"},
    { M1OrientationManagerDeviceTypeSerial, "Serial"},
    { M1OrientationManagerDeviceTypeBLE, "BLE"},
    { M1OrientationManagerDeviceTypeOSC, "OSC"},
	{ M1OrientationManagerDeviceTypeCamera, "Camera"},
	{ M1OrientationManagerDeviceTypeEmulator, "Emulator"},
};

std::map<enum M1OrientationStatusType, std::string> M1OrientationStatusTypeName = {
    { M1OrientationManagerStatusTypeError, "Error"},
    { M1OrientationManagerStatusTypeNotConnectable, "Not Connectable"},
    { M1OrientationManagerStatusTypeConnectable, "Connectable"},
    { M1OrientationManagerStatusTypeConnected, "Connected"},
};

void Orientation::setYPR(M1OrientationYPR orientation) {
    if (orientation.angleType == M1OrientationYPR::AngleType::DEGREES) {
        // passthrough for conversion to Quat
        orientationYPR = orientation;
    } else if (orientation.angleType == M1OrientationYPR::AngleType::RADIANS) {
        // convert to degrees for conversion to Quat
        orientationYPR.yaw = juce::radiansToDegrees(orientation.yaw);
        orientationYPR.pitch = juce::radiansToDegrees(orientation.pitch);
        orientationYPR.roll = juce::radiansToDegrees(orientation.roll);
        orientationYPR.angleType = M1OrientationYPR::AngleType::DEGREES;
    } else if (orientation.angleType == M1OrientationYPR::AngleType::NORMALED) {
        // convert to degrees for conversion to Quat
        orientationYPR.yaw = (float)juce::jmap(orientation.yaw, (float)0.0, (float)1.0, (float)-180, (float)180);
        orientationYPR.pitch = (float)juce::jmap(orientation.pitch, (float)0.0, (float)1.0, (float)-180, (float)180);
        orientationYPR.roll = (float)juce::jmap(orientation.roll, (float)0.0, (float)1.0, (float)-180, (float)180);
        orientationYPR.angleType = M1OrientationYPR::AngleType::DEGREES;
    } else {
        // TODO: error for not defining angle type or default to DEGREES
        DBG("ERROR: no type discovered when setting YPR!");
        orientationYPR = orientation;
    }

    // Below converts YPR DEGREES -> YPR RADIANS -> Quat
    // It is better to avoid this function and stick to updating quat and calculating best YPR
    orientationQuat.w = cos(juce::degreesToRadians(orientationYPR.roll) / 2) * cos(juce::degreesToRadians(orientationYPR.pitch) / 2) * cos(juce::degreesToRadians(orientationYPR.yaw) / 2) + sin(juce::degreesToRadians(orientationYPR.roll) / 2) * sin(juce::degreesToRadians(orientationYPR.pitch) / 2) * sin(juce::degreesToRadians(orientationYPR.yaw) / 2);
    orientationQuat.x = sin(juce::degreesToRadians(orientationYPR.roll) / 2) * cos(juce::degreesToRadians(orientationYPR.pitch) / 2) * cos(juce::degreesToRadians(orientationYPR.yaw) / 2) - cos(juce::degreesToRadians(orientationYPR.roll) / 2) * sin(juce::degreesToRadians(orientationYPR.pitch) / 2) * sin(juce::degreesToRadians(orientationYPR.yaw) / 2);
    orientationQuat.y = cos(juce::degreesToRadians(orientationYPR.roll) / 2) * sin(juce::degreesToRadians(orientationYPR.pitch) / 2) * cos(juce::degreesToRadians(orientationYPR.yaw) / 2) + sin(juce::degreesToRadians(orientationYPR.roll) / 2) * cos(juce::degreesToRadians(orientationYPR.pitch) / 2) * sin(juce::degreesToRadians(orientationYPR.yaw) / 2);
    orientationQuat.z = cos(juce::degreesToRadians(orientationYPR.roll) / 2) * cos(juce::degreesToRadians(orientationYPR.pitch) / 2) * sin(juce::degreesToRadians(orientationYPR.yaw) / 2) - sin(juce::degreesToRadians(orientationYPR.roll) / 2) * sin(juce::degreesToRadians(orientationYPR.pitch) / 2) * cos(juce::degreesToRadians(orientationYPR.yaw) / 2);
}

void Orientation::offsetYPR(M1OrientationYPR offset) {
    M1OrientationYPR currentOrientation = getYPR();
    M1OrientationYPR resultingOrientation;

    if (currentOrientation.angleType != offset.angleType) {
        // mismatch of types, we need to ensure the offset is added properly
        // convert to degrees to match default
        // Warning: Requires properly set min/max values per type
        if (currentOrientation.angleType == M1OrientationYPR::AngleType::DEGREES) {
            if (offset.angleType == M1OrientationYPR::AngleType::NORMALED) {
                offset.yaw = (float)juce::jmap(offset.yaw, (float)offset.yaw_min, (float)offset.yaw_max, (float)-180, (float)180);
                offset.pitch = (float)juce::jmap(offset.pitch, (float)offset.pitch_min, (float)offset.pitch_max, (float)-180, (float)180);
                offset.roll = (float)juce::jmap(offset.roll, (float)offset.roll_min, (float)offset.roll_max, (float)-180, (float)180);
                offset.angleType = M1OrientationYPR::AngleType::DEGREES;
                // skip setting min/max values since we will discard offset
            } else if (offset.angleType == M1OrientationYPR::AngleType::RADIANS) {
                offset.yaw = juce::radiansToDegrees(offset.yaw);
                offset.pitch = juce::radiansToDegrees(offset.pitch);
                offset.roll = juce::radiansToDegrees(offset.roll);
                offset.angleType = M1OrientationYPR::AngleType::DEGREES;
                // skip setting min/max values since we will discard offset
            }
        } else if (currentOrientation.angleType == M1OrientationYPR::AngleType::NORMALED) {
            offset.yaw = (float)juce::jmap(offset.yaw, (float)offset.yaw_min, (float)offset.yaw_max, (float)0.0, (float)1.0);
            offset.pitch = (float)juce::jmap(offset.pitch, (float)offset.pitch_min, (float)offset.pitch_max, (float)0.0, (float)1.0);
            offset.roll = (float)juce::jmap(offset.roll, (float)offset.roll_min, (float)offset.roll_max, (float)0.0, (float)1.0);
            offset.angleType = M1OrientationYPR::AngleType::NORMALED;
            // skip setting min/max values since we will discard offset
        } else if (currentOrientation.angleType == M1OrientationYPR::AngleType::RADIANS) {
            if (offset.angleType == M1OrientationYPR::AngleType::DEGREES) {
                offset.yaw = juce::degreesToRadians(offset.yaw);
                offset.pitch = juce::degreesToRadians(offset.pitch);
                offset.roll = juce::degreesToRadians(offset.roll);
                offset.angleType = M1OrientationYPR::AngleType::RADIANS;
                // skip setting min/max values since we will discard offset
            } else if (offset.angleType == M1OrientationYPR::AngleType::NORMALED) {
                offset.yaw = (float)juce::jmap(offset.yaw, (float)offset.yaw_min, (float)offset.yaw_max, (float)-juce::MathConstants<float>::pi, (float)juce::MathConstants<float>::pi);
                offset.pitch = (float)juce::jmap(offset.pitch, (float)offset.pitch_min, (float)offset.pitch_max, (float)-juce::MathConstants<float>::pi, (float)juce::MathConstants<float>::pi);
                offset.roll = (float)juce::jmap(offset.roll, (float)offset.roll_min, (float)offset.roll_max, (float)-juce::MathConstants<float>::pi, (float)juce::MathConstants<float>::pi);
                offset.angleType = M1OrientationYPR::AngleType::RADIANS;
                // skip setting min/max values since we will discard offset
            }
        }
    }
    
    // now that we have matching types we can apply offset
    resultingOrientation.yaw = currentOrientation.yaw + offset.yaw;
    resultingOrientation.pitch = currentOrientation.yaw + offset.pitch;
    resultingOrientation.roll = currentOrientation.yaw + offset.roll;
    setYPR(resultingOrientation);
}

void Orientation::setQuat(M1OrientationQuat orientation) {
    
    //normalize
    auto temp = getNormalised(orientation);
    orientationQuat.wIn = temp.w;
    orientationQuat.xIn = temp.x;
    orientationQuat.yIn = temp.y;
    orientationQuat.zIn = temp.z;

    orientationQuat.w = orientation.wb * orientation.wIn + orientation.xb * orientation.xIn + orientation.yb * orientation.yIn + orientation.zb * orientation.zIn;
    orientationQuat.x = orientation.wb * orientation.xIn - orientation.xb * orientation.wIn - orientation.yb * orientation.zIn + orientation.zb * orientation.yIn;
    orientationQuat.y = orientation.wb * orientation.yIn + orientation.xb * orientation.zIn - orientation.yb * orientation.wIn - orientation.zb * orientation.xIn;
    orientationQuat.z = orientation.wb * orientation.zIn - orientation.xb * orientation.yIn + orientation.yb * orientation.xIn - orientation.zb * orientation.wIn;
    
    //TODO: add logic for reordering and inversing

    double test = orientationQuat.x * orientationQuat.z + orientationQuat.y * orientationQuat.w;
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
        double sqx = orientationQuat.x * orientationQuat.x;
        double sqy = orientationQuat.z * orientationQuat.z;
        double sqz = orientationQuat.y * orientationQuat.y;

        float y = atan2(2 * orientationQuat.z * orientationQuat.w - 2 * orientationQuat.x * orientationQuat.y, 1 - 2 * sqy - 2 * sqz);
        float p = asin(2 * test);
        float r = atan2(2 * orientationQuat.x * orientationQuat.w - 2 * orientationQuat.z * orientationQuat.y, 1 - 2 * sqx - 2 * sqz);

        y *= -1.0f;

        orientationYPR.yaw = juce::radiansToDegrees(y);
        orientationYPR.pitch = juce::radiansToDegrees(p);
        orientationYPR.roll = juce::radiansToDegrees(r);
        orientationYPR.angleType = M1OrientationYPR::AngleType::DEGREES;
    }
}

void Orientation::offsetQuat(M1OrientationQuat offset) {
    // TODO: implement this function
}

M1OrientationYPR Orientation::getYPR() {
    return orientationYPR;
}

M1OrientationYPR Orientation::getNormalised(M1OrientationYPR orientation) {
    M1OrientationYPR normalised_orientation;
    normalised_orientation.yaw = (float)juce::jmap(orientation.yaw, (float)orientation.yaw_min , (float)orientation.yaw_max, (float)0.0, (float)1.0);
    normalised_orientation.pitch = (float)juce::jmap(orientation.pitch, (float)orientation.pitch_min , (float)orientation.pitch_max, (float)0.0, (float)1.0);
    normalised_orientation.roll = (float)juce::jmap(orientation.roll, (float)orientation.roll_min , (float)orientation.roll_max, (float)0.0, (float)1.0);
    return normalised_orientation;
}

M1OrientationQuat Orientation::getNormalised(M1OrientationQuat orientation) {
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
    
    quat.wb = quat.wIn;
    quat.xb = quat.xIn;
    quat.yb = quat.yIn;
    quat.zb = quat.zIn;
    
    setQuat(quat);
}
