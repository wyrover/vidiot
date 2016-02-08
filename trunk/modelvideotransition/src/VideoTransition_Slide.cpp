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

#include "VideoTransition_Slide.h"

#include "TransitionParameterDirection8.h"
#include "VideoClip.h"
#include "VideoCompositionParameters.h"
#include "VideoFrameLayer.h"
#include "VideoSkipFrame.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Slide* Slide::clone() const
{
    return new Slide(static_cast<const Slide&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool Slide::supports(TransitionType type) const
{
    return true;
}

ParameterAttributes Slide::getAvailableParameters() const
{
    return
    {
        { TransitionParameterDirection8::sParameterDirection8, _("Direction"), _("Select the movement direction."), boost::make_shared<TransitionParameterDirection8>(Direction8RightToLeft) },
        { TransitionParameterBool::sParameterInversed, _("Inversed"), _("Select between 'sliding in' the second clip or 'sliding away' the first clip."), boost::make_shared<TransitionParameterBool>(false) },
    };
}

wxString Slide::getDescription(TransitionType type) const
{
    return _("Slide");
}

//////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION OF TRANSITION
//////////////////////////////////////////////////////////////////////////

VideoFramePtr Slide::getVideo(pts position, const IClipPtr& leftClip, const IClipPtr& rightClip, const VideoCompositionParameters& parameters)
{
    VAR_DEBUG(position)(parameters);
    VideoFramePtr result{ parameters.getSkip() ? boost::make_shared<VideoSkipFrame>(parameters) : boost::make_shared<VideoFrame>(parameters) };

    Direction8 direction{ getParameter<TransitionParameterDirection8>(TransitionParameterDirection8::sParameterDirection8)->getValue() };
    bool inverse{ getParameter<TransitionParameterBool>(TransitionParameterBool::sParameterInversed)->getValue() };

    float factor{ static_cast<float>(position) / static_cast<float>(getLength()) };

    wxRect r{ parameters.getRequiredRectangle() };
    int w{ r.GetWidth() };
    int h{ r.GetHeight() };

    wxPoint offset{ 0,0 };

    model::IClipPtr staticClip{ leftClip };
    model::IClipPtr slidingClip{ rightClip };

    if (inverse)
    {
        factor = 1.0 - factor;
        direction = TransitionParameterDirection8::getInversedDirection(direction);
        staticClip = rightClip;
        slidingClip = leftClip;
    }

    switch (direction)
    {
        case model::Direction8TopLeftToBottomRight:
            offset.x = std::trunc(factor * w) - w;
            offset.y = std::trunc(factor * h) - h;
            break;
        case model::Direction8TopToBottom:
            offset.y = std::trunc(factor * h) - h;
            break;
        case model::Direction8TopRightToBottomLeft:
            offset.x = w - std::trunc(factor * w);
            offset.y = std::trunc(factor * h) - h;
            break;
        case model::Direction8RightToLeft:
            offset.x = w - std::trunc(factor * w);
            break;
        case model::Direction8BottomRightToTopLeft:
            offset.x = w - std::trunc(factor * w);
            offset.y = h - std::trunc(factor * h);
            break;
        case model::Direction8BottomToTop:
            offset.y = h - std::trunc(factor * h);
            break;
        case model::Direction8BottomLeftToTopRight:
            offset.x = std::trunc(factor * w) - w;
            offset.y = h - std::trunc(factor * h);
            break;
        case model::Direction8LeftToRight:
            offset.x = std::trunc(factor * w) - w;
            break;
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

    addClip(staticClip, { 0,0 });
    addClip(slidingClip, offset);

    return result;
};

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Slide::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Slide::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Slide::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::Slide)
