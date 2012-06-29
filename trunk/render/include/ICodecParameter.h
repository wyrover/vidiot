#ifndef MODEL_RENDER_I_CODEC_PARAMETER_H
#define MODEL_RENDER_I_CODEC_PARAMETER_H

#include <ostream>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <wx/window.h>
#include "UtilCloneable.h"

struct AVCodecContext;

namespace model { namespace render {

struct ICodecParameter;
typedef boost::shared_ptr<ICodecParameter> ICodecParameterPtr;

struct ICodecParameter
    :   public ICloneable
{
    virtual ICodecParameter* clone() const = 0;
    virtual wxString getName() const = 0;
    virtual wxWindow* makeWidget(wxWindow* parent) = 0;
    virtual void destroyWidget() = 0;
    virtual void set(AVCodecContext* codec) = 0;
    virtual void log( std::ostream& os ) const = 0;

    friend std::ostream& operator<<( std::ostream& os, const ICodecParameter& obj )
    {
        obj.log(os);
        return os;
    }
};

}} // namespace

#endif // MODEL_RENDER_I_CODEC_PARAMETER_H