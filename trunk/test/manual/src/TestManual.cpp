#include "TestManual.h"

#include "Dialog.h"
#include "DetailsClip.h"
#include "FixtureGui.h"
#include "HelperApplication.h"
#include "HelperDetails.h"
#include "HelperModel.h"
#include "HelperProjectView.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "ProjectView.h"
#include "ids.h"
#include "Layout.h"
#include "File.h"
#include "Logging.h"
#include "Scrolling.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "UtilLog.h"
#include "VideoTrack.h"
#include "UtilLogWxwidgets.h"
#include <boost/foreach.hpp>
#include <wx/uiaction.h>

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TestManual::TestManual()
    :   mProjectFixture(false) // Set to true if the timeline must be focused initially
{
}

void TestManual::setUp()
{
    mProjectFixture.init();
}

void TestManual::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestManual::testManual()
{
    StartTestSuite();
    StartTest("Manual...");
    LOG_ERROR << dump(getSequence());

    // todo make help text
    // todo make audio composition
    // todo scale clip (reduce size), then put this clip in track2, then fade in this clip above another clip -> the black is also 'faded in', whereas it should have been 'empty'!
    // todo bug: trim to the edge of empty clip uses non inited preview data)
    // todo at end of clip edit, remove any empty clips which are the last clips of the track
    pause(60000 * 60); // One hour should be enough...

    // todo crash
// Did several add transitions (both video and audio) and some undo/redoing. THen crash when generating sequence.....
//    05-07-2013 13:35:26.865 ERROR   t@bd30 Y:/Software/Vidiot/vidiot_trunk/util/src/Config.cpp(64) Config::init [ConfigFile=C:\Users\epra\AppData\Roaming\Vidiot.ini]
//05-07-2013 13:36:14.418 WARNING t@8188 Y:/Software/Vidiot/vidiot_trunk/modelaudio/src/AudioClip.cpp(114) model::AudioClip::getNextAudio   2|Audio|00FCE634|00F84D14|00FFB3C4|   489|0|     0|   250|  881712: (0102 12.25.34 - Copy.avi) Adding 288 samples to make audio length equal to video length
//05-07-2013 13:36:14.702 WARNING t@8188 Y:/Software/Vidiot/vidiot_trunk/modelaudio/src/AudioClip.cpp(114) model::AudioClip::getNextAudio   3|Audio|00FCE73C|00F84D14|09580D5C|   739|0|     0|   250|  881712: (0102 12.25.34.avi) Adding 288 samples to make audio length equal to video length
//05-07-2013 13:36:15.253 WARNING t@c2e4 Y:/Software/Vidiot/vidiot_trunk/gui/src/VideoDisplay.cpp(378) gui::VideoDisplay::videoDisplayThread [paTime=595325775][mStartTime=595324770][mCurrentTime=1005][sleepTime=-5][nextFrameTime=1000][nextFrameTimeAdaptedForPlaybackSpeed=1000][mStartPts=341][videoFrame->getPts()=366]
//05-07-2013 13:36:28.188 WARNING t@c07c Y:/Software/Vidiot/vidiot_trunk/modelaudio/src/AudioClip.cpp(114) model::AudioClip::getNextAudio   2|Audio|00FCE634|00F84D14|00FFB3C4|   489|0|     0|   250|  880574: (0102 12.25.34 - Copy.avi) Adding 1426 samples to make audio length equal to video length
//05-07-2013 13:36:28.237 WARNING t@c07c Y:/Software/Vidiot/vidiot_trunk/modelaudio/src/AudioClip.cpp(114) model::AudioClip::getNextAudio   3|Audio|00FCE73C|00F84D14|09580D5C|   739|0|     0|   250|  881712: (0102 12.25.34.avi) Adding 288 samples to make audio length equal to video length
//05-07-2013 13:36:29.773 WARNING t@c6fc Y:/Software/Vidiot/vidiot_trunk/gui/src/VideoDisplay.cpp(378) gui::VideoDisplay::videoDisplayThread [paTime=595340295][mStartTime=595339070][mCurrentTime=1225][sleepTime=-5][nextFrameTime=2440][nextFrameTimeAdaptedForPlaybackSpeed=1220][mStartPts=551][videoFrame->getPts()=612]
//.
//.
//05-07-2013 13:36:33.288 WARNING t@c6fc Y:/Software/Vidiot/vidiot_trunk/gui/src/VideoDisplay.cpp(378) gui::VideoDisplay::videoDisplayThread [paTime=595343811][mStartTime=595339070][mCurrentTime=4741][sleepTime=-1][nextFrameTime=9480][nextFrameTimeAdaptedForPlaybackSpeed=4740][mStartPts=551][videoFrame->getPts()=788]
//05-07-2013 13:37:01.740 WARNING t@f258 Y:/Software/Vidiot/vidiot_trunk/modelaudio/src/AudioClip.cpp(114) model::AudioClip::getNextAudio   1|Audio|010192A8|00F84D14|01018320|   114|0|   111|   139|  489840: (0102 12.25.34 - Copy.avi) Adding 552 samples to make audio length equal to video length
//05-07-2013 13:37:20.986 WARNING t@a4c0 Y:/Software/Vidiot/vidiot_trunk/modelaudio/src/AudioClip.cpp(114) model::AudioClip::getNextAudio   2|Audio|12D78448|00F84D14|09685C90|   240|0|   237|    13|   44902: (0102 12.25.34 - Copy.avi) Adding 962 samples to make audio length equal to video length
//05-07-2013 13:37:21.737 WARNING t@c564 Y:/Software/Vidiot/vidiot_trunk/gui/src/VideoDisplay.cpp(378) gui::VideoDisplay::videoDisplayThread [paTime=595392262][mStartTime=595391820][mCurrentTime=442][sleepTime=-2][nextFrameTime=880][nextFrameTimeAdaptedForPlaybackSpeed=440][mStartPts=201][videoFrame->getPts()=223]
//05-07-2013 13:37:24.126 WARNING t@c35c Y:/Software/Vidiot/vidiot_trunk/gui/src/VideoDisplay.cpp(378) gui::VideoDisplay::videoDisplayThread [paTime=595394651][mStartTime=595394570][mCurrentTime=81][sleepTime=-1][nextFrameTime=160][nextFrameTimeAdaptedForPlaybackSpeed=80][mStartPts=286][videoFrame->getPts()=290]
//05-07-2013 13:37:40.673 ERROR   t@bd30 Y:/Software/Vidiot/vidiot_trunk/timeline/src/DetailsClip.cpp(635) gui::timeline::DetailsClip::preview [position=100]
//05-07-2013 13:37:47.854 ERROR   t@bd30 Y:/Software/Vidiot/vidiot_trunk/modelempty/src/EmptyClip.cpp(192) model::EmptyClip::getNextVideo [getLength()=586][videoFrame=00F55200|100|1|(531,247)|(     0,     0)|255|{(0,0,531,247)}|0]
//.
//.
//05-07-2013 13:39:58.018 ERROR   t@a634 Y:/Software/Vidiot/vidiot_trunk/modelempty/src/EmptyClip.cpp(192) model::EmptyClip::getNextVideo [getLength()=586][videoFrame=00F55200|240|1|(720,576)|(     0,     0)|255|{(0,0,720,576)}|0]
//05-07-2013 13:39:58.026 ASSERT  t@a634 Y:/Software/Vidiot/vidiot_trunk/modelvideo/src/VideoTransition.cpp(57) model::VideoTransition::getNextVideo [ASSERT:getPrev()]
//05-07-2013 13:40:12.916 STACK   t@a634
//  0 0131536D      445  y:\software\vidiot\vidiot_trunk\util\src\utillog.cpp(262): LogVar::~LogVar
//  1 0151F4A5      405  y:\software\vidiot\vidiot_trunk\modelvideo\src\videotransition.cpp(58): model::VideoTransition::getNextVideo
//  2 0151C23A      234  y:\software\vidiot\vidiot_trunk\modelvideo\src\videotrack.cpp(64): model::VideoTrack::getNextVideo
//  3 013D9CDA      218  y:\software\vidiot\vidiot_trunk\model\src\sequence.cpp(316): model::Sequence::getVideoComposition
//  4 013DF697       87  y:\software\vidiot\vidiot_trunk\model\src\sequence.cpp(143): model::Sequence::getNextVideo
//  5 014CC70B     6603  y:\software\vidiot\vidiot_trunk\render\src\render.cpp(494): model::render::Render::generate
//  6 014B50A3      115  y:\software\vidiot\boost_1_48_0\boost\bind\mem_fn_template.hpp(399): boost::_mfi::mf3<void,model::render::Render,boost::shared_ptr<model::Sequence>,int,int>::operator()<boost::shared_ptr<model::render::Render> >
//  7 014BE31E      110  y:\software\vidiot\boost_1_48_0\boost\function\function_template.hpp(154): boost::detail::function::void_function_obj_invoker0<boost::_bi::bind_t<void,boost::_mfi::mf3<void,model::render::Render,boost::shared_ptr<model::Sequence>,int,int>,boost::_bi::list4<boost::_bi::value<boost::shared_ptr<model::render::Render> >,boost::_bi::value<boost::shared_ptr<model::Sequence> >,boost::_bi::value<int>,boost::_bi::value<int> > >,void>::invoke
//  8 01302427      119  y:\software\vidiot\boost_1_48_0\boost\function\function_template.hpp(760): boost::function0<void>::operator()
//  9 0153CCE8      184  y:\software\vidiot\vidiot_trunk\worker\src\work.cpp(23): worker::Work::execute
// 10 0153DDE0      736  y:\software\vidiot\vidiot_trunk\worker\src\worker.cpp(84): worker::Worker::thread
// 11 0F494CE3      819  boost::detail::allocate_raw_heap_memory
// 12 784DC556       58  endthreadex
// 13 784DC600      228  endthreadex
// 14 753E33AA       18  BaseThreadInitThunk
// 15 77069EF2       99  RtlInitializeExceptionChain
// 16 77069EC5       54  RtlInitializeExceptionChain

}

} // namespace