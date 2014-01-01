// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef I_NODE_H
#define I_NODE_H

#include "UtilRTTI.h"
#include "UtilSelf.h"

namespace model {

class INode;
typedef INode* NodeId;

/// Base class for all model objects that can be made visible in the project view
class INode
    :   public Self<INode>
    ,   public IRTTI
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    INode();
    virtual ~INode();

    //////////////////////////////////////////////////////////////////////////
    // IDS
    //////////////////////////////////////////////////////////////////////////

    NodeId id();

    static NodePtr Ptr(NodeId id);

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
    virtual NodePtrs getAllDescendants() const = 0;

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

    friend std::ostream& operator<<( std::ostream& os, const INode& obj );

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
BOOST_CLASS_VERSION(model::INode, 1)
BOOST_CLASS_EXPORT_KEY(model::INode)

#endif // I_NODE_H