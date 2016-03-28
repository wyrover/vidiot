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

#include "VideoTransition_ImageGradient.h"

#include "TransitionParameterBool.h"
#include "TransitionParameterFilename.h"

namespace model { namespace video { namespace transition {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ImageGradient* ImageGradient::clone() const
{
    return new ImageGradient(static_cast<const ImageGradient&>(*this));
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

bool ImageGradient::supports(TransitionType type) const
{
    return
        type == TransitionTypeFadeInFromPrevious ||
        type == TransitionTypeFadeOutToNext;
}

ParameterAttributes ImageGradient::getAvailableParameters() const
{
    return
    {
        { TransitionParameterFilename::sParameterImageFilename, _("Image"), "", boost::make_shared<TransitionParameterFilename>("", false, true) },
        { TransitionParameterBool::sParameterInversed, _("Inversed"), "", boost::make_shared<TransitionParameterBool>(false) },
    };
}

wxString ImageGradient::getDescription(TransitionType type) const
{
    // TRANSLATORS: This string is not yet used.
    return _("Image gradient");
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

std::function<float (int,int)> ImageGradient::getRightMethod(const wxImagePtr& image, const float& factor) const
{
//    wxFileName filename{ getParameter<TransitionParameterFilename>(TransitionParameterFilename::sParameterImageFilename)->getValue() };
//    bool inversed{ getParameter<TransitionParameterBool>(TransitionParameterBool::sParameterInversed)->getValue() };

//    double w{ static_cast<double>(image->GetWidth()) };
//    double h{ static_cast<double>(image->GetHeight()) };
    FATAL("Wrong direction");
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void ImageGradient::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(VideoTransition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

}}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::ImageGradient)
