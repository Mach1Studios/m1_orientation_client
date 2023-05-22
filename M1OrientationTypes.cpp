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
    orientationYPR = orientation;
    
    if (orientation.angleType == M1OrientationYPR::AngleType::DEGREES) {
        // passthrough for conversion to Quat
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
    }

    // Below converts YPR DEGREES -> YPR RADIANS -> Quat
    // It is better to avoid this function and stick to updating quat and calculating best YPR
    orientationQuat.w = cos(juce::degreesToRadians(orientationYPR.roll) / 2) * cos(juce::degreesToRadians(orientationYPR.pitch) / 2) * cos(juce::degreesToRadians(orientationYPR.yaw) / 2) + sin(juce::degreesToRadians(orientationYPR.roll) / 2) * sin(juce::degreesToRadians(orientationYPR.pitch) / 2) * sin(juce::degreesToRadians(orientationYPR.yaw) / 2);
    orientationQuat.x = sin(juce::degreesToRadians(orientationYPR.roll) / 2) * cos(juce::degreesToRadians(orientationYPR.pitch) / 2) * cos(juce::degreesToRadians(orientationYPR.yaw) / 2) - cos(juce::degreesToRadians(orientationYPR.roll) / 2) * sin(juce::degreesToRadians(orientationYPR.pitch) / 2) * sin(juce::degreesToRadians(orientationYPR.yaw) / 2);
    orientationQuat.y = cos(juce::degreesToRadians(orientationYPR.roll) / 2) * sin(juce::degreesToRadians(orientationYPR.pitch) / 2) * cos(juce::degreesToRadians(orientationYPR.yaw) / 2) + sin(juce::degreesToRadians(orientationYPR.roll) / 2) * cos(juce::degreesToRadians(orientationYPR.pitch) / 2) * sin(juce::degreesToRadians(orientationYPR.yaw) / 2);
    orientationQuat.z = cos(juce::degreesToRadians(orientationYPR.roll) / 2) * cos(juce::degreesToRadians(orientationYPR.pitch) / 2) * sin(juce::degreesToRadians(orientationYPR.yaw) / 2) - sin(juce::degreesToRadians(orientationYPR.roll) / 2) * sin(juce::degreesToRadians(orientationYPR.pitch) / 2) * cos(juce::degreesToRadians(orientationYPR.yaw) / 2);
}

void Orientation::setQuat(M1OrientationQuat orientation) {
    orientationQuat = orientation;

    //normalize
    double magnitude = sqrt(orientationQuat.wIn * orientationQuat.wIn + orientationQuat.xIn * orientationQuat.xIn + orientationQuat.yIn * orientationQuat.yIn + orientationQuat.zIn * orientationQuat.zIn);
    orientationQuat.wIn /= magnitude;
    orientationQuat.xIn /= magnitude;
    orientationQuat.yIn /= magnitude;
    orientationQuat.zIn /= magnitude;

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
        orientationYPR.custom_output_yaw = (float)juce::jmap(orientationYPR.yaw, (float)-180, (float)180, orientationYPR.yaw_min, orientationYPR.yaw_max);
        orientationYPR.custom_output_pitch = (float)juce::jmap(orientationYPR.pitch, (float)-180, (float)180, orientationYPR.pitch_min, orientationYPR.pitch_max);
        orientationYPR.custom_output_roll = (float)juce::jmap(orientationYPR.roll, (float)-180, (float)180, orientationYPR.roll_min, orientationYPR.roll_max);
        return;
    } else if (test < -0.499999) {
        // singularity at south pole
        orientationYPR.yaw = -2 * atan2(orientationQuat.x, orientationQuat.w);
        orientationYPR.pitch = -juce::MathConstants<double>::pi / 2;
        orientationYPR.roll = 0;
        orientationYPR.custom_output_yaw = (float)juce::jmap(orientationYPR.yaw, (float)-180, (float)180, orientationYPR.yaw_min, orientationYPR.yaw_max);
        orientationYPR.custom_output_pitch = (float)juce::jmap(orientationYPR.pitch, (float)-180, (float)180, orientationYPR.pitch_min, orientationYPR.pitch_max);
        orientationYPR.custom_output_roll = (float)juce::jmap(orientationYPR.roll, (float)-180, (float)180, orientationYPR.roll_min, orientationYPR.roll_max);
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

        // remap output orientationYPR
        orientationYPR.custom_output_yaw = (float)juce::jmap(orientationYPR.yaw, (float)-180, (float)180, orientationYPR.yaw_min, orientationYPR.yaw_max);
        orientationYPR.custom_output_pitch = (float)juce::jmap(orientationYPR.pitch, (float)-180, (float)180, orientationYPR.pitch_min, orientationYPR.pitch_max);
        orientationYPR.custom_output_roll = (float)juce::jmap(orientationYPR.roll, (float)-180, (float)180, orientationYPR.roll_min, orientationYPR.roll_max);
    }
}

M1OrientationYPR Orientation::getYPR() {
    return orientationYPR;
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
