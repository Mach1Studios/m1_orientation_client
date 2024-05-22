/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 m1_orientation_client
  vendor:             Mach1
  version:            0.0.1
  name:               Mach1 Orientation Client
  description:        An orientation client that receives the aggregate peripheral orienatation or headtracking devices and inputs from the orientation-server.
  website:            https://github.com/Mach1Studios/m1_orientation_client
  license:            Free
  searchpaths:        libs/m1-mathematics/include

  dependencies:       juce_osc

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#pragma once

// TODO: fix this definition
// #if defined(WIN32) && !defined(WIN32_LEAN_AND_MEAN) 
// #error need to define WIN32_LEAN_AND_MEAN in project settings
// #endif

#include "M1OrientationTypes.h"
#include "M1OrientationSettings.h"
#include "M1OrientationClient.h"
