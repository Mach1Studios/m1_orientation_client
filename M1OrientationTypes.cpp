#include <JuceHeader.h>
#include "M1OrientationTypes.h"

std::map<enum M1OrientationDeviceType, std::string> M1OrientationDeviceTypeName = {
    { M1OrientationManagerDeviceTypeNone, "Unknown"},
    { M1OrientationManagerDeviceTypeSerial, "Serial"},
    { M1OrientationManagerDeviceTypeBLE, "BLE"},
    { M1OrientationManagerDeviceTypeOSC, "OSC"},
    { M1OrientationManagerDeviceTypeCamera, "Camera"},
};

std::map<enum M1OrientationStatusType, std::string> M1OrientationStatusTypeName = {
    { M1OrientationManagerStatusTypeError, "Error"},
    { M1OrientationManagerStatusTypeNotConnectable, "Not Connectable"},
    { M1OrientationManagerStatusTypeConnectable, "Connectable"},
    { M1OrientationManagerStatusTypeConnected, "Connected"},
};

void Orientation::setYPR(M1OrientationYPR orientation) {
    orientationYPR = orientation;

    /// It is better to avoid this function and stick to updating quat and calculating best YPR
    orientationQuat.x = sin(orientationYPR.roll / 2) * cos(orientationYPR.pitch / 2) * cos(orientationYPR.yaw / 2) - cos(orientationYPR.roll / 2) * sin(orientationYPR.pitch / 2) * sin(orientationYPR.yaw / 2);
    orientationQuat.y = cos(orientationYPR.roll / 2) * sin(orientationYPR.pitch / 2) * cos(orientationYPR.yaw / 2) + sin(orientationYPR.roll / 2) * cos(orientationYPR.pitch / 2) * sin(orientationYPR.yaw / 2);
    orientationQuat.z = cos(orientationYPR.roll / 2) * cos(orientationYPR.pitch / 2) * sin(orientationYPR.yaw / 2) - sin(orientationYPR.roll / 2) * sin(orientationYPR.pitch / 2) * cos(orientationYPR.yaw / 2);
    orientationQuat.w = cos(orientationYPR.roll / 2) * cos(orientationYPR.pitch / 2) * cos(orientationYPR.yaw / 2) + sin(orientationYPR.roll / 2) * sin(orientationYPR.pitch / 2) * sin(orientationYPR.yaw / 2);
}

void Orientation::setQuat(M1OrientationQuat orientation) {
    orientationQuat = orientation;

    //normalize
    double magnitude = sqrt(orientationQuat.w * orientationQuat.w + orientationQuat.x * orientationQuat.x + orientationQuat.y * orientationQuat.y + orientationQuat.z * orientationQuat.z);
    orientationQuat.w /= magnitude;
    orientationQuat.x /= magnitude;
    orientationQuat.y /= magnitude;
    orientationQuat.z /= magnitude;

    orientationQuat.w = orientation.lastw * orientation.w + orientation.lastx * orientation.x + orientation.lasty * orientation.y + orientation.lastz * orientation.z;
    orientationQuat.x = orientation.lastw * orientation.x - orientation.lastx * orientation.w - orientation.lasty * orientation.z + orientation.lastz * orientation.y;
    orientationQuat.y = orientation.lastw * orientation.y + orientation.lastx * orientation.z - orientation.lasty * orientation.w - orientation.lastz * orientation.x;
    orientationQuat.z = orientation.lastw * orientation.z - orientation.lastx * orientation.y + orientation.lasty * orientation.x - orientation.lastz * orientation.w;
    
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
    
    quat.lastw = quat.w;
    quat.lastx = quat.x;
    quat.lasty = quat.y;
    quat.lastz = quat.z;
    
    setQuat(quat);
}
