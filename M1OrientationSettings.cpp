#include <JuceHeader.h>
#include "M1OrientationSettings.h"

bool M1OrientationManagerOSCSettings::initFromSettings(std::string jsonSettingsFilePath) {
    juce::File settingsFile = juce::File(jsonSettingsFilePath);
    if (!settingsFile.exists()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::NoIcon,
            "Warning",
            "Mach1: settings.json file doesn't exist in Mach1's Application Support directory, please reinstall the Spatial System",
            "",
            nullptr,
            juce::ModalCallbackFunction::create(([&](int result) {
               //juce::JUCEApplicationBase::quit();
            }))
        );
        return false;
    }
    else {
        // Found the settings.json
        juce::var mainVar = juce::JSON::parse(juce::File(jsonSettingsFilePath));
        int serverPort = mainVar["serverPort"];
        int helperPort = mainVar["helperPort"];

        if (!init(serverPort, helperPort)) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Warning",
                "Mach1: Conflict is happening and you need to choose a new port by changing settings.json in Mach1's Application Support directory",
                "",
                nullptr,
                juce::ModalCallbackFunction::create(([&](int result) {
                    //juce::JUCEApplicationBase::quit();
                }))
            );
            return false;
        }
    }
    return true;
}
