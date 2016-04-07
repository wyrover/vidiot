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

#include "Convert.h"
#include "Properties.h"
#include "StatusBar.h"
#include "TransitionParameterBool.h"
#include "TransitionParameterDouble.h"
#include "TransitionParameterFilename.h"
#include "TransitionParameterInt.h"
#include "UtilPath.h"

namespace model { namespace video { namespace transition {

// static
wxString ImageGradient::getDefaultImageGradientPath()
{
    return util::path::getCombinedPath(util::path::getResourcesPath(), { "images","imagegradient" }).GetFullPath();
}

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

ParameterAttributes ImageGradient::getAvailableParameters() const
{
    return
    {
        { TransitionParameterFilename::sParameterImageFilename, _("Image"), 
        _("Select an image to be used for the effect. "
            "The colours in that image indicate in what way the transition will make the second video clip visible. "
            "The lightest regions are shown first. "
            "Then, gradually, the darker regions are shown until finally the darkest area is shown. "
            "Transparent areas in the image will cause the video of the right clip not to be shown at all in those areas. "
            "In GIMP, use the Blend tool, or a Paint Brush with 'Dynamics: Color from gradient' to create such images. "
            "Note that the darkest/lightest colours are determined from the image. "
            "Therefore, the gradient does not have to end exactly at white or black."), boost::make_shared<TransitionParameterFilename>("", false, true, getDefaultImageGradientPath()) },
        { TransitionParameterInt::sParameterSoften, _("Soften"), _("Select how much to soften the effect.") , boost::make_shared<TransitionParameterInt>(0, 0, 100) },
        { TransitionParameterInt::sParameterBlur, _("Blur"), _("Select how much to blur the image.") , boost::make_shared<TransitionParameterInt>(0, 0, 100) },
    };
}

wxString ImageGradient::getDescription(TransitionType type) const      
{
    return _("Image gradient");
}

void ImageGradient::onParameterChanged(const wxString& name)
{
    if ((name == TransitionParameterFilename::sParameterImageFilename) ||
        (name == TransitionParameterInt::sParameterBlur))
    {
        mImage = nullptr; // Use new image
    }
}
//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

std::function<float (int,int)> ImageGradient::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    wxFileName filename{ getParameter<TransitionParameterFilename>(TransitionParameterFilename::sParameterImageFilename)->getValue() };
    int soften{ getParameter<TransitionParameterInt>(TransitionParameterInt::sParameterSoften)->getValue() };
    ASSERT_MORE_THAN_EQUALS_ZERO(soften);
    int blur{ getParameter<TransitionParameterInt>(TransitionParameterInt::sParameterBlur)->getValue() };
    ASSERT_MORE_THAN_EQUALS_ZERO(blur);

    int darkest{ 255 * 3 };
    int lightest{ 0 };

    if (mImage == nullptr ||         // No cached image yet
        mImageFileName != filename)  // Cached image was for another file
    {
        mImageFileName = filename;
        mImage = boost::make_shared<wxImage>(filename.GetFullPath());

        if (mImage && mImage->IsOk())
        {
            // Apply blur to image
            if (blur != 0)
            {
                mImage = boost::make_shared<wxImage>(mImage->Blur(blur).Copy());
            }

            if (mImage && mImage->IsOk())
            {
                // Rescale the image to the required output size
                wxSize size{ Properties::get().getVideoSize() };
                if (mImage->GetSize() != size)
                {
                    mImage->Rescale(size.x, size.y);
                }

                if (mImage && mImage->IsOk())
                {
                    // Determine the darkest and lightest points of the image.
                    // These will be used as reference (starting and ending) points.
                    for (int x{ 0 }; x < mImage->GetWidth(); ++x)
                    {
                        for (int y{ 0 }; y < mImage->GetHeight(); ++y)
                        {
                            int lightness{ mImage->GetRed(x, y) + mImage->GetGreen(x, y) + mImage->GetBlue(x, y) };
                            if (!mImage->HasAlpha() ||
                                mImage->GetAlpha(x, y) > 0)
                            {
                                darkest = std::min(darkest, lightness);
                                lightest = std::max(lightest, lightness);
                            }
                        }
                    }
                }
            }
        }
    }

    mDarkest = static_cast<float>(darkest);
    mLightest = static_cast<float>(lightest);

    if (mImage && mImage->IsOk())
    {
        // Ensure proper WYSIWYG, scale the image extra in case the preview size differs from the output size of the project.
        wxSize outputSize{ Properties::get().getVideoSize() };

        // Center
        float zoom { static_cast<float>(image->GetWidth()) / static_cast<float>(outputSize.GetWidth()) };
        int patternW{ mImage->GetWidth() };
        int patternH{ mImage->GetHeight() };
        int patternZoomedW{ static_cast<int>(std::floor(static_cast<double>(patternW) * zoom)) };
        int patternZoomedH{ static_cast<int>(std::floor(static_cast<double>(patternH) * zoom)) };
        int patternZoomedOffsetX{ (image->GetWidth() - patternZoomedW) / 2 };
        int patternZoomedOffsetY{ (image->GetHeight() - patternZoomedH) / 2 };

        float softenFactor{ static_cast<float>(soften) / 100.0f };

        // Adjust such that the softness is shown both in the begin and the end AND 
        // the entire transition fills the image complete at both the begin and the end
        // Basically, this stretches the factor such that there is extra room for the 
        // softness at both the beginning and the end.
        //
        // In case soften == 0, this equals factor.
        float stretchedFactor{ factor - (softenFactor * (1 - factor)) };

        return [this, patternW, patternH, softenFactor, stretchedFactor, patternZoomedOffsetX, patternZoomedOffsetY, zoom](int x, int y) -> float
        {
            // Zoom
            int patternX{ static_cast<int>(std::floor(static_cast<float>(x - patternZoomedOffsetX) / zoom)) };
            int patternY{ static_cast<int>(std::floor(static_cast<float>(y - patternZoomedOffsetY) / zoom)) };
            float result{ 0.0 };
            if (patternX >= 0 &&
                patternX < patternW &&
                patternY >= 0 &&
                patternY < patternH)
            {
                int lightness{ mImage->GetRed(patternX, patternY) + mImage->GetGreen(patternX, patternY) + mImage->GetBlue(patternX, patternY) };
                float f{ (static_cast<float>(lightness) - mDarkest) / (mLightest - mDarkest) };
                float a{ mImage->HasAlpha() ? static_cast<float>(mImage->GetAlpha(patternX, patternY)) / 255.0f : 1.0f };
                if (softenFactor > 0.0f)
                {
                    result = f <= stretchedFactor ? a : f - softenFactor <= stretchedFactor ? (stretchedFactor - (f - softenFactor)) / softenFactor * a : 0.0f;
                }
                else
                {
                    result = f <= stretchedFactor ? a : 0.0f;
                }
            }
            return result;
        };
    }

    if (!mErrorShown)
    {
        mErrorShown = true;
        VAR_WARNING(filename);
        wxString error =
            filename.IsOk() ?
            // TRANSLATORS: This is shown when the Wipe Image/Image Gradient transition at time %2$s used a file (named %1$s) which the application was unable to read.
            wxString::Format(_("Couldn't read %1$s at %2$s."), filename.GetFullName(), Convert::ptsToHumanReadibleString(getLeftPts())) :
            // TRANSLATORS: This is shown when for the Wipe Image/Image Gradient transition at time %s no image has been selected yet.
            wxString::Format(_("No image selected at %s."), Convert::ptsToHumanReadibleString(getLeftPts()));
        gui::StatusBar::get().timedInfoText(error, 10000);
    }
    return [](int x, int y) { return static_cast<float>(0.0); };
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
