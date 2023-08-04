#pragma once

#include <string>

class M1OrientationManagerOSCSettings
{
public:
    virtual bool init(int serverPort, int watcherPort, bool useWatcher) = 0;
    bool initFromSettings(std::string jsonSettingsFilePath, bool useWatcher);
};
