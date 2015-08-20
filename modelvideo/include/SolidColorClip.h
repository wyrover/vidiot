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
//#ifndef MODEL_SOLIDCOLOR_CLIP_H
//#define MODEL_SOLIDCOLOR_CLIP_H
//
//#include "VideoClip.h"
//
//namespace model {
//
//class SolidColorClip
//    : public VideoClip
//{
//public:
//
//    //////////////////////////////////////////////////////////////////////////
//    // INITIALIZATION
//    //////////////////////////////////////////////////////////////////////////
//
//    SolidColorClip();
//    SolidColorClip(const VideoFilePtr& clip);
//    virtual ~SolidColorClip();
//
//private:
//
//    //////////////////////////////////////////////////////////////////////////
//    // LOGGING
//    //////////////////////////////////////////////////////////////////////////
//
//    friend std::ostream& operator<<(std::ostream& os, const SolidColorClip& obj);
//
//    //////////////////////////////////////////////////////////////////////////
//    // SERIALIZATION
//    //////////////////////////////////////////////////////////////////////////
//
//    friend class boost::serialization::access;
//    template<class Archive>
//    void serialize(Archive & ar, const unsigned int version);
//};
//
//} // namespace
//
//BOOST_CLASS_VERSION(model::SolidColorClip, 1)
//BOOST_CLASS_EXPORT_KEY(model::SolidColorClip)
//
//#endif