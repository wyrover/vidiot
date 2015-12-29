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
#include "AudioCompositionParameters.h"
#include "AudioFile.h"
#include "AudioPeaks.h"
#include "ClipView.h"
#include "Properties.h"
#include "Transition.h"
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
    explicit RenderPeaksWork(const model::IClipPtr& clip, const wxSize& size, rational64 zoom)
        : RenderClipPreviewWork(clip,size,zoom)
    {
        ASSERT(clip->isA<model::AudioClip>())(clip);

        mParameters.setSampleRate(model::Properties::get().getAudioSampleRate()).setNrChannels(1).setPts(0).determineChunkSize();

        // Can't make the clone in the separate thread, hence this duplication.
        // Otherwise, the clip may be (partially) opened/opening in the main thread at the moment
        // the clone is made in the createBitmap method. That resulted in empty peaks views,
        // because clipclone->fileclone::mFileOpenedOk was not yet initialized.
        mAudioClipClone = make_cloned<model::AudioClip>(boost::dynamic_pointer_cast<model::AudioClip>(clip));
    }

    wxImagePtr createBitmap() override
    {
        // Note: if this would return a nullptr then scheduling would be repeated over and over again, since nothing is generated.
        ASSERT_MORE_THAN_EQUALS(mSize.x, 2); // Avoid drawing over the clip bounds
        ASSERT_MORE_THAN_EQUALS(mSize.y, 10); 
        if (!wxThread::IsMain())
        {
            setThreadName("RenderPeaks");
        }

        wxImagePtr result = boost::make_shared<wxImage>(mSize);
        memset(result->GetData(), 0, mSize.x * mSize.y * 3); // Default: all black (is the transparent colour)

        model::TransitionPtr inTransition{ mClip->getInTransition() };
        if (inTransition != nullptr)
        {
            ASSERT(inTransition->getRight())(inTransition);
            ASSERT_MORE_THAN_EQUALS_ZERO(*inTransition->getRight());
            mAudioClipClone->adjustBegin(-1 * *inTransition->getRight());
        }
        model::TransitionPtr outTransition{ mClip->getOutTransition() };
        if (outTransition != nullptr)
        {
            ASSERT(outTransition->getLeft())(outTransition);
            ASSERT_MORE_THAN_EQUALS_ZERO(*outTransition->getLeft());
            mAudioClipClone->adjustEnd(*outTransition->getLeft());
        }
        ASSERT(mAudioClipClone);
        ASSERT(!mAudioClipClone->getTrack()); // NOTE: This is a check to ensure that a clone is used, and not the original is 'moved'
        if (mAudioClipClone->getLength() > 0)
        {
            // The if is required to avoid errors during editing operations.
            int origin{ mSize.y / 2 };

            for (int x = 0; x < mSize.x; ++x)
            {
                result->SetRGB(wxRect{ 0, origin, mSize.x, 1 }, 87, 120, 74);
            }

            model::AudioPeaks peaks = mAudioClipClone->getPeaks(mParameters);
            int nPeaks = peaks.size();

            if (nPeaks > 0)
            {
                int peakIndex{ 0 };
                model::AudioPeak peak = peaks[0];
                ASSERT_LESS_THAN_EQUALS(peak.first, peak.second);

                int totalPeaks = mAudioClipClone->getFile()->getLength();
                int totalPixels = Zoom::ptsToPixels(totalPeaks, mZoom);

                int firstPeak = mAudioClipClone->getOffset();
                int firstPixel = Zoom::ptsToPixels(firstPeak, mZoom);

                for (int x{ 0 }; x < mSize.GetWidth() && !isAborted(); ++x)
                {
                    // Always computed wrt the total file length. This ensures consistent drawing during trimming operations.
                    // Without this, during trimming the displayed images flickers a bit (because of rounding issues).
                    //
                    // Due to this computation, rounding errors sometimes lead to 'too high' values. Hence, the std::min.
                    int requiredPeakIndex = std::min(nPeaks - 1, boost::rational_cast<int>(rational64(firstPixel + x, totalPixels) * rational64(totalPeaks - 1)) - firstPeak);

                    // Do not initialize with '0': that gives problems when zooming in further than 1-on-1.
                    // In that case, the body of the while loop below is never entered.
                    int negativePeak = boost::rational_cast<int>(rational64(mSize.y, 2) * rational64(peak.first, std::numeric_limits<sample>::min()));; // Note: - * - = +
                    int positivePeak = boost::rational_cast<int>(rational64(mSize.y, 2) * rational64(peak.second, std::numeric_limits<sample>::max()));

                    while (peakIndex < requiredPeakIndex)
                    {
                        peakIndex++;
                        peak = peaks[peakIndex];
                        negativePeak = std::max(negativePeak, boost::rational_cast<int>(rational64(mSize.y, 2) * rational64(peak.first, std::numeric_limits<sample>::min()))); // Note: - * - = +
                        positivePeak = std::max(positivePeak, boost::rational_cast<int>(rational64(mSize.y, 2) * rational64(peak.second, std::numeric_limits<sample>::max())));
                    }
                    ASSERT_MORE_THAN_EQUALS_ZERO(negativePeak);
                    ASSERT_MORE_THAN_EQUALS_ZERO(positivePeak);
                    int h{ positivePeak + negativePeak };
                    ASSERT_MORE_THAN_EQUALS_ZERO(h);
                    int y{ std::max(0,origin - negativePeak) };
                    result->SetRGB(wxRect{ x, y, 1, h }, 87, 120, 74); 
                }
            }
        }
        result->SetMaskColour(0,0,0);
        return result;
    }
    model::AudioClipPtr mAudioClipClone = nullptr;
    model::AudioCompositionParameters mParameters;
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

AudioPeakView::AudioPeakView(const model::IClipPtr& clip, View* parent)
    : ClipPreview(clip, parent)
{
    ASSERT(mClip->isA<model::IAudio>())(mClip);

}

AudioPeakView::~AudioPeakView()
{
}

//////////////////////////////////////////////////////////////////////////
// CLIPPREVIEW
//////////////////////////////////////////////////////////////////////////

RenderClipPreviewWorkPtr AudioPeakView::render() const
{
    return boost::make_shared<RenderPeaksWork>(mClip, getSize(), getZoom().getCurrent());
}

wxSize AudioPeakView::getRequiredSize() const
{
    wxSize result{ getParent().getW() - 2 * ClipView::getBorderSize(), getParent().getH() - ClipView::getBorderSize() - ClipView::getDescriptionHeight() };
    return result;
}

wxSize AudioPeakView::getMinimumSize() const
{
    return wxSize(2,10);
}


}} // namespace
