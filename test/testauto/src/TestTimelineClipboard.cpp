// Copyright 2014-2015 Eric Raijmakers.
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

#include "TestTimelineClipboard.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestTimelineClipboard::setUp()
{
    mProjectFixture.init();
}

void TestTimelineClipboard::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestTimelineClipboard::testCutAndPaste()
{
    StartTestSuite();
	FillClipboardWithFiles(getListOfInputPathsAsFileNames());

	TimelineLeftClick(Center(VideoClip(0, 4)));
	ASSERT_SELECTION_SIZE(1);
	ASSERT(VideoClip(0,4)->getSelected());
	ASSERT(AudioClip(0,4)->getSelected());
	WindowTriggerMenu(wxID_CUT);
	ASSERT_CLIPBOARD_CONTAINS_CLIPS;
	ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
	ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip);
	TimelinePositionCursor(RightPixel(VideoTrack(0)));
	WindowTriggerMenu(wxID_PASTE);
	ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
	ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
	ASSERT_EQUALS(VideoClip(0,7)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
	ASSERT_EQUALS(AudioClip(0,7)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,4));
}

void TestTimelineClipboard::testCopyAndPaste()
{
    StartTestSuite();
	TimelineLeftClick(Center(VideoClip(0, 4)));
	ASSERT_SELECTION_SIZE(1);
	ASSERT(VideoClip(0,4)->getSelected());
	ASSERT(AudioClip(0,4)->getSelected());
	WindowTriggerMenu(wxID_COPY);
	ASSERT_CLIPBOARD_CONTAINS_CLIPS;
	ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
	ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
	TimelinePositionCursor(RightPixel(VideoTrack(0)));
	WindowTriggerMenu(wxID_PASTE);
	ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
	ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
	ASSERT_EQUALS(VideoClip(0,7)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
	ASSERT_EQUALS(AudioClip(0,7)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,4));
}

void TestTimelineClipboard::testPasteFromProjectView()
{
	StartTestSuite();
	{
		ClearClipboard();
		StartTest("Preparation: Nodes in clipboard.");
		gui::ProjectView::get().expand(mProjectFixture.mAutoFolder);
		util::thread::RunInMainAndWait([this]
		{
			gui::ProjectView::get().select(mProjectFixture.mAutoFolder->getChildren());
		});
		ProjectViewSetFocus();
		WindowTriggerMenu(wxID_COPY);
		ASSERT_CLIPBOARD_CONTAINS_NODES;
	}
	{
		StartTest("Paste in timeline");
		TimelinePositionCursor(RightPixel(VideoTrack(0))); // Also gives focus to timeline
		WindowTriggerMenu(wxID_PASTE);
		ASSERT_VIDEOTRACK0SIZE(mProjectFixture.InputFiles.size() * 2);
		ASSERT_AUDIOTRACK0SIZE(mProjectFixture.InputFiles.size() * 2);
	}
}

void TestTimelineClipboard::testPasteFromProjectView_IllegalNodes()
{
    StartTestSuite();
	{
		ClearClipboard();
		StartTest("Preparation: Nodes in clipboard.");
		util::thread::RunInMainAndWait([this]
		{
			gui::ProjectView::get().select(mProjectFixture.mRoot->getChildren());
		});
		ProjectViewSetFocus();
		WindowTriggerMenu(wxID_COPY);
		ASSERT_CLIPBOARD_CONTAINS_NODES;
	}
	{
		StartTest("Paste in timeline");
		TimelinePositionCursor(RightPixel(VideoTrack(0))); // Also gives focus to timeline
		WindowTriggerMenu(wxID_PASTE);
		ASSERT_VIDEOTRACK0SIZE(mProjectFixture.InputFiles.size());
		ASSERT_AUDIOTRACK0SIZE(mProjectFixture.InputFiles.size());
	}
}

void TestTimelineClipboard::testPasteFromFileSystem()
{
	StartTestSuite();
	{
		StartTest("Preparation: Files in clipboard.");
		FillClipboardWithFiles(getListOfInputPathsAsFileNames());
	}
	{
		StartTest("Paste in timeline");
		TimelinePositionCursor(RightPixel(VideoTrack(0))); // Also gives focus to timeline
		WindowTriggerMenu(wxID_PASTE);
		ASSERT_VIDEOTRACK0SIZE(mProjectFixture.InputFiles.size() * 2);
		ASSERT_AUDIOTRACK0SIZE(mProjectFixture.InputFiles.size() * 2);
	}
}

void TestTimelineClipboard::testPasteFromFileSystem_IllegalNodes()
{
	StartTestSuite();
	{
		StartTest("Preparation: Files and one folder in clipboard.");
		wxFileNames l(getListOfInputPathsAsFileNames());
		l.push_back(getTestFilesPath().GetFullPath());
		FillClipboardWithFiles(l);
	}
	{
		StartTest("Paste in timeline");
		TimelinePositionCursor(RightPixel(VideoTrack(0))); // Also gives focus to timeline
		WindowTriggerMenu(wxID_PASTE);
		ASSERT_VIDEOTRACK0SIZE(mProjectFixture.InputFiles.size());
		ASSERT_AUDIOTRACK0SIZE(mProjectFixture.InputFiles.size());
	}
}



} // namespace
