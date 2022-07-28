#include <JuceHeader.h>
#include "M1OrientationTypes.h"

std::map<enum M1OrientationDeviceType, std::string> M1OrientationDeviceTypeName = {
    { M1OrientationManagerDeviceTypeNone, ""},
    { M1OrientationManagerDeviceTypeBLE, "BLE"},
    { M1OrientationManagerDeviceTypeSerial, "Serial"},
    { M1OrientationManagerDeviceTypeOSC, "OSC"},
    { M1OrientationManagerDeviceTypeCamera, "Camera"},
};

std::map<enum M1OrientationStatusType, std::string> M1OrientationStatusTypeName = {
    { M1OrientationManagerStatusTypeNone, ""},
    { M1OrientationManagerStatusTypeConnecting, "Connecting"},
    { M1OrientationManagerStatusTypeConnected, "Connected"},
    { M1OrientationManagerStatusTypeError, "Error"},
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

    // quat.qw = quat.lastqw * orientationQuat.w + quat.lastqx * orientationQuat.x + quat.lastqy * orientationQuat.y + quat.lastqz * orientationQuat.z;
    // quat.qx = quat.lastqw * orientationQuat.x - quat.lastqx * orientationQuat.w - quat.lastqy * orientationQuat.z + quat.lastqz * orientationQuat.y;
    // quat.qy = quat.lastqw * orientationQuat.y + quat.lastqx * orientationQuat.z - quat.lastqy * orientationQuat.w - quat.lastqz * orientationQuat.x;
    // quat.qz = quat.lastqw * orientationQuat.z - quat.lastqx * orientationQuat.y + quat.lastqy * orientationQuat.x - quat.lastqz * orientationQuat.w;
    
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
    // quat.qw = quat.lastqw;
    // quat.qx = quat.lastqx;
    // quat.qy = quat.lastqy;
    // quat.qz = quat.lastqz;
}
