// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#ifndef PRECOMPILED_TEST_H
#define PRECOMPILED_TEST_H

#include "Precompiled.h"

#include <typeinfo>

#include <cxxtest/GlobalFixture.h>
#include <cxxtest/TestSuite.h>
#include <cxxtest/TestTracker.h>

#include <wx/filefn.h>
#include <wx/uiaction.h>

// All Vidiot includes
#include "Application.h"
#include "AudioChunk.h"
#include "AudioClip.h"
#include "AudioCompositionParameters.h"
#include "AudioTrack.h"
#include "AutoFolder.h"
#include "ClipCreator.h"
#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "CreateTransition.h"
#include "EmptyClip.h"
#include "File.h"
#include "Folder.h"
#include "IClip.h"
#include "ids.h"
#include "IPath.h"
#include "Layout.h"
#include "Node.h"
#include "NodeEvent.h"
#include "Project.h"
#include "Properties.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "UtilClone.h"
#include "UtilFrameRate.h"
#include "UtilInt.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "UtilThread.h"
#include "VideoClip.h"
#include "VideoCompositionParameters.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "VideoTransitionFactory.h"
#include "Window.h"
#include "Zoom.h"

// All test includes (avoid having to include them in all tests.cpp files)
#include "FixtureConfig.h"
#include "FixtureGui.h"
#include "FixtureProject.h"
#include "HelperApplication.h"
#include "HelperConfig.h"
#include "HelperDetails.h"
#include "HelperFileSystem.h"
#include "HelperModel.h"
#include "HelperPlayback.h"
#include "HelperPopupMenu.h"
#include "HelperProject.h"
#include "HelperProjectView.h"
#include "HelperSequence.h"
#include "HelperTestSuite.h"
#include "HelperThread.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperTimelineTrim.h"
#include "HelperWindow.h" // Required by HelperTransform.h
#include "HelperTransform.h"
#include "HelperTransition.h"
#include "HelperWatcher.h"
#include "HelperWorker.h"
#include "SuiteCreator.h"

#endif
