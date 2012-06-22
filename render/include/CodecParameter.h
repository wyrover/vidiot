#ifndef MODEL_RENDER_CODEC_PARAMETER_H
#define MODEL_RENDER_CODEC_PARAMETER_H

#include <wx/spinctrl.h>
#include <ostream>
#include <boost/serialization/access.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/serialization/version.hpp>
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

template <class MOSTDERIVED, class IDTYPE, class TYPE>
class CodecParameter
    :   public ICodecParameter
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit CodecParameter(IDTYPE id)
        :   mId(id)
        ,   mEnabled(false)
        ,   mDefault()
        ,   mMinimum()
        ,   mMaximum()
        ,   mWindow(0)    {
    }

    CodecParameter(const CodecParameter& other)
        :   mId(other.mId)
        ,   mEnabled(other.mEnabled)
        ,   mDefault(other.mDefault)
        ,   mMinimum(other.mMinimum)
        ,   mMaximum(other.mMaximum)
        ,   mWindow(0)
    {
    }

	virtual ~CodecParameter()
    {
    }

    virtual CodecParameter* clone() const
    {
        return new MOSTDERIVED(static_cast<const MOSTDERIVED&>(*this));
    };

    //////////////////////////////////////////////////////////////////////////
    // BIT RATE
    //////////////////////////////////////////////////////////////////////////

    inline CodecParameter& enable()
    {
        mEnabled = true;
        return *this;
    }

    inline CodecParameter& setDefault(TYPE value)
    {
        mDefault = value;
        mValue = value;
        return *this;
    }

    inline CodecParameter& setMinimum(TYPE value)
    {
        mMinimum = value;
        return *this;
    }

    inline CodecParameter& setMaximum(TYPE value)
    {
        mMaximum = value;
        return *this;
    }

    inline IDTYPE getId() const
    {
        return mId;
    }

    bool enabled()
    {
        return mEnabled;
    }

    TYPE getDefault() const
    {
        return mDefault;
    }

    TYPE getMinimum() const
    {
        return mMinimum;
    }

    TYPE getMaximum() const
    {
        return mMaximum;
    }

    void setValue(TYPE value)
    {
        mValue = value;
    }

    TYPE getValue() const
    {
        return mValue;
    }

    wxString getName() const override
    {
        return getHumandReadibleName(mId);
    }

protected:

    wxWindow* mWindow;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    IDTYPE mId;
    bool mEnabled;
    TYPE  mDefault;
    TYPE  mMinimum;
    TYPE  mMaximum;
    TYPE  mValue;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    void log( std::ostream& os ) const override
    {
        os << *this;
    }

    friend std::ostream& operator<<( std::ostream& os, const CodecParameter& obj )
    {
        os << &obj << '|' << obj.mId << '|' << obj.mEnabled << '|' << obj.mDefault << '|' << obj.mMinimum << '|' << obj.mMaximum << '|' << obj.mValue;
        return os;
    }

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & mId;
        ar & mEnabled;
        ar & mDefault;
        ar & mMinimum;
        ar & mMaximum;
        ar & mValue;
    }

};

template <typename MOSTDERIVED, typename IDTYPE, IDTYPE ID>
struct CodecParameterInt
    :   public CodecParameter<MOSTDERIVED,IDTYPE,int>
{
    CodecParameterInt()
        :   CodecParameter(ID)
    {
    }

    //////////////////////////////////////////////////////////////////////////
    // ICodecParameter
    //////////////////////////////////////////////////////////////////////////

    wxWindow* makeWidget(wxWindow *parent) override
    {
        wxSpinCtrl* spin = new wxSpinCtrl(parent);
        spin->SetRange(getMinimum(),getMaximum());
        spin->SetValue(getValue());
        mWindow = spin;
        spin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &CodecParameterInt::onSpinChanged, this);
        return mWindow;
    }
    void destroyWidget() override
    {
        wxSpinCtrl* spin = static_cast<wxSpinCtrl*>(mWindow);
        spin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &CodecParameterInt::onSpinChanged, this);
    }

    void onSpinChanged(wxSpinEvent& event)
    {
        wxSpinCtrl* spin = static_cast<wxSpinCtrl*>(mWindow);
        setValue(spin->GetValue());
        event.Skip();
    }
};

}} // namespace

#endif // MODEL_RENDER_CODEC_PARAMETER_H