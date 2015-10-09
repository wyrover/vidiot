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

#include "VideoTransition_Bands.h"

#include "TransitionFactory.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilVector.h"
#include "VideoClip.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Bands::Bands()
    :	VideoTransitionOpacity()
    ,   mBands(10)
{
    VAR_DEBUG(this);
}

Bands::Bands(const Bands& other)
    :   VideoTransitionOpacity(other)
    ,   mBands(other.mBands)
{
    VAR_DEBUG(*this);
}

Bands* Bands::clone() const
{
    return new Bands(static_cast<const Bands&>(*this));
}

Bands::~Bands()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

wxString Bands::getDescription(TransitionType type) const
{
    return _("Bands");
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

void Bands::handleFullyOpaqueImage(const wxImagePtr& image, const std::function<float (int, int)>& f) const
{
    applyToFirstLineThenCopy(image,f);
}

void Bands::handleImageWithAlpha(const wxImagePtr& image, const std::function<float (int, int)>& f) const
{
    applyToAllPixels(image,f);
}

std::function<float (int,int)> Bands::getLeftMethod(const wxImagePtr& image, const float& factor) const
{
    int bandSize = image->GetWidth() / mBands;
    std::function<float (int,int)> f = [bandSize,factor](int x, int y) -> float
    {
        if (x % bandSize >= factor * bandSize)
        {
            return 1.0;
        }
        return 0.0;
    };
    return f;
}

std::function<float (int,int)> Bands::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    int bandSize = image->GetWidth() / mBands;
    std::function<float (int,int)> f =[bandSize,factor](int x, int y) -> float
    {
        if (x % bandSize <= factor * bandSize)
        {
            return 1.0;
        }
        return 0.0;
    };
    return f;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Bands& obj)
{
    os << static_cast<const VideoTransition&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Bands::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Bands::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Bands::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::Bands)