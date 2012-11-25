#ifndef MODEL_RENDER_CODEC_PARAMETER_H
#define MODEL_RENDER_CODEC_PARAMETER_H

#include "UtilCloneable.h"
#include "UtilEnumSelector.h"
#include "ICodecParameter.h"

struct AVCodecContext;

namespace model { namespace render {

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
        ,   mDefault(0)
        ,   mMinimum(std::numeric_limits<int>::max())
        ,   mMaximum(std::numeric_limits<int>::min())
        ,   mValue(0)
        ,   mWindow(0)
    {
    }

    CodecParameter(const CodecParameter& other)
        :   mId(other.mId)
        ,   mEnabled(other.mEnabled)
        ,   mDefault(other.mDefault)
        ,   mMinimum(other.mMinimum)
        ,   mMaximum(other.mMaximum)
        ,   mValue(other.mValue)
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
    // COMPARISON
    //////////////////////////////////////////////////////////////////////////

    bool equals(const ICodecParameter& other) override
    {
        const CodecParameter* pOther = dynamic_cast<const CodecParameter*>(&other);
        ASSERT(pOther);
        return
            (mId == pOther->getId()) &&
            (mDefault == pOther->mDefault) &&
            (mMinimum == pOther->mMinimum) &&
            (mMaximum == pOther->mMaximum) &&
            (mValue == pOther->mValue);
    }

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
        setValue(value);
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

    inline bool getEnabled() const
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
        ASSERT_LESS_THAN_EQUALS(mValue,mMaximum);
        ASSERT_MORE_THAN_EQUALS(mValue,mMinimum);
    }

    TYPE getValue() const
    {
        return mValue;
    }

    wxString getName() const override
    {
        return getHumanReadibleName(mId);
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
        boost::serialization::void_cast_register<MOSTDERIVED, ICodecParameter>(
            static_cast<MOSTDERIVED *>(0),
            static_cast<ICodecParameter *>(0)
            );

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

    wxWindow* makeWidget(wxWindow *parent, ICodecParameterChangeListener* listener) override
    {
        mChangeListener = listener;
        wxSpinCtrl* spin = new wxSpinCtrl(parent);
        spin->SetRange(getMinimum(),getMaximum());
        spin->SetValue(getValue());
        mWindow = spin;
        mWindow->Enable(getEnabled());
        spin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &CodecParameterInt::onSpinChanged, this);
        return mWindow;
    }
    void destroyWidget() override
    {
        mChangeListener = 0;
        ASSERT(mWindow);
        wxSpinCtrl* spin = static_cast<wxSpinCtrl*>(mWindow);
        spin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &CodecParameterInt::onSpinChanged, this);
        mWindow->Destroy();
        mWindow = 0;
    }

    void onSpinChanged(wxSpinEvent& event)
    {
        wxSpinCtrl* spin = static_cast<wxSpinCtrl*>(mWindow);
        int value = spin->GetValue(); // Do not use event.GetPosition(), gives strange results
        setValue(value);
        mChangeListener->onParameterChange();
        event.Skip();
    }

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    ICodecParameterChangeListener* mChangeListener;
};

typedef boost::bimap<int,wxString> MappingType;
template <typename MOSTDERIVED, typename IDTYPE, IDTYPE ID, MappingType& NAMEMAPPING >
struct CodecParameterEnum
    :   public CodecParameter<MOSTDERIVED,IDTYPE,int>
{
    CodecParameterEnum()
        :   CodecParameter(ID)
    {
        BOOST_FOREACH( MappingType::left_reference item, NAMEMAPPING.left )
        {
            if (item.first < getMinimum()) { setMinimum(item.first); }
            if (item.first > getMaximum()) { setMaximum(item.first); }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // ICodecParameter
    //////////////////////////////////////////////////////////////////////////

    wxWindow* makeWidget(wxWindow *parent, ICodecParameterChangeListener* listener) override
    {
        mChangeListener = listener;
        EnumSelector<int>* selector = new EnumSelector<int>(parent, NAMEMAPPING, getDefault());
        selector->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &CodecParameterEnum::onChoiceChanged, this);
        mWindow = selector;
        mWindow->Enable(getEnabled());
        return mWindow;
    }
    void destroyWidget() override
    {
        mChangeListener = 0;
        EnumSelector<int>* selector = static_cast<EnumSelector<int>*>(mWindow);
        selector->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &CodecParameterEnum::onChoiceChanged, this);
        mWindow->Destroy();
        mWindow = 0;
    }

    void onChoiceChanged(wxCommandEvent& event)
    {
        EnumSelector<int>* selector = static_cast< EnumSelector<int>* >(mWindow);
        setValue(selector->getValue());
        mChangeListener->onParameterChange();
        event.Skip();
    }

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    ICodecParameterChangeListener* mChangeListener;
};

}} // namespace

#endif // MODEL_RENDER_CODEC_PARAMETER_H