#ifndef MODEL_RENDER_I_CODEC_PARAMETER_H
#define MODEL_RENDER_I_CODEC_PARAMETER_H

#include "UtilCloneable.h"

struct AVCodecContext;

namespace model { namespace render {

struct ICodecParameter
    :   public ICloneable
{
    ICodecParameter() {};
    virtual ~ICodecParameter() {};
    virtual ICodecParameter* clone() const = 0;
    virtual wxString getName() const = 0;
    virtual wxWindow* makeWidget(wxWindow* parent) = 0;
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

    //todo
    ////////////////////////////////////////////////////////////////////////////
    //// SERIALIZATION
    ////////////////////////////////////////////////////////////////////////////

    //friend class boost::serialization::access;
    //template<class Archive>
    //void serialize(Archive & ar, const unsigned int version)
    //{
    //}
};

}} // namespace

BOOST_CLASS_VERSION(model::render::ICodecParameter, 1)

#endif // MODEL_RENDER_I_CODEC_PARAMETER_H