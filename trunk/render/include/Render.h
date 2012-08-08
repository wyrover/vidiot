#ifndef MODEL_RENDER_H
#define MODEL_RENDER_H

#include "UtilCloneable.h"
#include "UtilEvent.h"

namespace model { namespace render {

class Render
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public ICloneable
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

	virtual ~Render();

    virtual Render* clone() const override;

    //////////////////////////////////////////////////////////////////////////
    // OPERATORS
    //////////////////////////////////////////////////////////////////////////

    bool operator== (const Render& other) const;
    bool operator!= (const Render& other) const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    OutputFormatPtr getOutputFormat() const;
    void setOutputFormat(OutputFormatPtr format);

    wxFileName getFileName() const;
    void setFileName(wxFileName filename);

    ///\ return true if the given filename can be used for rendering
    bool checkFileName() const;

    /// \return a clone of this object with the filename (last part) removed
    /// This returned object can be used in comparisons, where the file name
    /// part is not relevant for the comparison.
    RenderPtr withFileNameRemoved() const;

    //////////////////////////////////////////////////////////////////////////
    // SCHEDULING
    //////////////////////////////////////////////////////////////////////////

    static void schedule(SequencePtr sequence);
    static void scheduleAll();

    //////////////////////////////////////////////////////////////////////////
    // RENDERING
    //////////////////////////////////////////////////////////////////////////

    void generate(SequencePtr sequence);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxFileName mFileName;
    OutputFormatPtr mOutputFormat;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Render& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

};

}} // namespace

BOOST_CLASS_VERSION(model::render::Render, 1)

#endif // MODEL_RENDER_H