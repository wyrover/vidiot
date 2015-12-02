// Copyright 2013-2015 Eric Raijmakers.
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

#include "DetailsClip.h"

#include "ClipPreview.h"
#include "CommandProcessor.h"
#include "EditClipDetails.h"
#include "EditClipSpeed.h"
#include "ProjectModification.h"
#include "VideoComposition.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

// Helper methods for factor slider values. These ensure that 1/1 is in the middle.
// 100 <-> 9999: 0.01 <-> 0.99 (divide by 10000)
// 10000 : 1
// 10001 <-> 19900 : 1.01 <-> 100.00

// static
int DetailsClip::factorToSliderValue(rational64 speed)
{
    if (speed == 1) { return 10000; }
    if (speed < 1) { return boost::rational_cast<int>(speed * 10000); }
    return boost::rational_cast<int>(speed * 100 + 9900);
}

// static
rational64 DetailsClip::sliderValueToFactor(int slidervalue)
{
    if (slidervalue < 10000) { return rational64(slidervalue, 10000); }
    if (slidervalue == 10000) { return 1; }
    int diff = slidervalue - 10000;
    return rational64(diff + 100, 100); // +100 ensures 10001 starts at 1.01. -(diff/100) ensures that 20000 is exactly 100.00 again.
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

model::VideoClipPtr DetailsClip::getVideoClip(const model::IClipPtr& clip) const
{
    return
        !clip ? nullptr :
        clip->isA<model::VideoClip>() ? boost::dynamic_pointer_cast<model::VideoClip>(clip) :
        clip->isA<model::AudioClip>() ? boost::dynamic_pointer_cast<model::VideoClip>(clip->getLink()) :
        nullptr;
}

model::AudioClipPtr DetailsClip::getAudioClip(const model::IClipPtr& clip) const
{
    return
        !clip ? nullptr :
        clip->isA<model::AudioClip>() ? boost::dynamic_pointer_cast<model::AudioClip>(clip) :
        clip->isA<model::VideoClip>() ? boost::dynamic_pointer_cast<model::AudioClip>(clip->getLink()) :
        nullptr;
}

model::TransitionPtr DetailsClip::getTransition(const model::IClipPtr& clip) const
{
    return
        !clip ? nullptr :
        clip->isA<model::Transition>() ? boost::dynamic_pointer_cast<model::Transition>(clip) :
        nullptr;
}

pts DetailsClip::getVideoKeyFrameOffset() const
{
    model::VideoClipPtr videoclip{ getVideoClip(mClip) };
    ASSERT_NONZERO(videoclip);

    pts result{ 0 }; // Default: use first key frame
    if (getCursor().getLogicalPosition() >= videoclip->getLeftPts() &&  // By default, edit the frame under the cursor
        getCursor().getLogicalPosition() < videoclip->getRightPts())    // (which is already currently shown, typically)
    {
        // The cursor is positioned inside the clip being adjusted.
        result = getCursor().getLogicalPosition() - videoclip->getLeftPts();
    }
    ASSERT_MORE_THAN_EQUALS_ZERO(result);
    return result;
}

model::VideoClipKeyFramePtr DetailsClip::getVideoKeyFrame() const
{
    model::VideoClipPtr videoclip{ getVideoClip(mClip) };
    if (!videoclip) { return nullptr; }
    
    model::VideoClipKeyFramePtr result{ videoclip->getKeyFrameAt(getVideoKeyFrameOffset()) };
    ASSERT_NONZERO(result);

    return result;
}

void DetailsClip::submitEditCommandUponAudioVideoEdit(const wxString& message)
{
    getPlayer()->stop(); // Stop iteration through the sequence, since the sequence is going to be changed.

    if (mEditCommand == nullptr || // No command submit yet
        mEditCommand != model::CommandProcessor::get().GetCurrentCommand() || // If another command was done inbetween
        mEditCommand->getMessage() != message) // Another aspect was changed
    {
        // Use new clones for the new command
        mClones = std::unique_ptr<ClonesContainer>(new ClonesContainer(this, mClip));

        // Create the command - which replaces the original clip(s) with their changed clones - and add it to the undo system.
        mEditCommand = new command::EditClipDetails(getSequence(), message, mClip, mClip->getLink(), mClones->Clip, mClones->Link);

        // The submission of the command will result in a newly selected clip: the clone
        // Storing that clone as the current clip serves two purposes:
        // - The first 'selectClip' after the submission is ignored (since it's the clone).
        // - Undo will actually cause mClip to be selected again.
        mClip = mClones->Clip;

        mEditCommand->submit();

        // Do not reset mEditCommand: required for checking subsequent edits.
        // If a clip aspect is edited twice, simply adjust the clone twice,
        // but the command may only be submitted once.

        // Do not try invalidating the rectangle: mClones->Video addition event has not been received
        // by the track view class. So, no clip view/clip preview is known yet.
    }
    else
    {
        // Update the thumbnail for video (otherwise is not updated, since only one edit command is done).
        // Note that this code is executed BEFORE resetting mClones below. The reason for this is that when the new clones
        // are created, their views are not immediately initialized. However, the old clones have the same rect.
        if (mClones &&
            mClones->Video &&
            mClones->Video->getTrack())
        {
            ClipPreview* preview{ getViewMap().getClipPreview(mClip) };
            preview->invalidateCachedBitmaps();
            preview->invalidateRect();
            getTimeline().repaint(preview->getRect());
        }
    }

    preview();
}

void DetailsClip::submitEditCommandUponTransitionEdit(const wxString& parameter)
{
    getPlayer()->stop(); // Stop iteration through the sequence, since the sequence is going to be changed.

    ASSERT_NONZERO(mTransitionClone);

    if (mEditCommand == nullptr)
    {
        // Create the command - which replaces the original clip(s) with their changed clones - and add it to the undo system.
        mEditCommand = new command::EditClipDetails(getSequence(), _("Change "), mClip, nullptr, mTransitionClone, nullptr);

        // The submission of the command will result in a newly selected clip: the clone
        // Storing that clone as the current clip serves two purposes:
        // - The first 'selectClip' after the submission is ignored (since it's the clone).
        // - Undo will actually cause mClip to be selected again.
        mClip = mTransitionClone;

        mEditCommand->submit();

        // Do not reset mEditCommand: required for checking subsequent edits.
        // If a clip aspect is edited twice, simply adjust the clone twice,
        // but the command may only be submitted once.
    }
}

void DetailsClip::createOrUpdateSpeedCommand(rational64 speed)
{
    getPlayer()->stop(); // Stop iteration through the sequence, since the sequence is going to be changed.

    getTimeline().beginTransaction(); // Avoid flickering updates due to undo-ing the command.
    if (mEditSpeedCommand != nullptr &&
        mEditSpeedCommand == model::CommandProcessor::get().GetCurrentCommand())
    {
        // Avoid any subsequent setClip to reset the current clip (and reinitialize the controls and members)
        mClip = mEditSpeedCommand->getClip();
        model::CommandProcessor::get().Undo();
    }

    mClones = std::unique_ptr<ClonesContainer>(new ClonesContainer(this, mClip));

    // Avoid any subsequent setClip to reset the current clip (and reinitialize the controls and members)
    model::IClipPtr clip{ mClip };
    mClip = mClones->Clip;

    mEditSpeedCommand = new command::EditClipSpeed(getSequence(), clip, clip->getLink(), mClones->Clip, mClones->Link, speed);
    ASSERT_NONZERO(mEditSpeedCommand);

    if (model::ProjectModification::submitIfPossible(mEditSpeedCommand))
    {
        mSpeedSpin->SetValue(boost::rational_cast<double>(mEditSpeedCommand->getActualSpeed()));
        mSpeedSlider->SetValue(factorToSliderValue(mEditSpeedCommand->getActualSpeed()));
    }
    else
    {
        model::ClipIntervalPtr clipInterval{ boost::dynamic_pointer_cast<model::ClipInterval>(clip) };
        ASSERT_NONZERO(clipInterval);
        mSpeedSpin->SetValue(boost::rational_cast<double>(clipInterval->getSpeed()));
        mSpeedSlider->SetValue(factorToSliderValue(clipInterval->getSpeed()));
        mEditSpeedCommand = nullptr;
    }

    getTimeline().endTransaction();
    getTimeline().Update();
}

void DetailsClip::preview()
{
    if (!mClones || !mClones->Video) { return; }
    ASSERT_NONZERO(mClones->Video->getTrack()); // The edit command must have been submitted

    pts position = getCursor().getLogicalPosition(); // By default, show the frame under the cursor (which is already currently shown, typically)
    if ((position < mClones->Video->getLeftPts()) ||
        (position >= mClones->Video->getRightPts()))
    {
        // The cursor is not positioned under the clip being adjusted. Move the cursor to the middle frame of that clip
        position = mClones->Video->getLeftPts() + mClones->Video->getLength() / 2; // Show the middle frame of the clip
        VAR_DEBUG(position);
        getCursor().setLogicalPosition(position); // ...and move the cursor to that position
    }

    wxSize s = getPlayer()->getVideoSize();
    if (mClones->Video->getLength() > 0 &&
        s.GetWidth() > 0 &&
        s.GetHeight() > 0)
    {
        boost::shared_ptr<wxBitmap> bmp = boost::make_shared<wxBitmap>(s);
        wxMemoryDC dc(*bmp);

        // Fill with black
        dc.SetBrush(wxBrush{ wxColour{ 0, 0, 0 } });
        dc.SetPen(wxPen{ wxColour{ 0, 0, 0 } });
        dc.DrawRectangle(wxPoint(0,0),dc.GetSize());

        // Draw preview of operation
        getSequence()->moveTo(position);
        ASSERT_EQUALS(dc.GetSize(),s);
        model::VideoCompositionPtr composition = getSequence()->getVideoComposition(model::VideoCompositionParameters().setBoundingBox(dc.GetSize()).setPts(position));
        model::VideoFramePtr compositeFrame = composition->generate();
        if (compositeFrame)
        {
            wxBitmapPtr bitmap = compositeFrame->getBitmap();
            if (bitmap)
            {
                // Don't use DrawBitmap since this gives wrong output when using wxGTK.
                wxMemoryDC dcBmp(*bitmap);
                dc.Blit(wxPoint(0,0), bitmap->GetSize(), &dcBmp, wxPoint(0,0));
            }
        }

        dc.SelectObject(wxNullBitmap);
        getPlayer()->showPreview(bmp);
    }
}

void DetailsClip::updateAlignment(bool horizontalchange)
{
    auto getAlignment = [this, horizontalchange]() -> model::VideoAlignment
    {
        switch (mSelectAlignment->getValue())
        {
        case model::VideoAlignmentCenter:           return (horizontalchange ? model::VideoAlignmentCenterVertical      : model::VideoAlignmentCenterHorizontal);
        case model::VideoAlignmentCenterHorizontal: return (horizontalchange ? model::VideoAlignmentCustom              : model::VideoAlignmentCenterHorizontal);
        case model::VideoAlignmentCenterVertical:   return (horizontalchange ? model::VideoAlignmentCenterVertical      : model::VideoAlignmentCustom);
        }
        return mSelectAlignment->getValue();
    };
    mClones->Video->getKeyFrameAt(getVideoKeyFrameOffset())->setAlignment(getAlignment()); // todo just edit the cloned keyframe mClones->VideoKeyFrame
}

void DetailsClip::determineClipSizeBounds()
{
    ASSERT(mClip);

    model::IClipPtr link = mClip->getLink();
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(mClip);

    command::TrimClip::TrimLimit limitsBeginTrim;
    command::TrimClip::TrimLimit limitsEndTrim;
    if (transition)
    {
        limitsBeginTrim = command::TrimClip::determineBoundaries(getSequence(), transition, model::IClipPtr(), TransitionBegin, false);
        limitsEndTrim = command::TrimClip::determineBoundaries(getSequence(), transition, model::IClipPtr(), TransitionEnd, false);
    }
    else
    {
        limitsBeginTrim = command::TrimClip::determineBoundaries(getSequence(), mClip, link, ClipBegin, true);
        limitsEndTrim = command::TrimClip::determineBoundaries(getSequence(), mClip, link, ClipEnd, true);
    }

    pts clipPerceivedLength = mClip->getPerceivedLength();

    mMinimumLengthWhenBeginTrimming = clipPerceivedLength + -1 * limitsBeginTrim.Max;
    mMaximumLengthWhenBeginTrimming = clipPerceivedLength  + -1 * limitsBeginTrim.Min;
    mMinimumLengthWhenEndTrimming   = clipPerceivedLength  + limitsEndTrim.Min;
    mMaximumLengthWhenEndTrimming   = clipPerceivedLength  + limitsEndTrim.Max;
    mMinimumLengthWhenBothTrimming  = clipPerceivedLength  + -1 * limitsBeginTrim.Max + limitsEndTrim.Min;
    mMaximumLengthWhenBothTrimming  = clipPerceivedLength  + -1 * limitsBeginTrim.Min + limitsEndTrim.Max;

    // The 'both trimming' values are not 100% correct (the determined boundaries don't take
    // 'trimming on both sides simultaneously' into acount, only separate single trimming.
    // This can cause the smallest buttons to be enabled sometimes, although trimming to that
    // size is not possible (particularly applies for clips that have transitions on both edges).
    // To fix these cases, the minimum required clip size is taken as a lower bound also.
    //
    // the '-' here results in the 'area required for adjacent transitions'.
    mMinimumLengthWhenBothTrimming = std::max(mMinimumLengthWhenBothTrimming, mClip->getPerceivedLength() - mClip->getLength());
    if (link)
    {
        pts linkPerceivedLength = link->getPerceivedLength();
        mMinimumLengthWhenBothTrimming = std::max(mMinimumLengthWhenBothTrimming, linkPerceivedLength - link->getLength());
        if (linkPerceivedLength != clipPerceivedLength)
        {
            // In case the two linked clips have different lengths, disable all buttons,
            // except optionallly the button for the current length.
            mMinimumLengthWhenBeginTrimming = clipPerceivedLength;
            mMaximumLengthWhenBeginTrimming = clipPerceivedLength;
            mMinimumLengthWhenEndTrimming   = clipPerceivedLength;
            mMaximumLengthWhenEndTrimming   = clipPerceivedLength;
            mMinimumLengthWhenBothTrimming  = clipPerceivedLength;
            mMaximumLengthWhenBothTrimming  = clipPerceivedLength;
        }
    }

    ASSERT_MORE_THAN_EQUALS(mMaximumLengthWhenBothTrimming, mMaximumLengthWhenEndTrimming);
    ASSERT_MORE_THAN_EQUALS(mMaximumLengthWhenBothTrimming, mMaximumLengthWhenBeginTrimming);
    ASSERT_LESS_THAN_EQUALS(mMinimumLengthWhenBothTrimming, mMinimumLengthWhenEndTrimming);
    ASSERT_LESS_THAN_EQUALS(mMinimumLengthWhenBothTrimming, mMinimumLengthWhenBeginTrimming);
    VAR_DEBUG(mMinimumLengthWhenBeginTrimming)(mMaximumLengthWhenBeginTrimming)(mMinimumLengthWhenEndTrimming)(mMaximumLengthWhenEndTrimming)(mMinimumLengthWhenBothTrimming)(mMaximumLengthWhenBothTrimming);

    // For each possible length, store if it should be achieved by trimming at the beginning or at the end (the default)
    mTrimAtEnd.clear();
    for ( wxToggleButton* button : mLengthButtons )
    {
        pts length = getLength(button);
        mTrimAtEnd[length] = 0; // Default: no trim
        mTrimAtBegin[length] = 0; // Default: no trim
        if (length != clipPerceivedLength)
        {
            if (length >= mMinimumLengthWhenEndTrimming && length <= mMaximumLengthWhenEndTrimming)
            {
                // Trim at end only - default
                mTrimAtEnd[length] = length - clipPerceivedLength;
            }
            else if (length >= mMinimumLengthWhenBeginTrimming && length <= mMaximumLengthWhenBeginTrimming)
            {
                // Trim at begin only
                mTrimAtBegin[length] = clipPerceivedLength - length;
            }
            else if (length >= mMinimumLengthWhenBothTrimming && length <= mMaximumLengthWhenBothTrimming)
            {
                if (length < clipPerceivedLength)
                {
                    // Size reduction
                    mTrimAtEnd[length] = limitsEndTrim.Min;
                    mTrimAtBegin[length] = (clipPerceivedLength - length) + limitsEndTrim.Min; // Reduce with the size used for the end trim. Note: limitsEndTrim.Min <= 0! (hence, the '+' before limitsEndTrim)
                }
                else
                {
                    // Size enlargement
                    mTrimAtEnd[length] = limitsEndTrim.Max;
                    mTrimAtBegin[length] = (clipPerceivedLength - length) + limitsEndTrim.Max; // Reduce with the size used for the end trim. Note: trim at begin <= 0! (hence, the '+' before limitsEndTrim)
                }
            }
        }
    }
}

void DetailsClip::updateLengthButtons()
{
    if (!mClip)
    {
        return;
    }
    determineClipSizeBounds();
    ASSERT(!mClip->isA<model::EmptyClip>());
    for ( wxToggleButton* button : mLengthButtons )
    {
        button->SetValue(false);
        button->Disable();
    }

    pts minimumClipLength = mMinimumLengthWhenBothTrimming;
    pts maximumClipLength = mMaximumLengthWhenBothTrimming;
    pts currentLength = mClip->getPerceivedLength();
    ASSERT_MORE_THAN_EQUALS(currentLength, minimumClipLength);
    ASSERT_LESS_THAN_EQUALS(currentLength, maximumClipLength);

    for ( wxToggleButton* button : mLengthButtons )
    {
        pts length = getLength(button);
        button->SetValue(mClip && currentLength == length);
        button->Disable();

        ASSERT_MAP_CONTAINS(mTrimAtBegin,length);
        ASSERT_MAP_CONTAINS(mTrimAtEnd,length);

        if (mTrimAtEnd[length] != 0 || mTrimAtBegin[length] != 0)
        {
            button->Enable();
        }
    }
}

void DetailsClip::updateVideoKeyFrameControls()
{
    model::VideoClipPtr videoclip{ getVideoClip(mClip) };
    ASSERT_NONZERO(videoclip);
    model::VideoClipKeyFramePtr videoKeyFrame{ getVideoKeyFrame() };
    ASSERT_NONZERO(videoKeyFrame);
    pts videoKeyFrameOffset{ getVideoKeyFrameOffset() };

    rational64 factor{ videoKeyFrame->getScalingFactor() };
    rational64 rotation{ videoKeyFrame->getRotation() };
    wxPoint position{ videoKeyFrame->getPosition() };
    wxPoint maxpos{ videoKeyFrame->getMaxPosition() };
    wxPoint minpos{ videoKeyFrame->getMinPosition() };
    int opacity{ videoKeyFrame->getOpacity() };

    mOpacitySlider->SetValue(opacity);
    mOpacitySpin->SetValue(opacity);

    mSelectScaling->select(videoKeyFrame->getScaling());
    mScalingSlider->SetValue(factorToSliderValue(factor));
    mScalingSpin->SetValue(boost::rational_cast<double>(factor));

    mRotationSlider->SetValue(boost::rational_cast<int>(rotation * sRotationPrecisionFactor));
    mRotationSpin->SetValue(boost::rational_cast<double>(rotation));

    mSelectAlignment->select(videoKeyFrame->getAlignment());
    mPositionXSlider->SetRange(minpos.x, maxpos.x);
    mPositionXSlider->SetValue(position.x);
    mPositionXSpin->SetRange(minpos.x, maxpos.x);
    mPositionXSpin->SetValue(position.x);
    mPositionYSlider->SetRange(minpos.y, maxpos.y);
    mPositionYSlider->SetValue(position.y);
    mPositionYSpin->SetRange(minpos.y, maxpos.y);
    mPositionYSpin->SetValue(position.y);

    mOpacitySlider->Enable(!videoKeyFrame->isInterpolated());
    mOpacitySpin->Enable(!videoKeyFrame->isInterpolated());
    mSelectScaling->Enable(!videoKeyFrame->isInterpolated());
    mScalingSlider->Enable(!videoKeyFrame->isInterpolated());
    mScalingSpin->Enable(!videoKeyFrame->isInterpolated());
    mRotationSlider->Enable(!videoKeyFrame->isInterpolated());
    mRotationSpin->Enable(!videoKeyFrame->isInterpolated());
    mSelectAlignment->Enable(!videoKeyFrame->isInterpolated());
    mPositionXSlider->Enable(!videoKeyFrame->isInterpolated());
    mPositionXSpin->Enable(!videoKeyFrame->isInterpolated());
    mPositionYSlider->Enable(!videoKeyFrame->isInterpolated());
    mPositionYSpin->Enable(!videoKeyFrame->isInterpolated());

    // todo If a keyframe button is pressed, move the timeline cursor accordingly.
    // todo for the preview do not pick the center frame but the first frame? (or simply: the current keyframe)

    size_t nKeyFrames{ videoclip->getNumberOfKeyFrames() };
    if (mVideoKeyFramesPanel->GetChildren().size() != nKeyFrames) // todo handle too much keyframes to show....
    {
        mVideoKeyFramesPanel->DestroyChildren();
        mVideoKeyFrames.clear();

        wxBoxSizer* sizer{ new wxBoxSizer{ wxHORIZONTAL } };
        for (size_t i{ 0 }; i < nKeyFrames; ++i)
        {
            mVideoKeyFrames[i] = new wxToggleButton{ mVideoKeyFramesPanel, wxID_ANY, wxString::Format("%d", i + 1), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT };
            mVideoKeyFrames[i]->SetFont(mVideoKeyFrames[i]->GetFont().Smaller().Smaller());
            sizer->Add(mVideoKeyFrames[i], wxSizerFlags{ 1 });
        }
        mVideoKeyFramesPanel->SetSizerAndFit(sizer);
        mVideoKeyFramesPanel->Layout();
        //mVideoKeyFramesEditPanel->Layout();
        //Layout();
    }
    for (size_t i{ 0 }; i < nKeyFrames; ++i)
    {
        mVideoKeyFrames[i]->SetValue(i == videoKeyFrame->getIndex() && nKeyFrames > 1);
        mVideoKeyFrames[i]->Enable(i != videoKeyFrame->getIndex());
    }
    mVideoKeyFramesHomeButton->Enable(videoKeyFrame->getIndex() > 0);
    mVideoKeyFramesPrevButton->Enable(videoKeyFrame->getIndex() > 0);
    mVideoKeyFramesNextButton->Enable(videoKeyFrame->getIndex() < nKeyFrames - 1);
    mVideoKeyFramesEndButton->Enable(videoKeyFrame->getIndex() < nKeyFrames - 1);
    mVideoKeyFramesRemoveButton->Enable(videoKeyFrame->getIndex() > 0);
    mVideoKeyFramesAddButton->Enable(videoKeyFrame->isInterpolated() || (nKeyFrames == 1 && videoKeyFrameOffset > 0));
}

}} // namespace
