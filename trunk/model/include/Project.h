#ifndef PROJECT_H
#define PROJECT_H

#include <wx/docview.h>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <boost/shared_ptr.hpp>
#include "RootCommand.h"
#include "UtilEvent.h"

class Work;
typedef boost::shared_ptr<Work> WorkPtr;

namespace model {

class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;
class Properties;
typedef boost::shared_ptr<Properties> PropertiesPtr;
class Project;

/// \class Project
/// This class is not managed via shared_ptr's since it's construction/destruction
/// is managed by the wxWidgets document/view framework. Therefore, pointer ownership
/// cannot be transferred to a shared_ptr. This was also the rational to not let
/// Project inherit from ProjectAsset, and let the document be the root node. Instead,
/// the document has a separate root node 'mRoot'.
/// \image html Project.png
class Project
    :   public wxDocument
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Project();
    ~Project();

    /// Get the get project. Used to avoid having to pass this object
    /// (or the Properties of it) throughout the entire code.
    static Project& get();

    //////////////////////////////////////////////////////////////////////////
    // OVERRIDES - wxDocument
    //////////////////////////////////////////////////////////////////////////

    /// Required for destruction (closing) of the project, because the lifetime
    /// of the project is handled by wxWidgets. Furthermore, the project tree
    /// (shared_ptr's to 'Node') is not automatically cleaned up
    /// upon destruction since parents and children keep references to each other.
    /// This method ensures that all these bidirectional references are removed.
    bool DeleteContents() override;
    bool OnCloseDocument() override;
    bool OnNewDocument() override;
    bool OnCreate(const wxString& path, long flags) override;
    void OnChangeFilename(bool notifyViews) override;
    virtual void Modify(bool modify) override;

    //////////////////////////////////////////////////////////////////////////
    // LOAD/SAVE - wxDocument
    //////////////////////////////////////////////////////////////////////////

    std::ostream& SaveObject(std::ostream& stream) override;
    std::istream& LoadObject(std::istream& stream) override;

    /// Overridden to change the error dialogs.
    bool DoOpenDocument(const wxString& file) override;


    //////////////////////////////////////////////////////////////////////////
    // CHANGE COMMANDS
    //////////////////////////////////////////////////////////////////////////

    void Submit(command::RootCommand* c);

    //////////////////////////////////////////////////////////////////////////
    // ACCESSORS
    //////////////////////////////////////////////////////////////////////////

    FolderPtr getRoot() const;
    PropertiesPtr getProperties() const;

private:

    FolderPtr mRoot;
    PropertiesPtr mProperties;

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    DECLARE_DYNAMIC_CLASS(Project)

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
BOOST_CLASS_VERSION(model::Project, 1)

#endif // PROJECT_H
