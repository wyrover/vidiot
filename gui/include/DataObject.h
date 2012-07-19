#ifndef DATA_OBJECT_H
#define DATA_OBJECT_H

namespace gui {

class DataObject
    :   public wxDataObjectSimple
    ,   public boost::noncopyable
{
public:

    /// A callback method which is called upon destruction.
    /// This is used to let the ProjectView know the end of a drag operation.
    typedef boost::function<void ()> CallbackOnDestruction;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DataObject();
    DataObject(model::NodePtrs assets, CallbackOnDestruction callback = 0);
    ~DataObject();

    static const wxString sFormat;

    //////////////////////////////////////////////////////////////////////////
    // FROM wxDataObjectSimple
    //////////////////////////////////////////////////////////////////////////

    virtual bool GetDataHere(void *buf) const override;
    virtual size_t GetDataSize () const override;
    virtual bool SetData(size_t len, const void *buf) override;

    //////////////////////////////////////////////////////////////////////////
    // GET ALL ASSETS
    //////////////////////////////////////////////////////////////////////////

    model::NodePtrs getAssets() const;

private:

    wxDataFormat mFormat;
    model::NodePtrs mAssets;
    CallbackOnDestruction mCallback;
};

} // namespace

#endif // DATA_OBJECT_H