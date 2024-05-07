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
