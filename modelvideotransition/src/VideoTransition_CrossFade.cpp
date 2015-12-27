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

#include "VideoTransition_CrossFade.h"

#include "TransitionFactory.h"
#include "VideoClip.h"
#include "VideoComposition.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

CrossFade::CrossFade()
    :	VideoTransitionOpacity()
{
    VAR_DEBUG(this);
}

CrossFade::CrossFade(const CrossFade& other)
    :   VideoTransitionOpacity(other)
{
    VAR_DEBUG(*this);
}

CrossFade* CrossFade::clone() const
{
    return new CrossFade(static_cast<const CrossFade&>(*this));
}

CrossFade::~CrossFade()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

wxString CrossFade::getDescription(TransitionType type) const
{
    return _("Video crossfade");
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

void CrossFade::handleFullyOpaqueImage(const wxImagePtr& image, const std::function<float (int, int)>& f) const
{
    applyToFirstLineThenCopy(image,f);
}

void CrossFade::handleImageWithAlpha(const wxImagePtr& image, const std::function<float (int, int)>& f) const
{
    applyToAllPixels(image,f);
}

std::function<float (int,int)> CrossFade::getLeftMethod(const wxImagePtr& image, const float& factor) const
{
    std::function<float (int,int)> f = [factor](int x, int y) -> float
    {
        return 1.0 - factor;
    };
    return f;
}

std::function<float (int,int)> CrossFade::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    std::function<float (int,int)> f =[factor](int x, int y) -> float
    {
        return factor;
    };
    return f;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const CrossFade& obj)
{
    os << static_cast<const VideoTransition&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void CrossFade::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void CrossFade::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void CrossFade::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::CrossFade)
