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

#include "VideoTransition_Push.h"

#include "TransitionParameterDirection4.h"
#include "VideoClip.h"
#include "VideoCompositionParameters.h"
#include "VideoFrameLayer.h"
#include "VideoSkipFrame.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Push* Push::clone() const
{
    return new Push(static_cast<const Push&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool Push::supports(TransitionType type) const
{
    return true;
}

std::vector<std::tuple<wxString, wxString, TransitionParameterPtr>> Push::getAvailableParameters() const
{
    return
    {
        std::make_tuple(TransitionParameterDirection4::sParameterDirection4, _("Direction"), boost::make_shared<TransitionParameterDirection4>(Direction4LeftToRight))
    };
}

wxString Push::getDescription(TransitionType type) const
{
    return _("Push");
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF TRANSITION
//////////////////////////////////////////////////////////////////////////

VideoFramePtr Push::getVideo(pts position, const IClipPtr& leftClip, const IClipPtr& rightClip, const VideoCompositionParameters& parameters)
{
    VAR_DEBUG(position)(parameters);
    VideoFramePtr result{ parameters.getSkip() ? boost::make_shared<VideoSkipFrame>(parameters) : boost::make_shared<VideoFrame>(parameters) };

    Direction4 direction{ getParameter<TransitionParameterDirection4>(TransitionParameterDirection4::sParameterDirection4)->getValue() };

    float factor{ static_cast<float>(position) / static_cast<float>(getLength()) };

    wxRect r{ parameters.getRequiredRectangle() };
    int w{ r.GetWidth() };
    int h{ r.GetHeight() };

    wxPoint leftPositionOffset{ 0,0 };
    wxPoint rightPositionOffset{ 0,0 };

    switch (direction)
    {
        case model::Direction4TopToBottom:
            leftPositionOffset.y = std::trunc(factor * h);
            rightPositionOffset.y = leftPositionOffset.y - h;
            break;
        case model::Direction4RightToLeft:
            leftPositionOffset.x = std::trunc(-1 * factor * w);
            rightPositionOffset.x = leftPositionOffset.x + w;
            break;
        case model::Direction4BottomToTop:
            leftPositionOffset.y = -1 * factor * h;
            rightPositionOffset.y = leftPositionOffset.y + h;
            break;
        case model::Direction4LeftToRight:
            leftPositionOffset.x = factor * w;
            rightPositionOffset.x = leftPositionOffset.x - w;
            break;
        default:
            VAR_WARNING(position);        // todo add tests for all directions?
    }

    auto addClip = [parameters, result](const model::IClipPtr& clip, wxPoint offset)
    {
        if (clip == nullptr) { return; }
        VideoFramePtr frame{ boost::static_pointer_cast<VideoClip>(clip)->getNextVideo(parameters) };
        for (VideoFrameLayerPtr layer : frame->getLayers())
        {
            layer->setPosition(layer->getPosition() + offset);
            result->addLayer(layer);
        }
    };

    addClip(leftClip, leftPositionOffset);
    addClip(rightClip, rightPositionOffset);

    return result;
};

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Push::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Push::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Push::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::Push)
