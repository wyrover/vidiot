//// Copyright 2015 Eric Raijmakers.
////
//// This file is part of Vidiot.
////
//// Vidiot is free software: you can redistribute it and/or modify
//// it under the terms of the GNU General Public License as published by
//// the Free Software Foundation, either version 3 of the License, or
//// (at your option) any later version.
////
//// Vidiot is distributed in the hope that it will be useful,
//// but WITHOUT ANY WARRANTY; without even the implied warranty of
//// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//// GNU General Public License for more details.
////
//// You should have received a copy of the GNU General Public License
//// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.
//
//#include "SolidColorClip.h"
//
//#include "Config.h"
//#include "UtilLog.h"
//#include "UtilLogWxwidgets.h"
//#include "VideoCompositionParameters.h"
//
//namespace model {
//
////////////////////////////////////////////////////////////////////////////
//// INITIALIZATION
////////////////////////////////////////////////////////////////////////////
//
//SolidColorClip::SolidColorClip()
//    : Clip()
//{
//    VAR_DEBUG(*this);
//}
//
//SolidColorClip::SolidColorClip(const VideoFilePtr& file)
//    : VideoClip(file)
//{
//    VAR_DEBUG(*this);
//    ASSERT(file->isA<SolidColorFile>())(file);
//    ASSERT_DIFFERS(file->getLength(),1);
//
//    pts length = getLength();
//    pts half = length / 2;
//    pts remainingLength = Config::ReadLong(Config::sPathTimelineDefaultStillImageLength);
//
//    // Move right edge to the left such that the clip can be extended if required
//    adjustEnd( - half );
//
//    // Move left edge to the right sich that the clip can be extended if required. The -remainingLength ensures that the resulting clip has the correct resulting size
//    adjustBegin(getLength() - remainingLength); // do not replace 'getLength()' with 'half' here (try doing the computation with 'length' an odd number...)
//
//    ASSERT_EQUALS(getLength(),remainingLength);
//}
//
//
//SolidColorClip::SolidColorClip(const wxColour& color)
//    : VideoClip()
//    , mColor(color)
//    , mLength{ Config::ReadLong(Config::sPathTimelineDefaultStillImageLength) }
//    , mNewStartPosition{ boost::none }
//    , mOutputFrame{}
//{
//    VAR_DEBUG(*this);
//}
//
//SolidColorClip::SolidColorClip(const SolidColorClip& other)
//    : Clip(other)
//    , mColor{ other.mColor }
//    , mLength{ other.mLength }
//    , mNewStartPosition{ boost::none }
//    , mOutputFrame{}
//{
//    VAR_DEBUG(*this);
//}
//
//SolidColorClip* SolidColorClip::clone() const
//{
//    return new SolidColorClip(static_cast<const SolidColorClip&>(*this));
//}
//
//SolidColorClip::~SolidColorClip()
//{
//    VAR_DEBUG(this);
//}
//
//    //////////////////////////////////////////////////////////////////////////
//    // ICONTROL
//    //////////////////////////////////////////////////////////////////////////
//
//pts SolidColorClip::getLength() const
//{
//    return mLength;
//}
//
//void SolidColorClip::moveTo(pts position)
//{
//    mNewStartPosition.reset(position);
//}
//wxString SolidColorClip::getDescription() const
//{
//    return _("Solid ") + wxString::Format("(%d,%d,%d)", mColor.Red(), mColor.Green(), mColor.Blue());
//}
//
//void SolidColorClip::clean()
//{
//    
//}
//
//    //////////////////////////////////////////////////////////////////////////
//    // ICLIP
//    //////////////////////////////////////////////////////////////////////////
//
//    pts SolidColorClip::getMinAdjustBegin() const
//    {
//        return (std::numeric_limits<pts>::max() - mLength) / 2;
//    }
//
//    pts SolidColorClip::getMaxAdjustBegin() const
//    {
//        return (std::numeric_limits<pts>::max() - mLength) / 2;
//    }
//
//    void SolidColorClip::adjustBegin(pts adjustment) override;
//
//    pts SolidColorClip::getMinAdjustEnd() const override;
//    pts SolidColorClip::getMaxAdjustEnd() const override;
//    void SolidColorClip::adjustEnd(pts adjustment) override;
//
//    //////////////////////////////////////////////////////////////////////////
//    // IVIDEO
//    //////////////////////////////////////////////////////////////////////////
//
//    virtual VideoFramePtr getNextVideo(const VideoCompositionParameters& parameters) override;
//
//
//
////////////////////////////////////////////////////////////////////////////
//// LOGGING
////////////////////////////////////////////////////////////////////////////
//
//std::ostream& operator<<(std::ostream& os, const SolidColorClip& obj)
//{
//    os << static_cast<const Clip&>(obj) << '|' << obj.mLength << '|' << obj.mColor;
//    return os;
//}
//
////////////////////////////////////////////////////////////////////////////
//// SERIALIZATION
////////////////////////////////////////////////////////////////////////////
//
//template<class Archive>
//void SolidColorClip::serialize(Archive & ar, const unsigned int version)
//{
//    try
//    {
//        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Clip);
//        ar & BOOST_SERIALIZATION_NVP(mColor);
//        ar & BOOST_SERIALIZATION_NVP(mLength);
//    }
//    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
//    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
//    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
//    catch (...)                                  { LOG_ERROR;                                   throw; }
//}
//template void SolidColorClip::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
//template void SolidColorClip::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);
//
//}
//
//BOOST_CLASS_EXPORT_IMPLEMENT(model::SolidColorClip)