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

#include "DetailsClip.h"

#include "AClipEdit.h"
#include "ClipPreview.h"
#include "CommandProcessor.h"
#include "EditClipDetails.h"
#include "EditClipSpeed.h"
#include "ProjectModification.h"
#include "VideoTransitionFactory.h"
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

void DetailsClip::requestShowAndUpdateTitle()
{
    ASSERT_NONZERO(mClip);
    // TRANSLATORS: This is used in the details header to indicate 'clip name and length in seconds': "Clip name (42.000s)"
    requestShow(true, wxString::Format(_("%s (%ss)"), mClip->getDescription(), model::Convert::ptsToHumanReadibleString(mClip->getPerceivedLength())));
}

void DetailsClip::submitEditCommandUponAudioVideoEdit(const wxString& message, bool video, std::function<void()> edit)
{
    pts offset{ video ? mVideoKeyFrameControls->getKeyFrameOffset() : mAudioKeyFrameControls->getKeyFrameOffset() };

    if (mEditCommand == nullptr || // No command submit yet
        mEditCommand != model::CommandProcessor::get().GetCurrentCommand() || // If another command was done inbetween
        mEditCommand->getMessage() != message ||  // Another aspect was changed
        message == sEditKeyFramesAdd || // A key frame addition is always a separate command
        message == sEditKeyFramesRemove ||  // A key frame removal is always a separate command
        mLastEditKeyFrameOffset != offset)  // If the same aspect is edited, but for a different key frame (position), use a separate command
    {
        // Use new clones for the new command
        model::IClipPtr originalClip{ mClip };
        std::pair<model::IClipPtr, model::IClipPtr> clones{ cmd::AClipEdit::clone(mClip) }; // Keep clones in scope to avoid the new clip's link being destructed
        mClip = clones.first;
        mLastEditKeyFrameOffset = offset;

        // Create the command - which replaces the original clip(s) with their changed clones - and add it to the undo system.
        mEditCommand = new cmd::EditClipDetails(getSequence(), message, originalClip, mClip);

        // Bind to the proper events before handling the command.
        // This ensures that the project events are received which, in turn,
        // results in updating the preview window.
        // Note that the proper key frame is requested from the clip in the model,
        // hence the command must be submit first.

        mEditCommand->submit();

        // Do not reset mEditCommand: required for checking subsequent edits.
        // If a clip aspect is edited twice, simply adjust the clone twice,
        // but the command may only be submitted once.

        // Do not try invalidating the timeline rectangle. Addition of clone to track event has not been received
        // by the track view class. So, no clip view/clip preview is known yet.
    }
    else
    {
        // Update the thumbnail/peaks (otherwise not updated, since only one edit command is done).
        auto refreshclip = [this](model::IClipPtr clip)
        {
            if (clip && clip->getTrack() != nullptr)
            {
                getViewMap().getClipPreview(clip)->redrawNow();
            }
        };
        getSequenceView().setRealtimeRedrawing(true);
        if (video)
        {
            refreshclip(mVideoKeyFrameControls->getClip());
        }
        else
        {
            refreshclip(mAudioKeyFrameControls->getClip());
        };
        getTimeline().Update();
        getSequenceView().setRealtimeRedrawing(false);
    }
    edit();
    if (video)
    {
        // For audio clips there is no preview only the updated clip preview (peaks).
        // Note that this 'if' also prevents moving the cursor in the following case:
        // - Move cursor to center of clip 'x + 1'
        // - Select clip 'x'
        // - Edit volume
        // Changing the cursor now looks weird.
        preview();
    }
    mVideoKeyFrameControls->update();
    mAudioKeyFrameControls->update();
}

void DetailsClip::submitEditCommandUponTransitionEdit(const wxString& parameter)
{
    ASSERT_NONZERO(mTransitionClone);

    // TRANSLATORS: This is shown in the undo history when a parameter of a transition has been edited. %1$s == Name of transition, %2$s == Name of parameter.
    wxString message{ wxString::Format(_("Change %1$s (%2$s)"), mClip->getDescription(), parameter) };

    if (mEditCommand == nullptr || // No command submit yet
        mEditCommand != model::CommandProcessor::get().GetCurrentCommand() || // If another command was done inbetween
        mEditCommand->getMessage() != message) // Another aspect was changed
    {
        // Create the command - which replaces the original clip(s) with their changed clones - and add it to the undo system.
        mEditCommand = new cmd::EditClipDetails(getSequence(), message, mClip, mTransitionClone);

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

    if (!mPlayButton->GetValue())
    {
        preview();
    }
}

void DetailsClip::submitEditCommandUponTransitionTypeChange(model::TransitionPtr transition)
{
    model::TransitionPtr original{ boost::dynamic_pointer_cast<model::Transition>(mClip) };
    ASSERT(original)(mClip);

    // Give the new transition the same parameters as the original transition, as much as possible.
    transition->init(original->getLeft(), original->getRight());
    transition->initParameters(original->getCurrentParameters()); // Parameters with same name are copied

    // Create the command - which replaces the original clip(s) with their changed clones - and add it to the undo system.
    mEditCommand = new cmd::EditClipDetails(getSequence(), _("Change transition type"), mClip, transition);

    destroyTransitionParameterWidgets();

    // The submission of the command will result in a newly selected clip: the clone
    // Storing that clone as the current clip serves two purposes:
    // - The first 'selectClip' after the submission is ignored (since it's the clone).
    // - Undo will actually cause mClip to be selected again.
    mClip = transition;

    makeTransitionCloneAndCreateTransitionParameterWidgets(mClip);

    mEditCommand->submit();

    requestShowAndUpdateTitle(); // Particularly for the update title

    // Do not reset mEditCommand: required for checking subsequent edits.
    // If a clip aspect is edited twice, simply adjust the clone twice,
    // but the command may only be submitted once.

    if (!mPlayButton->GetValue())
    {
        preview();
    }
}

void DetailsClip::createOrUpdateSpeedCommand(rational64 speed)
{
    getTimeline().beginTransaction(); // Avoid flickering updates due to undo-ing the command.
    if (mEditSpeedCommand != nullptr &&
        mEditSpeedCommand == model::CommandProcessor::get().GetCurrentCommand())
    {
        // Avoid any subsequent setClip to reset the current clip (and reinitialize the controls and members)
        mClip = mEditSpeedCommand->getClip();
        model::CommandProcessor::get().Undo();
    }

    model::IClipPtr originalClip{ mClip };
    std::pair<model::IClipPtr, model::IClipPtr> clones{ cmd::AClipEdit::clone(mClip) }; // Keep clones in scope to avoid the new clip's link being destructed
    mClip = clones.first;

    mEditSpeedCommand = new cmd::EditClipSpeed(getSequence(), originalClip, mClip, speed);
    ASSERT_NONZERO(mEditSpeedCommand);

    if (model::ProjectModification::submitIfPossible(mEditSpeedCommand))
    {
        mSpeedSpin->SetValue(boost::rational_cast<double>(mEditSpeedCommand->getActualSpeed()));
        mSpeedSlider->SetValue(factorToSliderValue(mEditSpeedCommand->getActualSpeed()));
        requestShowAndUpdateTitle(); // Update length in title bar of details view
    }
    else
    {
        mClip = originalClip;
        model::ClipIntervalPtr clipInterval{ boost::dynamic_pointer_cast<model::ClipInterval>(mClip) };
        ASSERT_NONZERO(clipInterval);
        mSpeedSpin->SetValue(boost::rational_cast<double>(clipInterval->getSpeed()));
        mSpeedSlider->SetValue(factorToSliderValue(clipInterval->getSpeed()));
        mEditSpeedCommand = nullptr;
    }

    getTimeline().endTransaction();
    getTimeline().Update();
    // NOT: preview(); -- leave cursor at same position
}

void DetailsClip::startStopPlayback(bool ignoreIndex)
{
    if (!mClip ||
        mClip->getTrack() == nullptr ||
        !mClip->isA<model::Transition>() ||
        !mClip->isA<model::IVideo>())
    {
        // Wrong clip type.
        mPlayButton->SetValue(false);
    }
    else
    {
        if (!ignoreIndex)
        {
            std::pair<int, int> index{ std::make_pair(mClip->getIndex(), mClip->getTrack()->getIndex()) };
            if (mPlayButton->GetValue() &&
                mPlaybackClipIndex != index)
            {
                // Check if playback active, and the clip with the same index is still selected.
                // Continue the playback in that case.
                // Example: Change transition type, then undo the change. After the undo, playback must remain active.
                mPlayButton->SetValue(false);
            }
        }
    }

    if (mPlayButton->GetValue())
    {
        mPlaybackClipIndex = std::make_pair(mClip->getIndex(), mClip->getTrack()->getIndex());
        getPlayer()->playRange(mClip->getPerceivedLeftPts(), mClip->getPerceivedRightPts());
    }
    else
    {
        mPlaybackClipIndex = std::make_pair(-1, -1);
        getPlayer()->stopRange();
    }
}

void DetailsClip::preview()
{
    if (mClip->isA<model::IVideo>())
    {
        model::VideoClipPtr videoclip{ getClipOfType<model::VideoClip>(mClip) };
        model::TransitionPtr transition{ boost::dynamic_pointer_cast<model::Transition>(mClip) };
        ASSERT_NONZERO(mClip->getTrack())(mClip);

        pts position{ getCursor().getLogicalPosition() }; // By default, show the frame under the cursor (which is already currently shown, typically)

        bool showCenterFrame{ position < mClip->getPerceivedLeftPts() || position > mClip->getPerceivedRightPts() }; // == getPerceivedRightPts() is the key frame AFTER the last frame of the clip.

        if (videoclip && videoclip->getKeyFramesOfPerceivedClip().size() > 0)
        {
            showCenterFrame = false;
        }

        if (showCenterFrame)
        {
            // In case of a clip without key frames, there is only the default key frame.
            // If the cursor is not yet 'inside' the clip, move it to the middle frame of that clip
            getCursor().setLogicalPosition(mClip->getPerceivedLeftPts() + (mClip->getPerceivedLength() / 2)); // ...and move the cursor to that position
        }
        else
        {
            getPlayer()->moveTo(getCursor().getLogicalPosition());
        }
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
        case model::VideoAlignmentTopLeft:          return (horizontalchange ? model::VideoAlignmentTop                 : model::VideoAlignmentLeft);
        case model::VideoAlignmentTop:              return (horizontalchange ? model::VideoAlignmentTop                 : model::VideoAlignmentCustom);
        case model::VideoAlignmentTopCenter:        return (horizontalchange ? model::VideoAlignmentTop                 : model::VideoAlignmentCenterHorizontal);
        case model::VideoAlignmentTopRight:         return (horizontalchange ? model::VideoAlignmentTop                 : model::VideoAlignmentRight);
        case model::VideoAlignmentRight:            return (horizontalchange ? model::VideoAlignmentCustom              : model::VideoAlignmentRight);
        case model::VideoAlignmentRightCenter:      return (horizontalchange ? model::VideoAlignmentCenterVertical      : model::VideoAlignmentRight);
        case model::VideoAlignmentBottomRight:      return (horizontalchange ? model::VideoAlignmentBottom              : model::VideoAlignmentRight);
        case model::VideoAlignmentBottom:           return (horizontalchange ? model::VideoAlignmentBottom              : model::VideoAlignmentCustom);
        case model::VideoAlignmentBottomCenter:     return (horizontalchange ? model::VideoAlignmentBottom              : model::VideoAlignmentCenterHorizontal);
        case model::VideoAlignmentBottomLeft:       return (horizontalchange ? model::VideoAlignmentBottom              : model::VideoAlignmentLeft);
        case model::VideoAlignmentLeft:             return (horizontalchange ? model::VideoAlignmentCustom              : model::VideoAlignmentLeft);
        case model::VideoAlignmentLeftCenter:       return (horizontalchange ? model::VideoAlignmentCenterVertical      : model::VideoAlignmentLeft);
        default:                                    return mSelectAlignment->getValue();
        }
    };
    mVideoKeyFrameControls->getKeyFrame()->setAlignment(getAlignment());
}

void DetailsClip::determineClipSizeBounds()
{
    ASSERT(mClip);

    model::IClipPtr link{ mClip->getLink() };
    model::TransitionPtr transition{ boost::dynamic_pointer_cast<model::Transition>(mClip) };

    cmd::TrimClip::TrimLimit limitsBeginTrim;
    cmd::TrimClip::TrimLimit limitsEndTrim;
    if (transition)
    {
        limitsBeginTrim = cmd::TrimClip::determineBoundaries(getSequence(), transition, model::IClipPtr(), TransitionBegin, false);
        limitsEndTrim = cmd::TrimClip::determineBoundaries(getSequence(), transition, model::IClipPtr(), TransitionEnd, false);
    }
    else
    {
        limitsBeginTrim = cmd::TrimClip::determineBoundaries(getSequence(), mClip, link, ClipBegin, true);
        limitsEndTrim = cmd::TrimClip::determineBoundaries(getSequence(), mClip, link, ClipEnd, true);
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

    bool oneButtonEnabled = false;
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
            oneButtonEnabled = true;
        }
    }
    mBitmapLength->SetBitmap(util::window::getBitmap(oneButtonEnabled ? "clock-select-blue.png" : "clock-select-bw.png" ));

}

void DetailsClip::updateSpeedControls()
{
    bool video = mClip->isA<model::VideoClip>();
    bool audio = mClip->isA<model::AudioClip>();
    bool enable = (video || audio) && (cmd::EditClipSpeed::isPossible(getSequence(), mClip, mClip->getLink()).first);
    mSpeedSlider->Enable(enable);
    mSpeedSpin->Enable(enable);
    mBitmapSpeed->SetBitmap(util::window::getBitmap(enable ? "dashboard-blue.png" : "dashboard-bw.png"));
    if (video || audio)
    {
        rational64 speed = boost::dynamic_pointer_cast<model::ClipInterval>(mClip)->getSpeed();
        mSpeedSlider->SetValue(factorToSliderValue(speed));
        mSpeedSpin->SetValue(boost::rational_cast<double>(speed));
    }
}

void DetailsClip::updateIcons()
{
    bool show = Config::get().read<bool>(Config::sPathDetailsShowIcons);
    for (auto bitmap : mBitmaps)
    {
        bitmap->Show(show);                
    }
    for (auto bitmap : mBitmapsTransitionParameters)
    {
        bitmap->Show(show);
    }
    DetailsPanel::updateIcons();
}

void DetailsClip::updateTitles()
{
    bool show = Config::get().read<bool>(Config::sPathDetailsShowTitles);
    for (auto title : mTitlesTransitionParameters)
    {
        title->Show(show && mClip && mClip->isA<model::Transition>() && mClip->isA<model::IVideo>());
    }
    DetailsPanel::updateTitles();
}

int DetailsClip::getNumberOfColumns() const
{
    return 3;
}

void DetailsClip::makeTransitionCloneAndCreateTransitionParameterWidgets(model::IClipPtr clip)
{
    model::TransitionPtr transition{ boost::dynamic_pointer_cast<model::Transition>(clip) };
    if (transition)
    {
        mTransitionClone = make_cloned<model::Transition>(transition);
        createTransitionParameterWidgets();
    }
}

void DetailsClip::createTransitionParameterWidgets()
{
    if (mTransitionClone)
    {
        mTransitionType->Clear();
        for (auto n_and_transition : getPossibleVideoTransitions())
        {
            mTransitionType->Append(n_and_transition.second->getDescription(mTransitionClone->getTransitionType()));
        }
        mTransitionType->SetStringSelection(mTransitionClone->getDescription());
        mBitmapsTransitionParameters.clear();
        mTitlesTransitionParameters.clear();

        bool showTitles = Config::get().read<bool>(Config::sPathDetailsShowTitles);
        bool showIcons = Config::get().read<bool>(Config::sPathDetailsShowIcons);

        for (auto parameter : mTransitionClone->getSortedParameters())
        {
            for (int i = 0; i < getNumberOfColumns(); ++i)
            {
                mTransitionBoxSizer->AddSpacer(2);
            }
            wxStaticText* title{ new wxStaticText(mTransitionPanel, wxID_ANY, parameter->getDescription(), wxDefaultPosition, wxSize(120, -1), wxST_ELLIPSIZE_END) };
            mTitlesTransitionParameters.push_back(title);
            wxStaticBitmap* bitmapParameter = new wxStaticBitmap(mTransitionBoxSizer->GetContainingWindow(), wxID_ANY, parameter->getBitmap());
            mBitmapsTransitionParameters.push_back(bitmapParameter);
            mTransitionBoxSizer->Add(title, wxSizerFlags(0).CenterVertical().Left());//, 0, wxALL|wxALIGN_TOP, 0);
            mTransitionBoxSizer->Add(bitmapParameter, wxSizerFlags(0).Left().CenterVertical());
            wxWindow* window = parameter->makeWidget(mTransitionPanel);
            mTransitionBoxSizer->Add(window, wxSizerFlags(1).Expand());
            bitmapParameter->SetToolTip(window->GetToolTipText());
            title->SetToolTip(window->GetToolTipText());
            parameter->Bind(model::EVENT_TRANSITION_PARAMETER_CHANGED, &DetailsClip::onTransitionParameterChanged, this);
            title->Show(showTitles);
            bitmapParameter->Show(showIcons);
        }

        mTransitionPanel->Layout();
        mTransitionPanel->Update();
        mTransitionBoxSizer->Layout();
        Update();

        show(mTransitionPanel, true);
    }
}

void DetailsClip::destroyTransitionParameterWidgets()
{
    show(mTransitionPanel, false);

    if (mTransitionClone)
    {
        mBitmapsTransitionParameters.clear();
        mTitlesTransitionParameters.clear();
        for (auto parameter : mTransitionClone->getSortedParameters())
        {
            parameter->Unbind(model::EVENT_TRANSITION_PARAMETER_CHANGED, &DetailsClip::onTransitionParameterChanged, this);
            parameter->destroyWidget();
        }
        mTransitionBoxSizer->Clear(true); // Destroy all the titles also.
    }
}

std::map<int, model::TransitionPtr> DetailsClip::getPossibleVideoTransitions() const
{

    std::map<int, model::TransitionPtr> result;
    if (mClip &&
        mClip->isA<model::Transition>() &&
        mClip->isA<model::IVideo>())
    {
        int n{ 0 };
        std::vector<model::TransitionPtr> all{ model::video::VideoTransitionFactory::get().getAllPossibleTransitionsOfType(mTransitionClone->getTransitionType()) };
        std::sort(all.begin(), all.end(), [](model::TransitionPtr t1, model::TransitionPtr t2) { return t1->getDescription() < t2->getDescription(); });
        for (model::TransitionPtr transition : all)
        {
            result[n] = transition;
            ++n;
        }
    }
    return result;
}

}} // namespace
