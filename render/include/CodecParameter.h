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

#pragma once

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

    explicit CodecParameter(const IDTYPE& id)
        :   mId(id)
        ,   mDefault(0)
        ,   mMinimum(std::numeric_limits<int>::max())
        ,   mMaximum(std::numeric_limits<int>::min())
        ,   mValue(0)
    {
    }

    CodecParameter(const CodecParameter& other)
        :   mId(other.mId)
        ,   mDefault(other.mDefault)
        ,   mMinimum(other.mMinimum)
        ,   mMaximum(other.mMaximum)
        ,   mValue(other.mValue)
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

    inline CodecParameter& setDefault(const TYPE& value)
    {
        mDefault = value;
        setValue(value);
        return *this;
    }

    inline CodecParameter& setMinimum(const TYPE& value)
    {
        mMinimum = value;
        return *this;
    }

    inline CodecParameter& setMaximum(const TYPE& value)
    {
        mMaximum = value;
        return *this;
    }

    inline IDTYPE getId() const
    {
        return mId;
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

    void setValue(const TYPE& value)
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

    int getIdAsInt() const override
    {
        return mId;
    }

protected:

    wxWindow* mWindow = nullptr;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    IDTYPE mId;
    TYPE  mDefault;
    TYPE  mMinimum;
    TYPE  mMaximum;
    TYPE  mValue;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    void log(std::ostream& os) const override
    {
        os << *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const CodecParameter& obj)
    {
        os << &obj << '|' << obj.mId << '|' << obj.mDefault << '|' << obj.mMinimum << '|' << obj.mMaximum << '|' << obj.mValue;
        return os;
    }

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        try
        {
            boost::serialization::void_cast_register<MOSTDERIVED, ICodecParameter>(
                static_cast<MOSTDERIVED *>(0),
                static_cast<ICodecParameter *>(0)
                );

            ar & BOOST_SERIALIZATION_NVP(mId);
            ar & BOOST_SERIALIZATION_NVP(mDefault);
            ar & BOOST_SERIALIZATION_NVP(mMinimum);
            ar & BOOST_SERIALIZATION_NVP(mMaximum);
            ar & BOOST_SERIALIZATION_NVP(mValue);
        }
        catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
        catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
        catch (...)                                  { LOG_ERROR;                                   throw; }
    }

};

template <typename MOSTDERIVED, typename IDTYPE, IDTYPE ID>
struct CodecParameterInt
    :   public CodecParameter<MOSTDERIVED,IDTYPE,int>
{
    CodecParameterInt()
        :   CodecParameter<MOSTDERIVED,IDTYPE,int>(ID)
    {
    }

    //////////////////////////////////////////////////////////////////////////
    // ICodecParameter
    //////////////////////////////////////////////////////////////////////////

    wxWindow* makeWidget(wxWindow *parent, ICodecParameterChangeListener* listener) override
    {
        mChangeListener = listener;
        wxSpinCtrl* spin = new wxSpinCtrl(parent);
        spin->SetRange(this->getMinimum(),this->getMaximum());
        spin->SetValue(this->getValue());
        this->mWindow = spin;
        this->mWindow->Enable(true);
        spin->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &CodecParameterInt::onSpinChanged, this);
        return this->mWindow;
    }
    void destroyWidget() override
    {
        mChangeListener = 0;
        ASSERT(this->mWindow);
        wxSpinCtrl* spin = static_cast<wxSpinCtrl*>(this->mWindow);
        spin->Unbind(wxEVT_COMMAND_SPINCTRL_UPDATED, &CodecParameterInt::onSpinChanged, this);
        this->mWindow->Destroy();
        this->mWindow = 0;
    }

    void onSpinChanged(wxSpinEvent& event)
    {
        wxSpinCtrl* spin = static_cast<wxSpinCtrl*>(this->mWindow);
        int value = spin->GetValue(); // Do not use event.GetPosition(), gives strange results
        this->setValue(value);
        this->mChangeListener->onParameterChange();
        event.Skip();
    }

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    ICodecParameterChangeListener* mChangeListener;
};

typedef std::map<int,wxString> MappingType;
template <typename MOSTDERIVED, typename IDTYPE, IDTYPE ID, MappingType& NAMEMAPPING >
struct CodecParameterEnum
    :   public CodecParameter<MOSTDERIVED,IDTYPE,int>
{
    CodecParameterEnum()
        :   CodecParameter<MOSTDERIVED,IDTYPE,int>(ID)
    {
        for ( auto kvp : NAMEMAPPING )
        {
            if (kvp.first < this->getMinimum()) { this->setMinimum(kvp.first); }
            if (kvp.first > this->getMaximum()) { this->setMaximum(kvp.first); }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // ICodecParameter
    //////////////////////////////////////////////////////////////////////////

    wxWindow* makeWidget(wxWindow *parent, ICodecParameterChangeListener* listener) override
    {
        this->mChangeListener = listener;
        EnumSelector<int>* selector = new EnumSelector<int>(parent, NAMEMAPPING, this->getDefault());
        selector->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &CodecParameterEnum::onChoiceChanged, this);
        this->mWindow = selector;
        this->mWindow->Enable(true);
        return this->mWindow;
    }
    void destroyWidget() override
    {
        this->mChangeListener = 0;
        EnumSelector<int>* selector = static_cast<EnumSelector<int>*>(this->mWindow);
        selector->Unbind(wxEVT_COMMAND_CHOICE_SELECTED, &CodecParameterEnum::onChoiceChanged, this);
        this->mWindow->Destroy();
        this->mWindow = 0;
    }

    void onChoiceChanged(wxCommandEvent& event)
    {
        EnumSelector<int>* selector = static_cast< EnumSelector<int>* >(this->mWindow);
        this->setValue(selector->getValue());
        this->mChangeListener->onParameterChange();
        event.Skip();
    }

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    ICodecParameterChangeListener* mChangeListener;
};

}} // namespace
