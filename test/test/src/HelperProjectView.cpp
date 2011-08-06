#include "HelperProjectView.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "AutoFolder.h"
#include "File.h"
#include "HelperApplication.h"
#include "ids.h"
#include "ProjectView.h"
#include "Sequence.h"
#include "UtilDialog.h"
#include "UtilLog.h"

namespace test {

model::FolderPtr addAutoFolder( wxFileName path, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    waitForIdle();
    UtilDialog::setDir( path.GetShortPath() ); // Add with short path
    triggerMenu(gui::ProjectView::get(),meID_NEW_AUTOFOLDER);
    waitForIdle();

    model::NodePtrs nodes = getRoot()->find( path.GetLongPath() ); // Converted to long path in vidiot
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::AutoFolder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

model::FolderPtr addFolder( wxString name, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    UtilDialog::setText( name );
    triggerMenu(gui::ProjectView::get(),meID_NEW_FOLDER);
    waitForIdle();

    model::NodePtrs nodes = getRoot()->find( name );
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::Folder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

model::SequencePtr addSequence( wxString name, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    UtilDialog::setText( name );
    triggerMenu(gui::ProjectView::get(),meID_NEW_SEQUENCE);
    waitForIdle();

    model::NodePtrs nodes = getRoot()->find( name );
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::Sequence>())(node);
    model::SequencePtr sequence = boost::static_pointer_cast<model::Sequence>(node);
    return sequence;
}

model::SequencePtr createSequence( model::FolderPtr folder )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(folder));
    waitForIdle();
    triggerMenu(gui::ProjectView::get(),meID_CREATE_SEQUENCE);
    waitForIdle();

    model::NodePtrs nodes = getRoot()->find( folder->getName() );
    ASSERT_EQUALS(nodes.size(),2); // The sequence and the folder
    model::SequencePtr result;
    BOOST_FOREACH( model::NodePtr node, nodes )
    {
        if (node->isA<model::Sequence>())
        {
            result = boost::static_pointer_cast<model::Sequence>(node);
        }
    }
    ASSERT(result);
    return result;
}

model::Files addFiles( std::list<wxFileName> paths, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    std::list<wxString> shortpaths;
    BOOST_FOREACH( wxFileName path, paths )
    {
        ASSERT( path.IsAbsolute() );
        shortpaths.push_back( path.GetShortPath() ); // Add with short path
    }
    UtilDialog::setFiles( shortpaths );
    triggerMenu(gui::ProjectView::get(),meID_NEW_FILE);
    waitForIdle();

    model::Files result;
    BOOST_FOREACH( wxFileName path, paths )
    {
        model::NodePtrs nodes = parent->find( path.GetLongPath() ); // Check that long path is generated by vidiot
        ASSERT_EQUALS( nodes.size(), 1 );
        model::NodePtr node = nodes.front();
        ASSERT(node->isA<model::File>())(node);
        result.push_back(boost::static_pointer_cast<model::File>(node));
    }
    return result;
}

void remove( model::NodePtr node )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(node));
    waitForIdle();
    triggerMenu(gui::ProjectView::get(),wxID_DELETE);
    waitForIdle();
}


int countProjectView()
{
    waitForIdle();
    gui::ProjectView::get().selectAll();
    model::NodePtrs selection = gui::ProjectView::get().getSelection();
    int result = selection.size();
    VAR_DEBUG(result);
    return result;
}

} // namespace