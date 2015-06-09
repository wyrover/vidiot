// Copyright 2015 Eric Raijmakers.
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

#ifndef AUDIO_PEAK_VIEW_H
#define AUDIO_PEAK_VIEW_H

#include "ClipPreview.h"

namespace model {
    class EventChangeAudioClipVolume;
}

namespace gui { namespace timeline {

class AudioPeakView
    :   public ClipPreview
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    AudioPeakView(const model::IClipPtr& clip, View* parent);
    ~AudioPeakView();

    //////////////////////////////////////////////////////////////////////////
    // CLIPPREVIEW
    //////////////////////////////////////////////////////////////////////////

    RenderClipPreviewWorkPtr render() const override;
    wxSize getRequiredSize() const override;
    wxSize getMinimumSize() const override;

    //////////////////////////////////////////////////////////////////////////
    // AUDIOCLIP EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onVolumeChanged(model::EventChangeAudioClipVolume& event);

};

}} // namespace

#endif
