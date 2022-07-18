#pragma once

#include <JuceHeader.h>

class M1OrientationManagerOSCSettings
{
public:
    virtual bool init(int serverPort) = 0;
   
    bool initFromSettings(std::string jsonSettingsFilePath);
};
