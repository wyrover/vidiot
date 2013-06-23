#ifndef I_NODE_H
#define I_NODE_H

#include "UtilRTTI.h"

namespace model {

class INode;
typedef INode* NodeId;

/// Base class for all model objects that can be made visible in the project view
class INode
    :   public boost::enable_shared_from_this<INode>
    ,   public IRTTI
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    INode() {};
    virtual ~INode() {};

    //////////////////////////////////////////////////////////////////////////
    // IDS
    //////////////////////////////////////////////////////////////////////////

    virtual NodeId id()
    {
        return static_cast<NodeId>(this);
    }

    static NodePtr Ptr(NodeId id)
    {
        return id->shared_from_this();
    }

    //////////////////////////////////////////////////////////////////////////
    // STRUCTURE
    //////////////////////////////////////////////////////////////////////////

    virtual bool hasParent() const = 0;
    virtual NodePtr getParent() const = 0;
    virtual void setParent(NodePtr parent) = 0;

    virtual NodePtr addChild(NodePtr newChild) = 0;
    virtual NodePtrs addChildren(NodePtrs children) = 0;

    virtual NodePtr removeChild(NodePtr child) = 0;
    virtual NodePtrs removeChildren(NodePtrs children) = 0;

    virtual NodePtrs getChildren() const = 0;

    /// Count the total number of items in the entire tree.
    /// The root node is included in the count.
    /// \return total count of all nodes in the tree, including this
    virtual int count() const = 0;

    /// Find all descendants with the given name, throughout
    /// the hierarchy.
    virtual NodePtrs find(wxString name) = 0;

    /// Find all descendants with the given path (on disk), throughout
    /// the hierarchy. Only returns nodes that are of type IPath.
    /// \param path fully expanded path
    virtual NodePtrs findPath(wxString path) = 0;

    /// \return true if the given path must be watched
    /// A folder (on disk) must be watched, one or more files in this folder
    /// (on disk) are present in the project tree.
    virtual bool mustBeWatched(wxString path) = 0;

    /// Is called when the object should do a check for consistency.
    /// i.e. Is the file/folder on disk still present?
    virtual void check() = 0;

    //////////////////////////////////////////////////////////////////////////
    // ATTRIBUTES
    //////////////////////////////////////////////////////////////////////////

    virtual wxString getName() const = 0;
    virtual void setName(wxString name) = 0;

private:

    /// Made private since behavior not correct: No member cloning is done.
    /// Reason: This class is also the baseclass for File objects, which are not
    /// necessarily part of the project tree. Hence, the project tree members
    /// (mParent/mChildren) should not be cloned always. Specifically, when using
    /// a File that IS part of the project view tree, and cloning that "INTO" a
    /// sequence, the mParent/mChildren members should not be cloned.
    INode(const INode& other) {};

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const INode& obj )
    {
        os << &obj << '|' << obj.getName() << '|' << obj.getParent() << '|' << obj.getChildren().size();
        return os;
    }

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    };
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::INode, 1)

#endif // I_NODE_H