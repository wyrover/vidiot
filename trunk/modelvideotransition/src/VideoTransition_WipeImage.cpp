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

#include "VideoTransition_WipeImage.h"

#include <wx/textfile.h>
#include "Convert.h"
#include "Properties.h"
#include "StatusBar.h"
#include "TransitionParameterBool.h"
#include "TransitionParameterDirection8.h"
#include "TransitionParameterDouble.h"
#include "TransitionParameterInt.h"
#include "TransitionParameterRotationDirection.h"
#include "UtilPath.h"

namespace model { namespace video { namespace transition {

// static 
wxString WipeImage::getDefaultZoomImagesPath()
{
    return util::path::getCombinedPath(util::path::getResourcesPath(), { "images","wipeimage" }).GetFullPath();
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

WipeImage::WipeImage(const wxString& file, const wxString& name)
    : VideoTransitionOpacity()
    , mName(name)
{
}

WipeImage* WipeImage::clone() const
{
    return new WipeImage(static_cast<const WipeImage&>(*this));
}

WipeImage::WipeImage(const WipeImage& other)
    : VideoTransitionOpacity(other)
    , mImage(nullptr) // Do not copy the cached image
{
}

//////////////////////////////////////////////////////////////////////////
// TRANSITION
//////////////////////////////////////////////////////////////////////////

void WipeImage::clean()
{
    VideoTransitionOpacity::clean();
    mImage = nullptr;
    mImageFileName.Clear();
}

bool WipeImage::supports(TransitionType type) const
{
    return
        type == TransitionTypeFadeInFromPrevious ||
        type == TransitionTypeFadeOutToNext;
}

std::vector<std::tuple<wxString, wxString, TransitionParameterPtr>> WipeImage::getAvailableParameters() const
{
    wxString descriptor{ _("Images") + " (" + File::sSupportedImageExtensions + ")|" + File::sSupportedImageExtensions + ";" + File::sSupportedImageExtensions.Upper() };
    return
    {
        std::make_tuple(TransitionParameterFilename::sParameterImageFilename, _("Image"), boost::make_shared<TransitionParameterFilename>(descriptor, true, false, getDefaultZoomImagesPath())),
        std::make_tuple(TransitionParameterDouble::sParameterScaling, _("Scaling"), boost::make_shared<TransitionParameterDouble>(1.0, 0.0, 10.0)),
        std::make_tuple(TransitionParameterInt::sParameterRotations, _("Rotations"), boost::make_shared<TransitionParameterInt>(0, 0, 100)),
        std::make_tuple(TransitionParameterRotationDirection::sParameterRotationDirection, _("Rotation direction"), boost::make_shared<TransitionParameterRotationDirection>(RotationDirectionClockWise)),
        std::make_tuple(TransitionParameterBool::sParameterInversed, _("Inversed"), boost::make_shared<TransitionParameterBool>(false)),
    };
}

wxString WipeImage::getDescription(TransitionType type) const
{
    return _("Wipe Image");
}

void WipeImage::onParameterChanged(const wxString& name)
{                              
    if (name == TransitionParameterFilename::sParameterImageFilename)
    {
        mImage = nullptr; // Use new image

        wxFileName filename{ getParameter<TransitionParameterFilename>(TransitionParameterFilename::sParameterImageFilename)->getValue() };
        filename.MakeRelativeTo(getDefaultZoomImagesPath());
        if (filename.GetDirCount() == 0)
        {
            // It's one of the default images. Find default parameters.
            wxFileName infofile{ getDefaultZoomImagesPath(), "info.txt" };

            wxTextFile file;
            bool found{ file.Open(infofile.GetFullPath()) };
            if (found)
            {
                wxString s;
                wxRegEx re{ wxT("([^ ]+) ([[:digit:]]+.[[:digit:]]+) ([[:digit:]]+)") };   // Factor for 1280x720 sized projects (which is the size used in test/input)
                for (s = file.GetFirstLine(); !file.Eof(); s = file.GetNextLine())
                {
                    if (re.Matches(s))
                    {
                        wxString file = re.GetMatch(s, 1);
                        if (filename.GetName().IsSameAs(file))
                        {
                            double scaling{ 1.0 };
                            if (re.GetMatch(s, 2).ToCDouble(&scaling))
                            {
                                wxSize projectSize{ Properties::get().getVideoSize() };
                                double projectFactor{ 1.0 * projectSize.x / 1280 };

                                boost::shared_ptr<TransitionParameterDouble> scalingParameter{ getParameter<TransitionParameterDouble>(TransitionParameterDouble::sParameterScaling) };
                                if (scaling >= scalingParameter->getMin() && scaling <= scalingParameter->getMax())
                                {
                                    scalingParameter->setValue(scaling * projectFactor);
                                }
                            }

                            long rotations;
                            if (re.GetMatch(s, 3).ToLong(&rotations))
                            {
                                boost::shared_ptr<TransitionParameterInt> rotationsParameter{ getParameter<TransitionParameterInt>(TransitionParameterInt::sParameterRotations) };
                                if (rotations >= rotationsParameter->getMin() && rotations <= rotationsParameter->getMax())
                                {
                                    rotationsParameter->setValue(rotations);
                                }
                            }

                            break;
                        }
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// VIDEOTRANSITIONOPACITY
//////////////////////////////////////////////////////////////////////////

std::function<float (int,int)> WipeImage::getRightMethod(const wxImagePtr& image, const float& factor) const
{
    wxFileName filename{ getParameter<TransitionParameterFilename>(TransitionParameterFilename::sParameterImageFilename)->getValue() };
    // Note: the scaling parameter gives the relative factor to apply the the 'pattern image' size, at the end of the (non inversed) transition.
    double scaling{ getParameter<TransitionParameterDouble>(TransitionParameterDouble::sParameterScaling)->getValue() };
    bool inversed{ getParameter<TransitionParameterBool>(TransitionParameterBool::sParameterInversed)->getValue() };

    if (mImage == nullptr ||         // No cached image yet
        mImageFileName != filename)  // Cached image was for another file
    {
        mImage = boost::make_shared<wxImage>(filename.GetFullPath());
        mImageFileName = filename;
    }
                            
    // todo add documentation on transitions   (part. about size of zoom images?)
    // todo hangup when transition playback is active (release mode without debugging) and selecting the imagezoom
                     
    if (mImage && mImage->IsOk())
    {
        if (!mImage->HasAlpha())    
        {       
            // Initialize the image from mask data, or initialize default alpha (can happen if images are manipulated after being added).
            mImage->InitAlpha();
        }
        ASSERT(mImage->HasAlpha())(filename);

        float directedFactor{ inversed ? 1.0f - factor : factor };

        // Ensure proper WYSIWYG, scale the image extra in case the preview size differs from the output size of the project.
        wxSize outputSize{ Properties::get().getVideoSize() }; // todo make properties more thread safe than now. This will often be done in secondary thread (rendering/playback!)
        float adjustedFactor{ directedFactor * static_cast<float>(image->GetWidth()) / static_cast<float>(outputSize.GetWidth()) };

        // Zoom
        double zoom{ inversed ? scaling - scaling * adjustedFactor : scaling * adjustedFactor };
        zoom = scaling * adjustedFactor;
        int patternW{ mImage->GetWidth() };
        int patternH{ mImage->GetHeight() };
        int patternZoomedW{ static_cast<int>(std::floor(static_cast<double>(patternW) * zoom)) };
        int patternZoomedH{ static_cast<int>(std::floor(static_cast<double>(patternH) * zoom)) };
        int patternZoomedOffsetX{ (image->GetWidth() - patternZoomedW) / 2 };
        int patternZoomedOffsetY{ (image->GetHeight() - patternZoomedH) / 2 };

        // Rotation
        int rotations{ getParameter<TransitionParameterInt>(TransitionParameterInt::sParameterRotations)->getValue() };
        RotationDirection rd{ getParameter<TransitionParameterRotationDirection>(TransitionParameterRotationDirection::sParameterRotationDirection)->getValue() };
        if (rd == RotationDirectionClockWise) { rotations = -1 * rotations; }
        double angle{ static_cast<double>(rotations) * factor * 360.0 };
        while (angle > 360.0) { angle -= 360.0; }
        double s{ std::sin(angle * M_PI / 180) };
        double c{ std::cos(angle * M_PI / 180) };
        int originX{ patternW / 2 };
        int originY{ patternH / 2 };

        return [=, this](int x, int y) -> float
        {
            // Zoom
            int patternX{ static_cast<int>(std::floor(static_cast<float>(x - patternZoomedOffsetX) / zoom)) };
            int patternY{ static_cast<int>(std::floor(static_cast<float>(y - patternZoomedOffsetY) / zoom)) };

            // Rotation
            patternX -= originX; // translate point back to origin
            patternY -= originY;
            double xnew{ patternX * c - patternY * s }; // rotate point
            double ynew{ patternX * s + patternY * c };
            patternX = xnew + originX; // translate point back
            patternY = ynew + originY;

            float result{ 0.0 };
            if (patternX >= 0 &&
                patternX < patternW &&
                patternY >= 0 &&
                patternY < patternH)
            {
                int alpha{ mImage->GetAlpha(patternX, patternY) };
                if (alpha > 0)
                {
                    result = static_cast<float>(255.0 / static_cast<float>(alpha));
                }
            }
            return inversed ? (1.0 - result) : result;
        };
    }

    // todo make mechanism to be able to set messages (dialog even?) from secondary thread? Optionally, abort rendering/playback? 
    // Maybe add exception with message for dialog and then catch in render loop (with yes/no to abort rendering?)
    // but then the exception must also be caught in playback threads, AND in main thread for all previews! (maybe ignore by default)
    // gui::StatusBar::get().timedInfoText(wxString::Format(_("Missing %s at %s."), filename.GetFullName(), Convert::ptsToHumanReadibleString(getLeftPts())), 10000);
    VAR_WARNING(filename);
    return [](int x, int y) { return static_cast<float>(0.0); };
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void WipeImage::serialize(Archive & ar, const unsigned int version)
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

BOOST_CLASS_EXPORT_IMPLEMENT(model::video::transition::WipeImage)
