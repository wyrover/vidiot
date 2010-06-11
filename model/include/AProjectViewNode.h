#ifndef A_PROJECT_VIEW_NODE
#define A_PROJECT_VIEW_NODE

#include <wx/string.h>
#include <boost/enable_shared_from_this.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/version.hpp>
#include "ModelPtr.h"

namespace model {

class AProjectViewNode
    :   public boost::enable_shared_from_this<AProjectViewNode>
{
public:

    AProjectViewNode();
    virtual ~AProjectViewNode();

    /**
    * Remove the substructure below this node. This type of explicit tree
    * deletion is needed due to the extensive use of shared_ptr's. The whole
    * tree is made of shared_ptr's. That means that parents keep children alive,
    * and vice versa. The references between them have to be explicitly destroyed
    * which is done here.
    * @todo move to the project class which can access the entire tree.
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

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_SERIALIZATION_ASSUME_ABSTRACT(model::AProjectViewNode)
BOOST_CLASS_VERSION(model::AProjectViewNode, 1)
BOOST_CLASS_EXPORT(model::AProjectViewNode)
BOOST_CLASS_TRACKING(model::AProjectViewNode, boost::serialization::track_always)

BOOST_SERIALIZATION_ASSUME_ABSTRACT(boost::enable_shared_from_this<AProjectViewNode>)

#endif A_PROJECT_VIEW_NODE
