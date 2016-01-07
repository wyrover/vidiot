// Copyright 2015-2016 Eric Raijmakers.
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

#include "Test.h"

namespace test {

model::VideoFramePtr GetFrame(model::IClipPtr clip, pts offset)
{
    ASSERT_MORE_THAN_EQUALS_ZERO(offset);
    ASSERT_LESS_THAN(offset, clip->getLength());
    model::IClipPtr clone{ make_cloned<model::IClip>(clip) };
    model::VideoClipPtr videoclip{ getVideoClip(clone) };
    ASSERT_NONZERO(videoclip);
    videoclip->moveTo(offset);
    return util::thread::RunInMainReturning<model::VideoFramePtr>([videoclip]
    {
        model::VideoCompositionParameters parameters; parameters.setPts(0).setBoundingBox(wxSize(1280, 720));
        model::VideoFramePtr result = videoclip->getNextVideo(parameters);
        ASSERT_NONZERO(result);
        return result;
    });
}

model::VideoFramePtr FirstFrame(model::IClipPtr clip)
{
    return GetFrame(clip, 0);
}

model::VideoFramePtr LastFrame(model::IClipPtr clip)
{
    return GetFrame(clip, clip->getLength() - 1);
}

void assertFramesEqual(model::VideoFramePtr frame1, model::VideoFramePtr frame2)
{
    wxImagePtr image1 = frame1->getImage();
    wxImagePtr image2 = frame2->getImage();
    //image1->SaveFile(wxFileName(wxStandardPaths::Get().GetTempDir(), "image1.png").GetFullPath());
    //image2->SaveFile(wxFileName(wxStandardPaths::Get().GetTempDir(), "image2.png").GetFullPath());
    pts ft1{ boost::rational_cast<pts>(frame1->getTime()) };
    pts ft2{ boost::rational_cast<pts>(frame2->getTime()) };
    ASSERT_EQUALS(frame1->getTime(), frame2->getTime())(ft1)(ft2);
    ASSERT_EQUALS(image1->HasAlpha(), image2->HasAlpha());
    ASSERT_EQUALS(image1->GetSize(), image2->GetSize());
    for (int x = 0; x < image1->GetWidth(); ++x)
    {
        for (int y = 0; y < image1->GetHeight(); ++y)
        {
            ASSERT_EQUALS(image1->GetRed(x, y), image2->GetRed(x, y))(x)(y);
            ASSERT_EQUALS(image1->GetGreen(x, y), image2->GetGreen(x, y))(x)(y);
            ASSERT_EQUALS(image1->GetBlue(x, y), image2->GetBlue(x, y))(x)(y);
            if (image1->HasAlpha()) { ASSERT_EQUALS(image1->GetAlpha(x, y), image2->GetAlpha(x, y))(x)(y); }
        }
    }
}

} // namespace
