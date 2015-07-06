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

#include "VideoTransition_FadeToColor.h"

#include "TransitionFactory.h"
#include "TransitionParameterColor.h"
#include "UtilEnum.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoComposition.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model { namespace video { namespace transition {

DECLAREENUM(FadeToColorParameters, \
    FadeToColorColor);
IMPLEMENTENUM(FadeToColorParameters);
std::map<FadeToColorParameters, wxString> FadeToColorParametersConverter::mapToHumanReadibleString = {
    { FadeToColorColor, _("Color") },
};



//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FadeToColor::FadeToColor()
    : VideoTransitionOpacity()
{
    VAR_DEBUG(this);
    addParameter(FadeToColorColor, boost::make_shared<TransitionParameterColor>(wxColour{ 255, 255, 255 }));// todo in config a default
}

FadeToColor::FadeToColor(const FadeToColor& other)
    : VideoTransitionOpacity(other)
{
    VAR_DEBUG(*this);
}

FadeToColor* FadeToColor::clone() const
{
    return new FadeToColor(static_cast<const FadeToColor&>(*this));
}

FadeToColor::~FadeToColor()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

model::IClipPtr FadeToColor::makeLeftClip()
{
    if (getLeft())
    {
        return Transition::makeLeftClip();
    }
    wxColour color{ boost::dynamic_pointer_cast<TransitionParameterColor>(getParameter(FadeToColorColor))->getColor() };
    model::IClipPtr result = boost::make_shared<model::ImageClip>(boost::make_shared<model::ImageFile>(color));
    result->adjustBegin(result->getLength());
    result->adjustEnd(getLength());
    return result;
}

model::IClipPtr FadeToColor::makeRightClip()
{
    if (getRight())
    {
        return Transition::makeRightClip();
    }
    wxColour color{ boost::dynamic_pointer_cast<TransitionParameterColor>(getParameter(FadeToColorColor))->getColor() };
    model::IClipPtr result = boost::make_shared<model::ImageClip>(boost::make_shared<model::ImageFile>(color));
    result->adjustEnd(-result->getLength());
    result->adjustBegin(-getLength());
    return result;
}

bool FadeToColor::supports(TransitionType type) const
{
    return 
        type == TransitionTypeFadeIn || 
        type == TransitionTypeFadeOut;
}

wxString FadeToColor::getDescription(TransitionType type) const// todo make dir modelvideotransition
{
    if (type == TransitionTypeFadeIn)
    {
        return _("Fade from color");
    }
    else
    {
        return _("Fade to color");
    }
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

void FadeToColor::handleFullyOpaqueImage(const wxImagePtr& image, const boost::function<float (int, int)>& f) const
{
    applyToFirstLineThenCopy(image,f);
}

void FadeToColor::handleImageWithAlpha(const wxImagePtr& image, const boost::function<float (int, int)>& f) const
{
    applyToAllPixels(image,f);
}

boost::function<float (int,int)> FadeToColor::getLeftMethod(const wxImagePtr& image, const float& factor) const
{
    boost::function<float (int,int)> f = [factor](int x, int y) -> float // todo use std:function iso boost
    {
        return 1.0 - factor;
    };
    return f;
}

boost::function<float (int,int)> FadeToColor::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    boost::function<float (int,int)> f =[factor](int x, int y) -> float
    {
        return factor;
    };
    return f;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const FadeToColor& obj)
{
    os << static_cast<const VideoTransition&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void FadeToColor::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void FadeToColor::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void FadeToColor::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::FadeToColor)
