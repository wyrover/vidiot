#ifndef A_PROJECT_VIEW_NODE
#define A_PROJECT_VIEW_NODE

#include <list>
#include <wx/string.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "UtilEvent.h"
#include "UtilRTTI.h"

namespace model {

class AProjectViewNode;
typedef AProjectViewNode* ProjectViewId;
typedef boost::shared_ptr<AProjectViewNode> ProjectViewPtr;
typedef boost::weak_ptr<AProjectViewNode> WeakProjectViewPtr;
typedef std::list<ProjectViewPtr> ProjectViewPtrs;

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

struct ParentAndChild
{
    ParentAndChild(model::ProjectViewPtr _parent, model::ProjectViewPtr _child) : parent(_parent), child(_child) {}
    model::ProjectViewPtr parent;
    model::ProjectViewPtr child;
    friend std::ostream& operator<<( std::ostream& os, const ParentAndChild& obj )
    {
        os << &obj << '|' << obj.parent << '|' << obj.child;
        return os;
    }
};

DECLARE_EVENT(EVENT_ADD_ASSET,      EventAddAsset,      ParentAndChild);
DECLARE_EVENT(EVENT_REMOVE_ASSET,   EventRemoveAsset,   ParentAndChild);

struct NodeWithNewName
{
    NodeWithNewName(model::ProjectViewPtr _node, wxString _newname) : node(_node), newname(_newname) {}
    model::ProjectViewPtr node;
    wxString newname;
    friend std::ostream& operator<<( std::ostream& os, const NodeWithNewName& obj )
    {
        os << &obj << '|' << obj.node << '|' << obj.newname;
        return os;
    }
};

DECLARE_EVENT(EVENT_RENAME_ASSET,   EventRenameAsset,   NodeWithNewName);

//////////////////////////////////////////////////////////////////////////
// CLASS
//////////////////////////////////////////////////////////////////////////

class AProjectViewNode
    :   public boost::enable_shared_from_this<AProjectViewNode>
    ,   public IRTTI
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AProjectViewNode();
    virtual ~AProjectViewNode();

    //////////////////////////////////////////////////////////////////////////
    // IDS
    //////////////////////////////////////////////////////////////////////////

    ProjectViewId id();
    static ProjectViewPtr Ptr(ProjectViewId id);

    //////////////////////////////////////////////////////////////////////////
    // STRUCTURE
    //////////////////////////////////////////////////////////////////////////

    bool hasParent() const;
    ProjectViewPtr getParent() const;
    void setParent(ProjectViewPtr parent);

    ProjectViewPtr addChild(ProjectViewPtr newChild);
    ProjectViewPtr removeChild(ProjectViewPtr child);
    ProjectViewPtrs getChildren() const;

    /// Find all descendants with the given name, throughout
    /// the hierarchy.
    ProjectViewPtrs find(wxString name);

    //////////////////////////////////////////////////////////////////////////
    // ATTRIBUTES
    //////////////////////////////////////////////////////////////////////////

    virtual wxString getName() const = 0;
    virtual void setName(wxString name);

protected:

    WeakProjectViewPtr mParent; // Children do not keep parents alive
    ProjectViewPtrs mChildren;  // Parents keep children alive

private:

    /// Made private since behavior not correct: No member cloning is done.
    /// Reason: This class is also the baseclass for File objects, which are not
    /// necessarily part of the project tree. Hence, the project tree members
    /// (mParent/mChildren) should not be cloned always. Specifically, when using
    /// a File that IS part of the project view tree, and cloning that "INTO" a
    /// sequence, the mParent/mChildren members should not be cloned.
    AProjectViewNode(const AProjectViewNode& other);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const AProjectViewNode& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::AProjectViewNode, 1)

#endif // A_PROJECT_VIEW_NODE
