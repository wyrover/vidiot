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

#include "VideoTransition_FadeToColor.h"

#include "ImageClip.h"
#include "ImageFile.h"
#include "TransitionParameterColor.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FadeToColor* FadeToColor::clone() const
{
    return new FadeToColor(static_cast<const FadeToColor&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool FadeToColor::supports(TransitionType type) const
{
    return
        type == TransitionTypeFadeIn ||
        type == TransitionTypeFadeOut;
}

std::vector<std::tuple<wxString, wxString, TransitionParameterPtr>> FadeToColor::getAvailableParameters() const
{
    return
    {
        std::make_tuple(TransitionParameterColor::sParameterColor, _("Color"), boost::make_shared<TransitionParameterColor>(wxColour{ 255, 255, 255 })),
    };
}


wxString FadeToColor::getDescription(TransitionType type) const
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
// CROSSFADE
//////////////////////////////////////////////////////////////////////////

model::IClipPtr FadeToColor::makeLeftClip()
{
    if (getLeft())
    {
        return Transition::makeLeftClip();
    }
    wxColour color{ getParameter<TransitionParameterColor>(TransitionParameterColor::sParameterColor)->getValue() };
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
    wxColour color{ getParameter<TransitionParameterColor>(TransitionParameterColor::sParameterColor)->getValue() };
    model::IClipPtr result = boost::make_shared<model::ImageClip>(boost::make_shared<model::ImageFile>(color));
    result->adjustEnd(-result->getLength());
    result->adjustBegin(-getLength());
    return result;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void FadeToColor::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CrossFade);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void FadeToColor::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void FadeToColor::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::FadeToColor)
