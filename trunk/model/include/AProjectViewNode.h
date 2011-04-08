#ifndef A_PROJECT_VIEW_NODE
#define A_PROJECT_VIEW_NODE

#include <list>
#include <wx/string.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/version.hpp>
#include "UtilEvent.h"
#include "UtilRTTI.h"

namespace model {

class AProjectViewNode;
typedef AProjectViewNode* ProjectViewId;
typedef boost::shared_ptr<AProjectViewNode> ProjectViewPtr;
typedef std::list<ProjectViewPtr> ProjectViewPtrs;

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

struct ParentAndChild
{
    ParentAndChild(model::ProjectViewPtr _parent, model::ProjectViewPtr _child) : parent(_parent), child(_child) {}
    model::ProjectViewPtr parent;
    model::ProjectViewPtr child;
};

DECLARE_EVENT(EVENT_ADD_ASSET,      EventAddAsset,      ParentAndChild);
DECLARE_EVENT(EVENT_REMOVE_ASSET,   EventRemoveAsset,   ParentAndChild);

struct NodeWithNewName
{
    NodeWithNewName(model::ProjectViewPtr _node, wxString _newname) : node(_node), newname(_newname) {}
    model::ProjectViewPtr node;
    wxString newname;
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

    /**
    * Remove the substructure below this node. This type of explicit tree
    * deletion is needed due to the extensive use of shared_ptr's. The whole
    * tree is made of shared_ptr's. That means that parents keep children alive,
    * and vice versa. The references between them have to be explicitly destroyed
    * which is done here.
    * @todo move to the project class which can access the entire tree.
    * @todo reimplement in sequence, track, clip etc. For instance Track and Clip keep each other alive.
    */
    virtual void Delete();

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

    //////////////////////////////////////////////////////////////////////////
    // ATTRIBUTES
    //////////////////////////////////////////////////////////////////////////

    virtual wxString getName() const = 0;
    virtual void setName(wxString name);

protected:

    ProjectViewPtr mParent;
    std::list<ProjectViewPtr> mChildren;

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

BOOST_SERIALIZATION_ASSUME_ABSTRACT(model::AProjectViewNode)
// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::AProjectViewNode, 1)
BOOST_CLASS_EXPORT(model::AProjectViewNode)
BOOST_CLASS_TRACKING(model::AProjectViewNode, boost::serialization::track_always)

BOOST_SERIALIZATION_ASSUME_ABSTRACT(boost::enable_shared_from_this<AProjectViewNode>)

#endif // A_PROJECT_VIEW_NODE
