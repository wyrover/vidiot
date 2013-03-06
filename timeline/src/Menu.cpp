#include "Menu.h"

#include "AudioClip.h"
#include "AudioTrack.h"
#include "Clip.h"
#include "CreateAudioTrack.h"
#include "CreateTransition.h"
#include "CreateVideoTrack.h"
#include "EmptyClip.h"
#include "ids.h"
#include "Intervals.h"
#include "MousePointer.h"
#include "PositionInfo.h"
#include "Render.h"
#include "RenderSettingsDialog.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "TimeLinesView.h"
#include "Track.h"
#include "VideoTransition_CrossFade.h"
#include "AudioTransition_CrossFade.h"
#include "RemoveEmptyTracks.h"
#include "RemoveAllEmptyRegions.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "VideoTrack.h"
#include "Window.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

MenuHandler::MenuHandler(Timeline* timeline)
:   wxEvtHandler()
,   Part(timeline)
,   mMenu()
,   mActive(true)
{
    VAR_DEBUG(this);

    mMenu.Append(ID_ADDVIDEOTRACK,  _("Add video track"));
    mMenu.Append(ID_ADDAUDIOTRACK,  _("Add audio track"));
    mMenu.Append(ID_REMOVE_EMPTY_TRACKS,  _("Remove empty video and audio tracks"));
    mMenu.AppendSeparator();
    mMenu.Append(ID_DELETEMARKED,   _("Delete marked regions from sequence"));
    mMenu.Append(ID_DELETEUNMARKED, _("Delete unmarked regions from sequence"));
    mMenu.Append(ID_REMOVEMARKERS,  _("Remove all markers"));
    mMenu.AppendSeparator();
    mMenu.Append(meID_REMOVE_EMPTY,  _("Remove empty regions"));
    mMenu.AppendSeparator();
    mMenu.Append(ID_RENDERSETTINGS, _("Render settings"));
    mMenu.Append(ID_RENDERSEQUENCE, _("Render '") + getSequence()->getName() + "'");
    mMenu.Append(ID_RENDERSEQUENCE, _("Render all modified sequences"));
    mMenu.AppendSeparator();
    mMenu.Append(ID_CLOSESEQUENCE,  _("Close"));

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddVideoTrack,      this, ID_ADDVIDEOTRACK);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddAudioTrack,      this, ID_ADDAUDIOTRACK);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveEmptyTracks,  this, ID_REMOVE_EMPTY_TRACKS);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteMarked,   this, ID_DELETEMARKED);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteUnmarked, this, ID_DELETEUNMARKED);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveMarkers,  this, ID_REMOVEMARKERS);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveAllEmpty,  this, meID_REMOVE_EMPTY);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderSettings, this, ID_RENDERSETTINGS);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderSequence, this, ID_RENDERSEQUENCE);
    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderAll,      this, ID_RENDERALL);

    Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onCloseSequence,  this, ID_CLOSESEQUENCE);

    // Popup menu items
    getTimeline().Bind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddInTransition,       this, meID_ADD_INTRANSITION);
    getTimeline().Bind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddOutTransition,      this, meID_ADD_OUTTRANSITION);
    getTimeline().Bind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddInOutTransition,    this, meID_ADD_INOUTTRANSITION);
    getTimeline().Bind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddInFade,             this, meID_ADD_INFADE);
    getTimeline().Bind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddOutFade,            this, meID_ADD_OUTFADE);
    getTimeline().Bind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddInOutFade,          this, meID_ADD_INOUTFADE);
    getTimeline().Bind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onRemoveEmpty,           this, meID_REMOVE_EMPTY);

    updateItems();

    Window::get().setSequenceMenu(getMenu());
}

MenuHandler::~MenuHandler()
{
    VAR_DEBUG(this);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddVideoTrack,      this, ID_ADDVIDEOTRACK);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onAddAudioTrack,      this, ID_ADDAUDIOTRACK);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveEmptyTracks,  this, ID_REMOVE_EMPTY_TRACKS);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteMarked,   this, ID_DELETEMARKED);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onDeleteUnmarked, this, ID_DELETEUNMARKED);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveMarkers,  this, ID_REMOVEMARKERS);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRemoveAllEmpty,  this, meID_REMOVE_EMPTY);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderSettings, this, ID_RENDERSETTINGS);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderSequence, this, ID_RENDERSEQUENCE);
    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onRenderAll,      this, ID_RENDERALL);

    Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED,    &MenuHandler::onCloseSequence,  this, ID_CLOSESEQUENCE);

    getTimeline().Unbind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddInTransition,    this, meID_ADD_INTRANSITION);
    getTimeline().Unbind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddOutTransition,    this, meID_ADD_OUTTRANSITION);
    getTimeline().Unbind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddInOutTransition,    this, meID_ADD_INOUTTRANSITION);
    getTimeline().Unbind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddInFade,    this, meID_ADD_INFADE);
    getTimeline().Unbind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddOutFade,    this, meID_ADD_OUTFADE);
    getTimeline().Unbind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onAddInOutFade,    this, meID_ADD_INOUTFADE);
    getTimeline().Unbind(wxEVT_COMMAND_MENU_SELECTED,   &MenuHandler::onRemoveEmpty,    this, meID_REMOVE_EMPTY);

    Window::get().setSequenceMenu(0); // If this is NOT the last timeline to be closed, then an 'activate()' will reset the menu to that other timeline
}

//////////////////////////////////////////////////////////////////////////
// MENU INTERFACE
//////////////////////////////////////////////////////////////////////////

wxMenu* MenuHandler::getMenu()
{
    return &mMenu;
}

void MenuHandler::updateItems()
{
    mMenu.Enable( ID_DELETEMARKED,   !getIntervals().isEmpty() );
    mMenu.Enable( ID_DELETEUNMARKED, !getIntervals().isEmpty() );
    mMenu.Enable( ID_REMOVEMARKERS,  !getIntervals().isEmpty() );
}

void MenuHandler::Popup(wxPoint position)
{
    PointerPositionInfo info = getMousePointer().getInfo(position);

    // Mechanism:
    // Default menu options are hidden and enabled.
    // If an item is selected for which a menu option makes sense, then the option is shown.
    // If an item is selected for which a menu option does not make sense, then the option is disabled.

    std::set<model::IClipPtr> selectedClips = getSequence()->getSelectedClips();

    struct MenuOption
    {
        explicit MenuOption(int _id, wxString _text, bool _show = false, bool _enable = true)
            : id(_id)
            , show(_show)
            , enable(_enable)
            , text(_text)
        {}
        int id;
        bool show;
        bool enable;
        wxString text;
        void add(wxMenu& menu)
        {
            if (show)
            {
                menu.Append( id, text );
                menu.Enable( id, enable );
            }
        }
    };

    bool selectedSingleClip = false;
    if ((selectedClips.size() == 1) && (!info.clip->getLink()))
    {
        selectedSingleClip = true;
    }
    if ((selectedClips.size() == 2) && (info.clip->getLink() && info.clip->getLink()->getSelected()))
    {
        selectedSingleClip = true;
    }

    //(info.clip && info.clip->getTrack()->isA<model::VideoTrack>()
    bool clickedOnVideoClip = (info.clip && info.clip->isA<model::VideoClip>());
    bool clickedOnAudioClip = (info.clip && info.clip->isA<model::AudioClip>());
    bool clickedOnEmptyClip = (info.clip && info.clip->isA<model::EmptyClip>());

    bool enableRemoveEmpty = true;

    if (info.onAudioVideoDivider)
    {
    }
    else if (info.onTrackDivider)
    {
    }
    else
    {
        if (info.clip)
        {
            switch (info.logicalclipposition)
            {
            case TransitionBegin:
            case TransitionLeftClipEnd:
            case TransitionInterior:
            case TransitionRightClipBegin:
            case TransitionEnd:
                break;
            case ClipBegin:
                break;
            case ClipInterior:
                break;
            case ClipEnd:
                break;
            default:
                FATAL("Unexpected logical clip position.");
            }
        }
    }

    wxMenu menu;

    MenuOption addInTransition(meID_ADD_INTRANSITION,   _("Add &in transition"),    clickedOnVideoClip, clickedOnVideoClip);
    MenuOption addOutTransition(meID_ADD_OUTTRANSITION, _("Add &out transition"),   clickedOnVideoClip, clickedOnVideoClip);

    MenuOption addInFade(meID_ADD_INFADE,   _("Add fade &in"),    clickedOnAudioClip, clickedOnAudioClip); // todo finish the popup menu handling
    MenuOption addOutFade(meID_ADD_OUTFADE, _("Add fade &out"),   clickedOnAudioClip, clickedOnAudioClip);

    MenuOption removeEmptySpace(meID_REMOVE_EMPTY, _("&Remove empty space"),   clickedOnEmptyClip, clickedOnEmptyClip);

    addInTransition.add(menu);
    addOutTransition.add(menu);
    addInFade.add(menu);
    addOutFade.add(menu);
    removeEmptySpace.add(menu);
    menu.AppendSeparator();

    //menu.AppendSeparator();
    //pAddMenu = menu.AppendSubMenu(&addMenu,_("&Add"));
    //menu.AppendSeparator();
    //pCreateMenu = menu.AppendSubMenu(&createMenu,_("&New"));
    //menu.UpdateUI(&getWindow());

    getTimeline().PopupMenu(&menu);
}

void MenuHandler::activate(bool active)
{
    VAR_ERROR(active);
    mActive = active;
    if (mActive)
    {
        Window::get().setSequenceMenu(getMenu());
    }
}

//////////////////////////////////////////////////////////////////////////
// SEQUENCE MENU
//////////////////////////////////////////////////////////////////////////

void MenuHandler::onAddVideoTrack(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        (new command::CreateVideoTrack(getSequence()))->submit();
    }
    event.Skip();
}

void MenuHandler::onAddAudioTrack(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        (new command::CreateAudioTrack(getSequence()))->submit();
    }
    event.Skip();
}

void MenuHandler::onRemoveEmptyTracks(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        (new command::RemoveEmptyTracks(getSequence()))->submit();
    }
    event.Skip();
}

void MenuHandler::onDeleteMarked(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        getIntervals().deleteMarked();
    }
    event.Skip();
}

void MenuHandler::onDeleteUnmarked(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        getIntervals().deleteUnmarked();
    }
    event.Skip();
}

void MenuHandler::onRemoveMarkers(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        getIntervals().clear();
    }
    event.Skip();
}

void MenuHandler::onRemoveAllEmpty(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        (new command::RemoveAllEmptyRegions(getSequence()))->submit();
    }
    event.Skip();
}

void MenuHandler::onRenderSettings(wxCommandEvent& event)
{
    if (mActive)
    {
        gui::RenderSettingsDialog(getSequence()).ShowModal();
    }
    event.Skip();
}

void MenuHandler::onRenderSequence(wxCommandEvent& event)
{
    if (mActive)
    {
        if (!getSequence()->getRender()->checkFileName())
        {
            gui::RenderSettingsDialog(getSequence()).ShowModal();
        }
        else
        {
            model::render::Render::schedule(getSequence());
        }
    }
    event.Skip();
}

void MenuHandler::onRenderAll(wxCommandEvent& event)
{
    // Done, regardless of the active timeline
    model::render::Render::scheduleAll();
    event.Skip();
}

void MenuHandler::onCloseSequence(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        TimelinesView& tv = Window::get().getTimeLines();
        tv.Close();
        // NOT: event.Skip(); - Causes crash, since the originating menu has been removed.
    }
    else
    {
        event.Skip();
    }
}

//////////////////////////////////////////////////////////////////////////
// POPUP MENU
//////////////////////////////////////////////////////////////////////////

void MenuHandler::onAddInTransition(wxCommandEvent& event)
{
    LOG_INFO;
    createTransition(boost::make_shared<model::video::transition::CrossFade>());
    event.Skip();
}

void MenuHandler::onAddOutTransition(wxCommandEvent& event)
{
    LOG_INFO; // todo make transitionfactory.... avoiding having to include all types of transitions everywhere. See also Idle::addTransition
    createTransition(boost::make_shared<model::video::transition::CrossFade>()); // todo this does not work, sometimes makes inouttransition, not in-only transition
    event.Skip();
}

void MenuHandler::onAddInOutTransition(wxCommandEvent& event)
{
    LOG_INFO;
    createTransition(boost::make_shared<model::video::transition::CrossFade>());
    event.Skip();
}

void MenuHandler::onAddInFade(wxCommandEvent& event)
{
    LOG_INFO;
    createTransition(boost::make_shared<model::audio::transition::CrossFade>());
    event.Skip();
}

void MenuHandler::onAddOutFade(wxCommandEvent& event)
{
    LOG_INFO;
    createTransition(boost::make_shared<model::audio::transition::CrossFade>());
    event.Skip();
}

void MenuHandler::onAddInOutFade(wxCommandEvent& event)
{
    LOG_INFO;
    createTransition(boost::make_shared<model::audio::transition::CrossFade>());
    event.Skip();
}

void MenuHandler::onRemoveEmpty(wxCommandEvent& event)
{
    if (mActive)
    {
        LOG_INFO;
        // todo next: make emptyclip selectable, then allow delete??(new command::RemoveEmptyTracks(getSequence()))->submit();
    }
    event.Skip();    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void MenuHandler::createTransition(model::TransitionPtr transition)
{
    command::CreateTransition* cmd = new command::CreateTransition(getSequence(), getMousePointer().getRightDownPosition(), transition);
    if (cmd->isPossible())
    {
        cmd->submit();
    }
    else
    {
        delete cmd;
    }
}

}} // namespace