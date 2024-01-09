#pragma once

#include <string>

class M1OrientationManagerOSCSettings
{
public:
    virtual bool init(int serverPort, int helperPort) = 0;
    bool initFromSettings(std::string jsonSettingsFilePath);
};
