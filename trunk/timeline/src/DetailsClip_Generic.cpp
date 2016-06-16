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

#include "AudioClip.h"
#include "AudioKeyFrame.h"
#include "UtilException.h"
#include "UtilWindow.h"
#include "VideoClip.h"
#include "VideoKeyFrame.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// GENERICS
//////////////////////////////////////////////////////////////////////////

template <typename CLIPTYPE>
boost::shared_ptr<CLIPTYPE> getClipOfType(const model::IClipPtr& clip)
{
    if (!clip)
    {
        return nullptr;
    }
    if (clip->isA<CLIPTYPE>())
    {
        return boost::dynamic_pointer_cast<CLIPTYPE>(clip);
    }
    return boost::dynamic_pointer_cast<CLIPTYPE>(clip->getLink());
}

template <typename CLIPTYPE, typename KEYFRAMETYPE>
struct KeyFrameControlsImpl
{
    KeyFrameControlsImpl(DetailsClip* parent, const wxString& tooltip)
        : mParent(parent)
        , mToolTip(tooltip)
    {
        mEditPanel = new wxPanel(mParent);
        wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
        mBitmapKeyFrames = new wxStaticBitmap(mEditPanel, wxID_ANY, util::window::getBitmap("key-blue.png"));
        mBitmapKeyFrames->SetToolTip(mToolTip);
        std::map<wxButton**, std::pair<wxString, wxString>> buttons
        {
            { &mHomeButton,{ "icon-home.png", _("Go to first key frame.") } },
            { &mPrevButton,{ "icon-previous.png", _("Go to previous key frame.") } },
            { &mNextButton,{ "icon-next.png", _("Go to next key frame.") } },
            { &mEndButton,{ "icon-end.png", _("Go to last key frame.") } },
            { &mAddButton,{ "icon-plus.png", _("Add a key frame at the current position.") } },
            { &mRemoveButton,{ "icon-minus.png", _("Remove the key frame at the current position.") } },
        };
        for (std::pair<wxButton**, std::pair<wxString, wxString>> button : buttons)
        {
            (*button.first) = new wxButton(mEditPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            (*button.first)->SetBitmap(util::window::getBitmap(button.second.first));
            (*button.first)->SetBitmapMargins(0, 0);
            (*button.first)->SetMinSize(wxSize{ wxDefaultCoord, wxButton::GetDefaultSize().GetHeight() });
            (*button.first)->SetToolTip(button.second.second);
        }
        sizer->Add(mBitmapKeyFrames, wxSizerFlags(0).Left().CenterVertical());

        sizer->Add(mHomeButton, wxSizerFlags(0));
        sizer->Add(mPrevButton, wxSizerFlags(0));
        mPanel = new wxPanel(mEditPanel);
        sizer->Add(mPanel, wxSizerFlags(1));
        sizer->Add(mNextButton, wxSizerFlags(0));
        sizer->Add(mEndButton, wxSizerFlags(0));
        sizer->Add(mAddButton, wxSizerFlags(0));
        sizer->Add(mRemoveButton, wxSizerFlags(0));
        mEditPanel->SetSizer(sizer);

        mHomeButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onHome, this);
        mPrevButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onPrev, this);
        mNextButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onNext, this);
        mEndButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onEnd, this);
        mAddButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onAdd, this);
        mRemoveButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onRemove, this);

    }
    ~KeyFrameControlsImpl()
    {
        mHomeButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onHome, this);
        mPrevButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onPrev, this);
        mNextButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onNext, this);
        mEndButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onEnd, this);
        mAddButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onAdd, this);
        mRemoveButton->Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &KeyFrameControlsImpl::onRemove, this);
        for (auto button : mKeyFramesButtons)
        {
            button.second->Unbind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &KeyFrameControlsImpl::onNumberedButton, this);
        }
    }

    void update()
    {
        updateKeyFrameSettings();

        boost::shared_ptr<CLIPTYPE> typedclip{ getClip() };

        mParent->showOption(mEditPanel, typedclip != nullptr);

        if (!typedclip) { return; }
        if (!typedclip->getTrack()) { return; }

        model::KeyFrameMap keyframes{ typedclip->getKeyFramesOfPerceivedClip() };

        boost::shared_ptr<KEYFRAMETYPE> keyFrame{ getKeyFrame() };
        ASSERT_NONZERO(keyFrame);
        pts keyFrameOffset{ getKeyFrameOffset() };

        if (mKeyFramesButtons.size() != keyframes.size())
        {
            for (auto button : mKeyFramesButtons) { button.second->Unbind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &KeyFrameControlsImpl::onNumberedButton, this); }

            mPanel->DestroyChildren();
            mKeyFramesButtons.clear();

            wxBoxSizer* sizer{ new wxBoxSizer{ wxHORIZONTAL } };
            for (size_t i{ 0 }; i < keyframes.size(); ++i)
            {
                mKeyFramesButtons[i] = new wxToggleButton{ mPanel, static_cast<int>(i), wxString::Format("%ld", i + 1), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT };
                mKeyFramesButtons[i]->SetFont(mKeyFramesButtons[i]->GetFont().Smaller().Smaller());
                mKeyFramesButtons[i]->SetToolTip(wxString::Format(_("Select key frame number %ld. Clicking on the key frame indicator in the timeline has the same effect."), i + 1));
                sizer->Add(mKeyFramesButtons[i], wxSizerFlags{ 1 });
            }
            mPanel->SetSizerAndFit(sizer);
            mEditPanel->Layout(); // Required for properly updating button lengths after adding/removing key frames.

            for (auto button : mKeyFramesButtons) { button.second->Bind(wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, &KeyFrameControlsImpl::onNumberedButton, this); }
        }
        ASSERT_EQUALS(mKeyFramesButtons.size(), keyframes.size());

        mHomeButton->Enable(!keyframes.empty() && keyFrameOffset > keyframes.cbegin()->first);
        mPrevButton->Enable(!keyframes.empty() && keyFrameOffset > keyframes.cbegin()->first);
        mNextButton->Enable(!keyframes.empty() && keyFrameOffset < keyframes.crbegin()->first);
        mEndButton->Enable(!keyframes.empty() && keyFrameOffset < keyframes.crbegin()->first);
        mRemoveButton->Enable(!keyframes.empty() && keyframes.find(keyFrameOffset) != keyframes.end());
        // Note: if there are no keyframes, there are no buttons to dis/enable.
        model::KeyFrameMap::const_iterator it{ keyframes.begin() };
        for (size_t i{ 0 }; i < keyframes.size(); ++i)
        {
            mKeyFramesButtons[i]->SetValue(it->first == keyFrameOffset);
            ++it;
        }

        // Only enable if cursor is 'inside' clip visible region
        bool cursorInBetween{ typedclip->getPerceivedLeftPts() <= getCursor() && typedclip->getPerceivedRightPts() >= getCursor() }; // The inclusion on the right hand size (getPerceivedRightPts() >= cursor) is required for enabling a key frame AFTER the last frame position.
        mAddButton->Enable(cursorInBetween && (keyframes.empty() || keyFrame->isInterpolated()));
        mBitmapKeyFrames->SetBitmap(util::window::getBitmap(!keyframes.empty() || mAddButton->IsEnabled() ? "key-blue.png" : "key-bw.png"));

        int availableSize{ mPanel->GetSize().x };
        int requiredSize{ mPanel->GetEffectiveMinSize().x };

        if (mKeyFramesButtons.size() > 0)
        {
            // Not all the buttons will fit in the available space. Show as much buttons as possible,
            // while keeping the current video offset approximately centered.
            //
            // Note that this algorithm is first called with a size '0' (first creation of details view)
            // leading to disabling of a lot of buttons initially. Therefore, this algorithm is
            // re-executed always, even if there's enough room for all buttons (because these buttons

            int totalNumberOfButtons{ static_cast<int>(mKeyFramesButtons.size()) };
            int buttonWidth{ requiredSize / totalNumberOfButtons };
            int maxFittingButtons{ availableSize / buttonWidth };
            int lastPossibleButton{ totalNumberOfButtons - 1 };
            int middle{ static_cast<int>(std::distance(keyframes.begin(), std::find_if(keyframes.begin(), keyframes.end(), [keyFrameOffset](auto kvp) { return kvp.first >= keyFrameOffset; }))) };
            int move{ maxFittingButtons / 2 };
            int first{ middle - move };
            int last{ middle + move };
            while (first < 0) { ++first; ++last; }
            while (last > lastPossibleButton) { --last; --first; }
            for (int count{ 0 }; count < static_cast<int>(mKeyFramesButtons.size()); ++count)
            {
                mKeyFramesButtons[count]->Show(count >= first && count <= last);
            }
        }
        mPanel->Layout();
    }

    void updateKeyFrameSettings();

    pts getKeyFrameOffset()
    {
        boost::shared_ptr<CLIPTYPE> typedclip{ getClipOfType<CLIPTYPE>(mParent->getClip()) };
        ASSERT_NONZERO(typedclip)(mParent->getClip());
        ASSERT_NONZERO(typedclip->getTrack())(mParent->getClip());

        pts firstFrame{ typedclip->getPerceivedLeftPts() }; // This is the 'included' boundary
        pts lastFrame{ typedclip->getPerceivedRightPts() }; // This is the 'excluded' boundary

        pts result{ -1 };
        if (getCursor() <= firstFrame)
        {
            // Cursor before clip: use first frame
            result = 0;
        }
        else if (getCursor() >= lastFrame)
        {
            // Cursor after clip : use last frame
            result = lastFrame - firstFrame - 1 + 1; // The extra +1 ensures that the last key frame may be positioned AFTER the last visible frame of the clip(for proper interpolation for that last frame).
        }
        else
        {
            // Inside the clip being adjusted
            result = getCursor() - firstFrame;
        }

        ASSERT_MORE_THAN_EQUALS_ZERO(result);
        return result;
    }

    boost::shared_ptr<CLIPTYPE> getClip()
    {
        return getClipOfType<CLIPTYPE>(mParent->getClip());
    }

    pts getCursor()
    {
        return mParent->getCursor().getLogicalPosition();
    }

    std::map<pts, boost::shared_ptr<KEYFRAMETYPE>> getKeyFrames()
    {
        boost::shared_ptr<CLIPTYPE> typedclip{ getClip() };
        ASSERT_NONZERO(typedclip)(mParent->getClip());
        ASSERT_NONZERO(typedclip->getTrack())(mParent->getClip());

        std::map<pts, boost::shared_ptr<KEYFRAMETYPE>> result;

        for (auto kvp : typedclip->getKeyFramesOfPerceivedClip())
        {
            boost::shared_ptr<KEYFRAMETYPE> keyframe{ boost::dynamic_pointer_cast<KEYFRAMETYPE>(kvp.second) };
            ASSERT_NONZERO(keyframe);
            result[kvp.first] = keyframe;
        }

        return result;
    }
    boost::shared_ptr<KEYFRAMETYPE> getKeyFrame()
    {
        boost::shared_ptr<CLIPTYPE> typedclip{ getClip() };
        if (!typedclip) { return nullptr; }
        if (!typedclip->getTrack()) { return nullptr; }

        boost::shared_ptr<KEYFRAMETYPE> result{ nullptr };
        std::map<pts, boost::shared_ptr<KEYFRAMETYPE>> keyframes{ getKeyFrames() };
        if (keyframes.empty())
        {
            // Clip without key frames. Use the default key frame for the overall clip settings
            result = boost::dynamic_pointer_cast<KEYFRAMETYPE>(typedclip->getDefaultKeyFrame());
        }
        else
        {
            pts position{ getKeyFrameOffset() };
            typename std::map<pts, boost::shared_ptr<KEYFRAMETYPE>>::const_iterator it{ keyframes.find(position) };
            if (it != keyframes.end())
            {
                result = it->second;
            }
            else
            {
                // Return interpolated frame (for previewing the values)
                result = boost::dynamic_pointer_cast<KEYFRAMETYPE>(typedclip->getFrameAt(position));
            }
        }
        ASSERT_NONZERO(result);
        return result;
    }

    void moveCursor(std::function<pts(const model::KeyFrameMap&, pts)> determineNewOffset)
    {
        CatchExceptions([this, determineNewOffset]
        {
            // This is only about moving the cursor. Err on the safe side: ignore.
            boost::shared_ptr<CLIPTYPE> typedclip{ getClip() };
            if (!typedclip) { return; }
            model::KeyFrameMap keyFrames{ typedclip->getKeyFramesOfPerceivedClip() };
            if (keyFrames.size() == 0) { return; }
            pts currentOffset{ getKeyFrameOffset() };
            pts newOffset{ determineNewOffset(keyFrames, currentOffset) };
            if (keyFrames.find(newOffset) == keyFrames.end()) { return; }
            mParent->getCursor().setLogicalPosition(typedclip->getPerceivedLeftPts() + newOffset);
        });
    }

    void onHome(wxCommandEvent& event)
    {
        moveCursor([](const model::KeyFrameMap& keyFrames, pts current) -> pts
        {
            return keyFrames.begin()->first;
        });
        event.Skip();
    }

    void onPrev(wxCommandEvent& event)
    {
        moveCursor([](const model::KeyFrameMap& keyFrames, pts current) -> pts
        {
            pts result{ 0 };
            model::KeyFrameMap::const_iterator it{ keyFrames.begin() };
            while (it != keyFrames.end() && it->first < current)
            {
                result = it->first;
                ++it;
            }
            if (it == keyFrames.end())
            {
                // offset was beyond last key frame. Move to last key frame.
                result = keyFrames.rbegin()->first;
            }
            return result;
        });
        event.Skip();
    }

    void onNumberedButton(wxCommandEvent& event)
    {
        size_t buttonNumber{ static_cast<size_t>(event.GetId()) };
        moveCursor([&buttonNumber](const model::KeyFrameMap& keyFrames, pts current) -> pts
        {
            model::KeyFrameMap::const_iterator it{ keyFrames.begin() };
            for (; it != keyFrames.end() && buttonNumber > 0; ++it, --buttonNumber);
            if (it == keyFrames.end()) { return keyFrames.rbegin()->first; } // Err on the safe side of caution
            return it->first;
        });
        event.Skip();
    }

    void onNext(wxCommandEvent& event)
    {
        moveCursor([](const model::KeyFrameMap& keyFrames, pts current) -> pts
        {
            pts result{ 0 };
            model::KeyFrameMap::const_reverse_iterator it{ keyFrames.rbegin() };
            while (it != keyFrames.rend() && it->first > current)
            {
                result = it->first;
                ++it;
            }
            if (it == keyFrames.rend())
            {
                // offset was before first key frame. Move to first key frame.
                result = keyFrames.begin()->first;
            }
            return result;
        });
        event.Skip();
    }

    void onEnd(wxCommandEvent& event)
    {
        moveCursor([](const model::KeyFrameMap& keyFrames, pts current) -> pts
        {
            return keyFrames.rbegin()->first;
        });
        event.Skip();
    }

    void onAdd(wxCommandEvent& event)
    {
        CatchExceptions([this]
        {
            mParent->submitEditCommandUponAudioVideoEdit(mParent->sEditKeyFramesAdd, std::is_same<CLIPTYPE, model::VideoClip>::value, [this]
            {
                getClip()->addKeyFrameAt(getKeyFrameOffset(), getKeyFrame());
            });
            update();
        });
        event.Skip();
    };

    void onRemove(wxCommandEvent& event)
    {
        CatchExceptions([this]
        {
            mParent->submitEditCommandUponAudioVideoEdit(mParent->sEditKeyFramesRemove, std::is_same<CLIPTYPE, model::VideoClip>::value, [this]
            {
                getClip()->removeKeyFrameAt(getKeyFrameOffset());
            });
            update();
        });
    }

    DetailsClip* mParent = nullptr;

    wxPanel* mEditPanel = nullptr;
    wxStaticBitmap* mBitmapKeyFrames = nullptr;
    wxButton* mHomeButton = nullptr;
    wxButton* mPrevButton = nullptr;
    wxButton* mNextButton = nullptr;
    wxButton* mEndButton = nullptr;
    wxButton* mAddButton = nullptr;
    wxButton* mRemoveButton = nullptr;
    wxPanel* mPanel = nullptr;
    std::map<size_t, wxToggleButton*> mKeyFramesButtons;
    wxString mToolTip;
};

typedef KeyFrameControlsImpl < model::VideoClip, model::VideoKeyFrame > VideoKeyFrameControls;
typedef KeyFrameControlsImpl < model::AudioClip, model::AudioKeyFrame > AudioKeyFrameControls;

template <>
void VideoKeyFrameControls::updateKeyFrameSettings()
{
    model::VideoKeyFramePtr videoKeyFrame{ getKeyFrame() };

    mParent->showOption(mParent->mOpacityPanel, videoKeyFrame != nullptr);
    mParent->showOption(mParent->mCropPanel, videoKeyFrame != nullptr);
    mParent->showOption(mParent->mScalingPanel, videoKeyFrame != nullptr);
    mParent->showOption(mParent->mRotationPanel, videoKeyFrame != nullptr);
    mParent->showOption(mParent->mAlignmentPanel, videoKeyFrame != nullptr);

    if (!videoKeyFrame) { return; }

    rational64 factor{ videoKeyFrame->getScalingFactor() };
    rational64 rotation{ videoKeyFrame->getRotation() };
    wxPoint position{ videoKeyFrame->getPosition() };
    wxPoint maxpos{ videoKeyFrame->getMaxPosition() };
    wxPoint minpos{ videoKeyFrame->getMinPosition() };
    int opacity{ videoKeyFrame->getOpacity() };
    int cropTop{ videoKeyFrame->getCropTop() };
    int cropBottom{ videoKeyFrame->getCropBottom() };
    int cropLeft{ videoKeyFrame->getCropLeft() };
    int cropRight{ videoKeyFrame->getCropRight() };

    mParent->mOpacitySlider->SetValue(opacity);
    mParent->mOpacitySpin->SetValue(opacity);

    mParent->mCropTopSlider->SetValue(cropTop);
    mParent->mCropTopSpin->SetValue(cropTop);
    mParent->mCropBottomSlider->SetValue(cropBottom);
    mParent->mCropBottomSpin->SetValue(cropBottom);
    mParent->mCropLeftSlider->SetValue(cropLeft);
    mParent->mCropLeftSpin->SetValue(cropLeft);
    mParent->mCropRightSlider->SetValue(cropRight);
    mParent->mCropRightSpin->SetValue(cropRight);

    mParent->mSelectScaling->select(videoKeyFrame->getScaling());
    mParent->mScalingSlider->SetValue(DetailsClip::factorToSliderValue(factor));
    mParent->mScalingSpin->SetValue(boost::rational_cast<double>(factor));

    // In case of no key frames, rotation ranges from -180 to 180.
    // In case of key frames, multiple rotations can be set (to show a rotating clip).
    model::VideoKeyFrameMap keyframes{ getKeyFrames() };
    int rotationMin{ keyframes.empty() ? DetailsClip::sRotationMinNoKeyFrames : DetailsClip::sRotationMinKeyFrames };
    int rotationMax{ keyframes.empty() ? DetailsClip::sRotationMaxNoKeyFrames : DetailsClip::sRotationMaxKeyFrames };
    mParent->mRotationSlider->SetMin(rotationMin);
    mParent->mRotationSlider->SetMax(rotationMax);
    mParent->mRotationSpin->SetRange(
        static_cast<double>(rotationMin) / static_cast<double>(DetailsClip::sRotationPrecisionFactor),
        static_cast<double>(rotationMax) / static_cast<double>(DetailsClip::sRotationPrecisionFactor));
    mParent->mRotationSlider->SetValue(boost::rational_cast<int>(rotation * DetailsClip::sRotationPrecisionFactor));
    mParent->mRotationSpin->SetValue(boost::rational_cast<double>(rotation));

    mParent->mSelectAlignment->select(videoKeyFrame->getAlignment());
    mParent->mPositionXSlider->SetRange(minpos.x, maxpos.x);
    mParent->mPositionXSlider->SetValue(position.x);
    mParent->mPositionXSpin->SetRange(minpos.x, maxpos.x);
    mParent->mPositionXSpin->SetValue(position.x);
    mParent->mPositionYSlider->SetRange(minpos.y, maxpos.y);
    mParent->mPositionYSlider->SetValue(position.y);
    mParent->mPositionYSpin->SetRange(minpos.y, maxpos.y);
    mParent->mPositionYSpin->SetValue(position.y);

    mParent->mOpacitySlider->Enable(!videoKeyFrame->isInterpolated());
    mParent->mOpacitySpin->Enable(!videoKeyFrame->isInterpolated());
    mParent->mCropTopSlider->Enable(!videoKeyFrame->isInterpolated());
    mParent->mCropTopSpin->Enable(!videoKeyFrame->isInterpolated());
    mParent->mCropBottomSlider->Enable(!videoKeyFrame->isInterpolated());
    mParent->mCropBottomSpin->Enable(!videoKeyFrame->isInterpolated());
    mParent->mCropLeftSlider->Enable(!videoKeyFrame->isInterpolated());
    mParent->mCropLeftSpin->Enable(!videoKeyFrame->isInterpolated());
    mParent->mCropRightSlider->Enable(!videoKeyFrame->isInterpolated());
    mParent->mCropRightSpin->Enable(!videoKeyFrame->isInterpolated());
    mParent->mSelectScaling->Enable(!videoKeyFrame->isInterpolated());
    mParent->mScalingSlider->Enable(!videoKeyFrame->isInterpolated());
    mParent->mScalingSpin->Enable(!videoKeyFrame->isInterpolated());
    mParent->mRotationSlider->Enable(!videoKeyFrame->isInterpolated());
    mParent->mRotationSpin->Enable(!videoKeyFrame->isInterpolated());
    mParent->mSelectAlignment->Enable(!videoKeyFrame->isInterpolated());
    mParent->mPositionXSlider->Enable(!videoKeyFrame->isInterpolated());
    mParent->mPositionXSpin->Enable(!videoKeyFrame->isInterpolated());
    mParent->mPositionYSlider->Enable(!videoKeyFrame->isInterpolated());
    mParent->mPositionYSpin->Enable(!videoKeyFrame->isInterpolated());
}

template <>
void AudioKeyFrameControls::updateKeyFrameSettings()
{
    model::AudioKeyFramePtr audioKeyFrame{ getKeyFrame() };

    mParent->showOption(mParent->mVolumePanel, audioKeyFrame != nullptr);

    if (!audioKeyFrame) { return; }

    int volume{ audioKeyFrame->getVolume() };

    mParent->mVolumeSlider->SetValue(volume);
    mParent->mVolumeSpin->SetValue(volume);

    mParent->mVolumeSlider->Enable(!audioKeyFrame->isInterpolated());
    mParent->mVolumeSpin->Enable(!audioKeyFrame->isInterpolated());
}

}} // namespace
