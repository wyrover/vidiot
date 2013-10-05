// Copyright 2013 Eric Raijmakers.
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

#ifndef A_PROJECT_VIEW_NODE
#define A_PROJECT_VIEW_NODE

#include "INode.h"

namespace model {

class Node
    :   public INode
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Node();
    virtual ~Node();

    //////////////////////////////////////////////////////////////////////////
    // INODE
    //////////////////////////////////////////////////////////////////////////

    bool hasParent() const override;
    NodePtr getParent() const override;
    void setParent(NodePtr parent) override;

    NodePtr addChild(NodePtr newChild) override;
    NodePtrs addChildren(NodePtrs children) override;

    NodePtr removeChild(NodePtr child) override;
    NodePtrs removeChildren(NodePtrs children) override;

    NodePtrs getChildren() const override;
    NodePtrs getAllDescendants() const override;

    int count() const override;

    NodePtrs find(wxString name) override;

    virtual NodePtrs findPath(wxString path) override;
    virtual bool mustBeWatched(wxString path) override;

    virtual void setName(wxString name) override;

protected:

    WeakNodePtr mParent; // Children do not keep parents alive
    NodePtrs mChildren;  // Parents keep children alive

private:

    /// Made private since behavior not correct: No member cloning is done.
    /// Reason: This class is also the baseclass for File objects, which are not
    /// necessarily part of the project tree. Hence, the project tree members
    /// (mParent/mChildren) should not be cloned always. Specifically, when using
    /// a File that IS part of the project view tree, and cloning that "INTO" a
    /// sequence, the mParent/mChildren members should not be cloned.
    Node(const Node& other);

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Node& obj );

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
BOOST_CLASS_VERSION(model::Node, 1)
BOOST_CLASS_EXPORT_KEY(model::Node)

#endif // A_PROJECT_VIEW_NODE