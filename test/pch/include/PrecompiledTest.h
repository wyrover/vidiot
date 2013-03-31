#ifndef PRECOMPILED_TEST_H
#define PRECOMPILED_TEST_H

#include "Application.h"
#include "AudioClip.h"
#include "AudioTrack.h"
#include "AudioView.h"
#include "AutoFolder.h"
#include "ClipView.h"
#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "CreateTransition.h"
#include "Cursor.h"
#include "DeleteSelectedClips.h"
#include "Details.h"
#include "DetailsClip.h"
#include "DetailsTrim.h"
#include "Dialog.h"
#include "Drag.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "File.h"
#include "FixtureConfig.h"
#include "FixtureGui.h"
#include "FixtureProject.h"
#include "HelperApplication.h"
#include "HelperConfig.h"
#include "HelperDetails.h"
#include "HelperModel.h"
#include "HelperProjectView.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimeLinesView.h"
#include "HelperTransform.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "ids.h"
#include "Layout.h"
#include "Logging.h"
#include "Menu.h"
#include "PositionInfo.h"
#include "Project.h"
#include "ProjectView.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateSequence.h"
#include "ProjectViewDropSource.h"
#include "Properties.h"
#include "Render.h"
#include "RenderSettingsDialog.h"
#include "Scrolling.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "SuiteCreator.h"
#include "Timeline.h"
#include "TimeLinesView.h"
#include "Track.h"
#include "Transition.h"
#include "Trim.h"
#include "TrimClip.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoCodecs.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "VideoView.h"
#include "ViewMap.h"
#include "Window.h"
#include "Worker.h"
#include "Zoom.h"
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <cxxtest/TestSuite.h>
#include <cxxtest/TestTracker.h>
#include <list>
#include <ostream>
#include <time.h>
#include <wx/ffile.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/mousestate.h>
#include <wx/msgdlg.h>
#include <wx/uiaction.h>
#include <wx/utils.h>

#endif