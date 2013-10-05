// Copyright 2013 Eric Raijmakers.
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

#ifndef MODEL_RENDER_I_CODEC_PARAMETER_H
#define MODEL_RENDER_I_CODEC_PARAMETER_H

struct AVCodecContext;

namespace model { namespace render {

struct ICodecParameterChangeListener
{
    virtual void onParameterChange() = 0;
};

struct ICodecParameter
{
    ICodecParameter() {};
    virtual ~ICodecParameter() {};
    virtual ICodecParameter* clone() const = 0;
    virtual wxString getName() const = 0;
    virtual wxWindow* makeWidget(wxWindow* parent, ICodecParameterChangeListener* listener) = 0;
    virtual void destroyWidget() = 0;
    virtual void set(AVCodecContext* codec) = 0;
    virtual bool equals(const ICodecParameter& other) = 0;
    virtual void log( std::ostream& os ) const = 0;

    bool operator==(const ICodecParameter& other)
    {
        return equals(other);
    }

    friend std::ostream& operator<<( std::ostream& os, const ICodecParameter& obj )
    {
        obj.log(os);
        return os;
    }
};

}} // namespace

BOOST_CLASS_VERSION(model::render::ICodecParameter, 1)
BOOST_CLASS_EXPORT_KEY(model::render::ICodecParameter)

#endif // MODEL_RENDER_I_CODEC_PARAMETER_H