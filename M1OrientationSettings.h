#pragma once

#include <string>

class M1OrientationManagerOSCSettings
{
public:
    virtual bool init(int serverPort, int watcherPort) = 0;
   
    bool initFromSettings(std::string jsonSettingsFilePath);
};
