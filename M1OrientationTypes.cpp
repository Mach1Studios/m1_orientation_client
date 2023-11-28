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

void Orientation::setYPR_type(M1OrientationYPR::AngleType type) {
    orientationYPR.angleType = type;
    shiftYPR.angleType = type;
}

void Orientation::setYPR_range(float min_yaw, float min_pitch, float min_roll, float max_yaw, float max_pitch, float max_roll) {
    orientationYPR.yaw_min = min_yaw; orientationYPR.yaw_max = max_yaw;
    orientationYPR.pitch_min = min_pitch; orientationYPR.pitch_max = max_pitch;
    orientationYPR.roll_min = min_roll; orientationYPR.roll_max = max_roll;
    shiftYPR.yaw_min = min_yaw; shiftYPR.yaw_max = max_yaw;
    shiftYPR.pitch_min = min_pitch; shiftYPR.pitch_max = max_pitch;
    shiftYPR.roll_min = min_roll; shiftYPR.roll_max = max_roll;
}

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
    if ((int)shiftYPR.angleType != (int)M1OrientationYPR::SIGNED_NORMALLED) {
        shiftYPR = getSignedNormalled(shiftYPR);
    }
    setYPR_type(M1OrientationYPR::SIGNED_NORMALLED);
    setYPR_range(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f);

    // Below converts YPR SIGNED NORMALLED -> YPR RADIANS -> Quat
    // It is better to avoid this function and stick to updating quat and calculating best YPR
    // Note:
    //   Skipping the setQuat() call and calculating the output directly
    
    // TODO: deal with left/right handedness before conversion
    orientationQuat.w = cos((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2) + sin((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2);
    orientationQuat.x = sin((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2) - cos((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2);
    orientationQuat.y = cos((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2) + sin((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2);
    orientationQuat.z = cos((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2) - sin((juce::MathConstants<float>::pi * orientationYPR.roll) / 2) * sin((juce::MathConstants<float>::pi * orientationYPR.pitch) / 2) * cos((juce::MathConstants<float>::pi * orientationYPR.yaw) / 2);
}

void Orientation::offsetYPR(M1OrientationYPR offset_delta) {
    // this function assumes the input is an orientation delta and applies it safely
    // Warning: shift all offsets to signed so that 0 is center of rotations
    
    if ((int)shiftYPR.angleType != (int)offset_delta.angleType) {
        if ((int)shiftYPR.angleType == (int)M1OrientationYPR::SIGNED_NORMALLED) {
            offset_delta = getSignedNormalled(offset_delta); // normalize
            shiftYPR = shiftYPR + offset_delta;
        } else if ((int)shiftYPR.angleType == (int)M1OrientationYPR::UNSIGNED_NORMALLED) {
            offset_delta = getUnsignedNormalled(offset_delta); // normalize
            shiftYPR = shiftYPR + offset_delta;
        } else if ((int)shiftYPR.angleType == (int)M1OrientationYPR::DEGREES) {
            offset_delta = getSignedNormalled(offset_delta); // normalize
            offset_delta.yaw *= 180.0f;
            offset_delta.pitch *= 180.0f;
            offset_delta.roll *= 180.0f;
            shiftYPR = shiftYPR + offset_delta;
        } else if ((int)shiftYPR.angleType == (int)M1OrientationYPR::RADIANS) {
            offset_delta = getSignedNormalled(offset_delta); // normalize
            offset_delta.yaw *= juce::MathConstants<float>::pi;
            offset_delta.pitch *= juce::MathConstants<float>::pi;
            offset_delta.roll *= juce::MathConstants<float>::pi;
            shiftYPR = shiftYPR + offset_delta;
        } else {
            DBG("Error: Unexpected angle type");
        }
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
    
    double qysqr = orientationQuat.x * orientationQuat.z - orientationQuat.y * orientationQuat.w;
    double t0 = -2.0 * (qysqr + orientationQuat.z * orientationQuat.z) + 1.0;
    double t1 = +2.0 * (orientationQuat.x * orientationQuat.y + orientationQuat.w * orientationQuat.z);
    double t2 = -2.0 * (orientationQuat.x * orientationQuat.z - orientationQuat.w * orientationQuat.y);
    double t3 = +2.0 * (orientationQuat.y * orientationQuat.z + orientationQuat.w * orientationQuat.x);
    double t4 = -2.0 * (orientationQuat.x * orientationQuat.x + qysqr) + 1.0;
    
    t2 = t2 > +1.0 ? +1.0 : t2; // singularity at north pole
    t2 = t2 < -1.0 ? -1.0 : t2; // singularity at south pole
    
    float y = (float)atan2(t1, t0);
    y *= -1.0f; // flip for lefthanded yaw+ is to the right
    float p = (float)asin(t2);
    p *= -1.0;
    float r = (float)atan2(t3, t4);

    // signed normalled -1 -> 1
    orientationYPR.yaw = y;
    orientationYPR.pitch = p;
    orientationYPR.roll = r;
    if ((int)shiftYPR.angleType != (int)M1OrientationYPR::SIGNED_NORMALLED) {
        shiftYPR = getSignedNormalled(shiftYPR);
    }
    setYPR_type(M1OrientationYPR::SIGNED_NORMALLED);
    setYPR_range(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f);
}

void Orientation::offsetQuat(M1OrientationQuat offset_delta) {
    orientationQuat.wb = offset_delta.w;
    orientationQuat.xb = offset_delta.x;
    orientationQuat.yb = offset_delta.y;
    orientationQuat.zb = offset_delta.z;
    setQuat(orientationQuat);
}

M1OrientationYPR Orientation::getYPRasUnsignedNormalled() {
    M1OrientationYPR returnOrientation;
    returnOrientation.angleType = M1OrientationYPR::UNSIGNED_NORMALLED;
    returnOrientation.yaw_min = 0.0f, returnOrientation.pitch_min = 0.0f, returnOrientation.roll_min = 0.0f;
    returnOrientation.yaw_max = 1.0f, returnOrientation.pitch_max = 1.0f, returnOrientation.roll_max = 1.0f;

    if (orientationYPR.angleType == M1OrientationYPR::UNSIGNED_NORMALLED) {
        // TODO: test this
        returnOrientation = orientationYPR + shiftYPR;
    } else {
        // shift yaw negative range to make 0deg = 0norm when yaw is not 0
        returnOrientation.yaw = std::fmod(  (float)juce::jmap(orientationYPR.yaw + std::abs(orientationYPR.yaw_min), orientationYPR.yaw_min, orientationYPR.yaw_max, 0.0f, 1.0f)
                                          + (float)juce::jmap(shiftYPR.yaw + std::abs(shiftYPR.yaw_min), shiftYPR.yaw_min, shiftYPR.yaw_max, 0.0f, 1.0f)
                                          , 1.0f);
        returnOrientation.pitch = std::fmod(  (float)juce::jmap(orientationYPR.pitch, orientationYPR.pitch_min, orientationYPR.pitch_max, 0.0f, 1.0f)
                                            + (float)juce::jmap(shiftYPR.pitch, shiftYPR.pitch_min, shiftYPR.pitch_max, 0.0f, 1.0f)
                                            , 1.0f);
        returnOrientation.roll = std::fmod(  (float)juce::jmap(orientationYPR.roll, orientationYPR.roll_min, orientationYPR.roll_max, 0.0f, 1.0f)
                                           + (float)juce::jmap(shiftYPR.roll, shiftYPR.roll_min, shiftYPR.roll_max, 0.0f, 1.0f)
                                           , 1.0f);
    }
    return returnOrientation;
}

M1OrientationYPR Orientation::getYPRasSignedNormalled() {
    M1OrientationYPR returnOrientation;
    returnOrientation.angleType = M1OrientationYPR::SIGNED_NORMALLED;
    returnOrientation.yaw_min = -1.0f, returnOrientation.pitch_min = -1.0f, returnOrientation.roll_min = -1.0f;
    returnOrientation.yaw_max = 1.0f, returnOrientation.pitch_max = 1.0f, returnOrientation.roll_max = 1.0f;
    
    if (orientationYPR.angleType == M1OrientationYPR::SIGNED_NORMALLED) {
        // TODO: test this
        returnOrientation = orientationYPR + shiftYPR;
    } else {
        returnOrientation.yaw = std::fmod(  (float)juce::jmap(orientationYPR.yaw + std::abs(orientationYPR.yaw_min), orientationYPR.yaw_min, orientationYPR.yaw_max, 0.0f, 2.0f)
                                          + (float)juce::jmap(shiftYPR.yaw + std::abs(shiftYPR.yaw_min), shiftYPR.yaw_min, shiftYPR.yaw_max, 0.0f, 2.0f)
                                          , 2.0f);
        returnOrientation.yaw -= 2.0f; // shift back to -1 -> 1
        returnOrientation.pitch = std::fmod(  (float)juce::jmap(orientationYPR.pitch, orientationYPR.pitch_min, orientationYPR.pitch_max, 0.0f, 2.0f)
                                            + (float)juce::jmap(shiftYPR.pitch, shiftYPR.pitch_min, shiftYPR.pitch_max, 0.0f, 2.0f)
                                            , 2.0f);
        returnOrientation.pitch -= 2.0f; // shift back to -1 -> 1
        returnOrientation.roll = std::fmod(  (float)juce::jmap(orientationYPR.roll, orientationYPR.roll_min, orientationYPR.roll_max, 0.0f, 2.0f)
                                           + (float)juce::jmap(shiftYPR.roll, shiftYPR.roll_min, shiftYPR.roll_max, 0.0f, 2.0f)
                                           , 2.0f);
        returnOrientation.roll -= 2.0f; // shift back to -1 -> 1
    }
    return returnOrientation;
}

M1OrientationYPR Orientation::getYPRasDegrees() {
    M1OrientationYPR returnOrientation;
    returnOrientation.angleType = M1OrientationYPR::DEGREES;
    returnOrientation.yaw_min = -180.0f, returnOrientation.pitch_min = -180.0f, returnOrientation.roll_min = -180.0f;
    returnOrientation.yaw_max = 180.0f, returnOrientation.pitch_max = 180.0f, returnOrientation.roll_max = 180.0f;
    
    if ((int)orientationYPR.angleType == (int)M1OrientationYPR::DEGREES) {
        // TODO: test this
        returnOrientation = orientationYPR + shiftYPR;
    } else {
        returnOrientation.yaw = std::fmod(  (float)juce::jmap(orientationYPR.yaw + std::abs(orientationYPR.yaw_min), orientationYPR.yaw_min, orientationYPR.yaw_max, -180.0f, 180.0f)
                                          + (float)juce::jmap(shiftYPR.yaw + std::abs(shiftYPR.yaw_min), shiftYPR.yaw_min, shiftYPR.yaw_max, -180.0f, 180.0f)
                                          , 360.0f);
        returnOrientation.pitch = std::fmod(  (float)juce::jmap(orientationYPR.pitch, orientationYPR.pitch_min, orientationYPR.pitch_max, -180.0f, 180.0f)
                                            + (float)juce::jmap(shiftYPR.pitch, shiftYPR.pitch_min, shiftYPR.pitch_max, -180.0f, 180.0f)
                                            , 360.0f);
        returnOrientation.roll = std::fmod(  (float)juce::jmap(orientationYPR.roll, orientationYPR.roll_min, orientationYPR.roll_max, -180.0f, 180.0f)
                                           + (float)juce::jmap(shiftYPR.roll, shiftYPR.roll_min, shiftYPR.roll_max, -180.0f, 180.0f)
                                           , 360.0f);
    }
    return returnOrientation;
}

M1OrientationYPR Orientation::getYPRasRadians() {
    M1OrientationYPR returnOrientation;
    returnOrientation.angleType = M1OrientationYPR::RADIANS;
    returnOrientation.yaw_min = -juce::MathConstants<float>::pi, returnOrientation.pitch_min = -juce::MathConstants<float>::pi, returnOrientation.roll_min = -juce::MathConstants<float>::pi;
    returnOrientation.yaw_max = juce::MathConstants<float>::pi, returnOrientation.pitch_max = juce::MathConstants<float>::pi, returnOrientation.roll_max = juce::MathConstants<float>::pi;

    if (orientationYPR.angleType == M1OrientationYPR::RADIANS) {
        // TODO: test this
        returnOrientation = orientationYPR + shiftYPR;
    } else {
        returnOrientation.yaw = std::fmod(  (float)juce::jmap(orientationYPR.yaw + std::abs(orientationYPR.yaw_min), orientationYPR.yaw_min, orientationYPR.yaw_max, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi)
                                          + (float)juce::jmap(shiftYPR.yaw + std::abs(shiftYPR.yaw_min), shiftYPR.yaw_min, shiftYPR.yaw_max, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi)
                                          , juce::MathConstants<float>::twoPi);
        returnOrientation.pitch = std::fmod(  (float)juce::jmap(orientationYPR.pitch, orientationYPR.pitch_min, orientationYPR.pitch_max, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi)
                                            + (float)juce::jmap(shiftYPR.pitch, shiftYPR.pitch_min, shiftYPR.pitch_max, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi)
                                            , juce::MathConstants<float>::twoPi);
        returnOrientation.roll = std::fmod(  (float)juce::jmap(orientationYPR.roll, orientationYPR.roll_min, orientationYPR.roll_max, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi)
                                           + (float)juce::jmap(shiftYPR.roll, shiftYPR.roll_min, shiftYPR.roll_max, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi)
                                           , juce::MathConstants<float>::twoPi);
    }
    return returnOrientation;
}

M1OrientationQuat Orientation::getQuat() {
    return orientationQuat;
}

void Orientation::resetOrientation() {
    shiftYPR.yaw = 0.0f;
    shiftYPR.pitch = 0.0f;
    shiftYPR.roll = 0.0f;
    orientationQuat.wb = 1.0f;
    orientationQuat.xb = 0.0f;
    orientationQuat.yb = 0.0f;
    orientationQuat.zb = 0.0f;
}

void Orientation::recenter() {
    M1OrientationYPR swpAngle;
    if ((int)shiftYPR.angleType == (int)M1OrientationYPR::SIGNED_NORMALLED) {
        M1OrientationYPR swpAngle = getYPRasSignedNormalled();
    } else if ((int)shiftYPR.angleType == (int)M1OrientationYPR::UNSIGNED_NORMALLED) {
        M1OrientationYPR swpAngle = getYPRasUnsignedNormalled();
    } else if ((int)shiftYPR.angleType == (int)M1OrientationYPR::DEGREES) {
        M1OrientationYPR swpAngle = getYPRasDegrees();
    } else if ((int)shiftYPR.angleType == (int)M1OrientationYPR::RADIANS) {
        M1OrientationYPR swpAngle = getYPRasRadians();
    } else {
        DBG("Error: Unexpected angle type");
    }

    shiftYPR = shiftYPR - swpAngle;
}

M1OrientationYPR getSignedNormalled(M1OrientationYPR orientation) {
    M1OrientationYPR normalised_orientation;
    normalised_orientation.yaw = (float)juce::jmap(orientation.yaw, orientation.yaw_min , orientation.yaw_max, -1.0f, 1.0f);
    normalised_orientation.pitch = (float)juce::jmap(orientation.pitch, orientation.pitch_min , orientation.pitch_max, -1.0f, 1.0f);
    normalised_orientation.roll = (float)juce::jmap(orientation.roll, orientation.roll_min , orientation.roll_max, -1.0f, 1.0f);
    normalised_orientation.angleType = M1OrientationYPR::SIGNED_NORMALLED;
    normalised_orientation.yaw_min = -1.0f, normalised_orientation.pitch_min = -1.0f, normalised_orientation.roll_min = -1.0f;
    normalised_orientation.yaw_max = 1.0f, normalised_orientation.pitch_max = 1.0f, normalised_orientation.roll_max = 1.0f;
    return normalised_orientation;
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
