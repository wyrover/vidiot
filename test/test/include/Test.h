// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

#include "Precompiled.h"

#include <typeinfo>

#include <cxxtest/GlobalFixture.h>
#include <cxxtest/TestSuite.h>
#include <cxxtest/TestTracker.h>

#include <wx/ffile.h>
#include <wx/filefn.h>
#include <wx/msgout.h>
#include <wx/uiaction.h>

#ifdef __GNUC__
#include <X11/Xlib.h>
#endif

// All Vidiot includes
#include "Application.h"
#include "AudioChunk.h"
#include "AudioClip.h"
#include "AudioCodecs.h"
#include "AudioCompositionParameters.h"
#include "AudioKeyFrame.h"
#include "AudioTrack.h"
#include "AudioView.h"
#include "AutoFolder.h"
#include "ClipCreator.h"
#include "ClipPreview.h"
#include "ClipView.h"
#include "Combiner.h"
#include "CommandLine.h"
#include "CommandProcessor.h"
#include "Config.h"
#include "Convert.h"
#include "CreateAudioTrack.h"
#include "CreateTransition.h"
#include "CreateTransitionHelper.h"
#include "CreateVideoTrack.h"
#include "Cursor.h"
#include "DeleteSelectedClips.h"
#include "Details.h"
#include "DetailsClip.h"
#include "DetailsTrim.h"
#include "Dialog.h"
#include "DialogRenderSettings.h"
#include "DividerView.h"
#include "Drag.h"
#include "EditClipDetails.h"
#include "EditClipSpeed.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "File.h"
#include "Folder.h"
#include "IClip.h"
#include "ids.h"
#include "IPath.h"
#include "Keyboard.h"
#include "Logging.h"
#include "Menu.h"
#include "Mouse.h"
#include "Node.h"
#include "NodeEvent.h"
#include "Player.h"
#include "PositionInfo.h"
#include "Project.h"
#include "ProjectEvent.h"
#include "ProjectModification.h"
#include "ProjectView.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateSequence.h"
#include "ProjectViewDataObject.h"
#include "Properties.h"
#include "Render.h"
#include "Scrolling.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "State.h"
#include "StatusBar.h"
#include "Timeline.h"
#include "TimelineDataObject.h"
#include "TimelinesView.h"
#include "TimescaleView.h"
#include "Track.h"
#include "TrackView.h"
#include "Transition.h"
#include "TransitionParameterInt.h"
#include "TrimClip.h"
#include "UnlinkClips.h"
#include "UtilFrameRate.h"
#include "UtilPath.h"
#include "UtilThread.h"
#include "UtilVector.h"
#include "VideoClip.h"
#include "VideoCodecs.h"
#include "VideoCompositionParameters.h"
#include "VideoDisplayEvent.h"
#include "VideoFrame.h"
#include "VideoKeyFrame.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "VideoTransitionFactory.h"
#include "VideoView.h"
#include "ViewMap.h"
#include "FileWatcher.h"
#include "Window.h"
#include "Worker.h"
#include "WximageClip.h"
#include "WximageFile.h"
#include "Zoom.h"

namespace test {

class RandomTempDir;
typedef boost::shared_ptr<RandomTempDir> RandomTempDirPtr;
typedef std::pair<RandomTempDirPtr, wxFileName> DirAndFile;

}

// All test includes (avoid having to include them in all tests.cpp files)
#include "FixtureConfig.h"
#include "FixtureGui.h"
#include "FixtureProject.h"
#include "HelperApplication.h"
#include "HelperAudioChunk.h"
#include "HelperClipboard.h"
#include "HelperConfig.h"
#include "HelperDetails.h"
#include "HelperFileSystem.h"
#include "HelperHistory.h"
#include "HelperInput.h"
#include "HelperModel.h"
#include "HelperPlayback.h"
#include "HelperPopupMenu.h"
#include "HelperProject.h"
#include "HelperProjectView.h"
#include "HelperRender.h"
#include "HelperSequence.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperTimelineTrim.h"
#include "HelperWidgets.h"
#include "HelperWindow.h" // Used by HelperTransform
#include "HelperTransform.h"
#include "HelperTransition.h"
#include "HelperVideoFrame.h"
#include "HelperWatcher.h"
#include "HelperWidgets.h"
#include "HelperWorker.h"
#include "HelperFiles.h" // Uses a lot of the other header files
#include "SuiteCreator.h"
