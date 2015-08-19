// Copyright 2013-2015 Eric Raijmakers.
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

#ifndef MODEL_RENDER_H
#define MODEL_RENDER_H

#include "UtilEvent.h"
#include "UtilInt.h"

namespace model { namespace render {

class Render
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public boost::enable_shared_from_this<Render>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// The initialization of all codec related administration is done via this class,
    /// to avoid exposing the VideoCodec/AudioCodec header files (and thus, the avcodec
    /// dependency) to other parts (particularly, to avoid integer types troubles).
    static void initialize();

    Render();

    Render(const Render& other);

    virtual Render* clone() const;

    virtual void onCloned();

    virtual ~Render();

    //////////////////////////////////////////////////////////////////////////
    // OPERATORS
    //////////////////////////////////////////////////////////////////////////

    bool operator== (const Render& other) const;
    bool operator!= (const Render& other) const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    OutputFormatPtr getOutputFormat() const;
    void setOutputFormat(const OutputFormatPtr& format);

    wxFileName getFileName() const;
    void setFileName(const wxFileName& filename);

    bool getSeparateAtCuts() const;
    void setSeparateAtCuts(bool separate);

    ///\ return true if the given filename can be used for rendering
    bool checkFileName() const;

    /// \return true if the given filename can be used for rendering
    /// \param filename filename to be checked
    static bool checkFileName(const wxFileName& filename);

    /// \return a clone of this object with the filename (last part) removed
    /// This returned object can be used in comparisons, where the file name
    /// part is not relevant for the comparison.
    RenderPtr withFileNameRemoved() const;

    //////////////////////////////////////////////////////////////////////////
    // SCHEDULING
    //////////////////////////////////////////////////////////////////////////

    static void schedule(const SequencePtr& sequence);
    static void scheduleAll();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxFileName mFileName;
    OutputFormatPtr mOutputFormat;
    bool mSeparateAtCuts;
    pts mStart;
    pts mEnd;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const Render& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

}} // namespace

BOOST_CLASS_VERSION(model::render::Render, 2)
BOOST_CLASS_EXPORT_KEY(model::render::Render)

#endif
