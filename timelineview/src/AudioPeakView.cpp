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

#include "AudioPeakView.h"

#include "AudioClip.h"
#include "AudioClipEvent.h"
#include "AudioFile.h"
#include "AudioPeaks.h"
#include "Layout.h"
#include "Transition.h"
#include "UtilClone.h"
#include "UtilInt.h"
#include "UtilLog.h"
#include "Zoom.h"

namespace gui { namespace timeline {

////////////////////////////////////////////////////////////////////////
// WORK OBJECT FOR RENDERING A THUMBNAIL
//////////////////////////////////////////////////////////////////////////

struct RenderPeaksWork
    : public RenderClipPreviewWork
{
    // Here, all access to folder must be done, not in the worker thread.
    // Rationale: all access to model objects must be done in the main thread!
    explicit RenderPeaksWork(const model::IClipPtr& clip, const wxSize& size, rational zoom)
        : RenderClipPreviewWork(clip,size,zoom)
    {
    }

    wxImagePtr createBitmap() override
    {
        if (!wxThread::IsMain())
        {
            setThreadName("RenderPeaks");
        }

        wxImagePtr result = boost::make_shared<wxImage>(mSize);
        result->InitAlpha();
        memset(result->GetAlpha(), 0, mSize.x * mSize.y);

        wxGraphicsContext* gc = wxGraphicsContext::Create(*result);
        gc->SetInterpolationQuality(wxINTERPOLATION_BEST);
        gc->SetAntialiasMode(wxANTIALIAS_NONE);
        gc->SetCompositionMode(wxCOMPOSITION_OVER);
        wxGCDC dc(gc); // When going out of scope the wxGraphicsContext* is also deleted.

        dc.SetPen(wxPen{ wxColour{ 87, 120, 74 }, 1 });

        model::AudioClipPtr clone = make_cloned<model::AudioClip>(boost::dynamic_pointer_cast<model::AudioClip>(mClip));

        model::TransitionPtr inTransition{ mClip->getInTransition() };
        if (inTransition != nullptr)
        {
            ASSERT(inTransition->getRight())(inTransition);
            ASSERT_MORE_THAN_EQUALS_ZERO(*inTransition->getRight());
            clone->adjustBegin(-1 * *inTransition->getRight());
        }
        model::TransitionPtr outTransition{ mClip->getOutTransition() };
        if (outTransition != nullptr)
        {
            ASSERT(outTransition->getLeft())(outTransition);
            ASSERT_MORE_THAN_EQUALS_ZERO(*outTransition->getLeft());
            clone->adjustEnd(*outTransition->getLeft());
        }
        ASSERT(clone);
        ASSERT(!clone->getTrack()); // NOTE: This is a check to ensure that a clone is used, and not the original is 'moved'
        if (clone->getLength() > 0)
        {
            // The if is required to avoid errors during editing operations.

            int origin{ mSize.y / 2 };
            dc.DrawLine(wxPoint(0, origin), wxPoint(mSize.x, origin));

            model::AudioPeaks peaks = clone->getPeaks();
            int nPeaks = peaks.size();

            if (nPeaks > 0)
            {
                int peakIndex{ 0 };
                model::AudioPeak peak = peaks.front();

                int totalPeaks = boost::dynamic_pointer_cast<model::AudioFile>(clone->getFile())->getLength();
                int totalPixels = Zoom::ptsToPixels(totalPeaks, mZoom);

                int firstPeak = clone->getOffset();
                int firstPixel = Zoom::ptsToPixels(firstPeak, mZoom);

                for (int x{ 0 }; x < mSize.GetWidth(); ++x)
                {
                    // Always computed wrt the total file length. This ensures consistent drawing during trimming operations.
                    // Without this, during trimming the displayed images flickers a bit (because of rounding issues).
                    //
                    // Due to this computation, rounding errors sometimes lead to 'too high' values. Hence, the std::min.
                    int requiredPeakIndex = std::min(nPeaks - 1, boost::rational_cast<int>(rational64(firstPixel + x, totalPixels) * rational64(totalPeaks - 1)) - firstPeak);

                    // Do not initialize with '0': that gives problems when zooming in further than 1-on-1.
                    // In that case, the body of the while loop below is never entered.
                    int min = boost::rational_cast<int>(rational64(mSize.y, 2) * rational64(peak.first, std::numeric_limits<sample>::min()));;
                    int max = boost::rational_cast<int>(rational64(mSize.y, 2) * rational64(peak.second, std::numeric_limits<sample>::max()));

                    while (peakIndex < requiredPeakIndex)
                    {
                        peakIndex++;
                        peak = peaks[peakIndex];
                        min = std::max(min, boost::rational_cast<int>(rational64(mSize.y, 2) * rational64(peak.first, std::numeric_limits<sample>::min())));
                        max = std::max(max, boost::rational_cast<int>(rational64(mSize.y, 2) * rational64(peak.second, std::numeric_limits<sample>::max())));
                    }

                    dc.DrawLine(wxPoint(x, origin - min), wxPoint(x, origin + max));
                }
            }
        }
        return result;
    }
};

    // todo make a 'missing file' then position cursor in that file and start playback. The video for that file is never ended...

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

AudioPeakView::AudioPeakView(const model::IClipPtr& clip, View* parent)
    : ClipPreview(clip, parent)
{
    mClip->Bind(model::EVENT_CHANGE_AUDIOCLIP_VOLUME, &AudioPeakView::onVolumeChanged, this);

}

AudioPeakView::~AudioPeakView()
{
    mClip->Unbind(model::EVENT_CHANGE_AUDIOCLIP_VOLUME, &AudioPeakView::onVolumeChanged, this);
}

//////////////////////////////////////////////////////////////////////////
// CLIPPREVIEW
//////////////////////////////////////////////////////////////////////////

RenderClipPreviewWorkPtr AudioPeakView::render() const
{
    return boost::make_shared<RenderPeaksWork>(mClip, getSize(), getZoom().getCurrent());
}

wxSize AudioPeakView::requiredSize() const
{
    return wxSize( getParent().getW() - 2 * Layout::ClipBorderSize, getParent().getH() - Layout::ClipBorderSize - Layout::ClipDescriptionBarHeight);
}

//////////////////////////////////////////////////////////////////////////
// AUDIOCLIP EVENTS
//////////////////////////////////////////////////////////////////////////

void AudioPeakView::onVolumeChanged(model::EventChangeAudioClipVolume& event)
{
    invalidateCachedBitmaps();
    invalidateRect();
    repaint();
    event.Skip();
}

}} // namespace
