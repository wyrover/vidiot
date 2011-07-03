#ifndef DATA_OBJECT_H
#define DATA_OBJECT_H

#include <list>
#include <wx/dataobj.h>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

namespace model {
class INode;
typedef boost::shared_ptr<INode> NodePtr;
typedef std::list<NodePtr> NodePtrs;
}

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

    virtual bool GetDataHere(void *buf) const;
    virtual size_t GetDataSize () const;
    virtual bool SetData(size_t len, const void *buf);

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