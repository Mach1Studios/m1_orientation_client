/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 m1_orientation_manager
  vendor:             Mach1
  version:            0.0.1
  name:               Mach1 Orientation Manager
  description:        An orientation manager to aggregate peripheral orienatation or headtracking devices and inputs.
  website:            https://github.com/Mach1Studios/m1_orientation_manager
  license:            Free

  dependencies:       juce_osc

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#pragma once

#include "M1OrientationTypes.h"
#include "M1OrientationSettings.h"
#include "M1OrientationOSCClient.h"
