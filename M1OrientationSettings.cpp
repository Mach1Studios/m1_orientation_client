#include <JuceHeader.h>
#include "M1OrientationSettings.h"

bool M1OrientationManagerOSCSettings::initFromSettings(std::string jsonSettingsFilePath) {
    juce::File settingsFile = juce::File(jsonSettingsFilePath);
    if (!settingsFile.exists()) {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::NoIcon,
            "Warning",
            "Settings file doesn't exist",
            "",
            nullptr,
            juce::ModalCallbackFunction::create(([&](int result) {
                juce::JUCEApplicationBase::quit();
                }))
        );
        return false;
    }
    else {
        juce::var mainVar = juce::JSON::parse(juce::File(jsonSettingsFilePath));
		int serverPort = mainVar["serverPort"];
		int watcherPort = mainVar["watcherPort"];

        if (!init(serverPort, watcherPort)) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Warning",
                "Conflict is happening and you need to choose a new port",
                "",
                nullptr,
                juce::ModalCallbackFunction::create(([&](int result) {
                    juce::JUCEApplicationBase::quit();
                    }))
            );
            return false;
        }
    }
    return true;
}
