#ifndef DETAILS_H
#define DETAILS_H

namespace gui { namespace timeline {

class DetailsPanel;
class Timeline;

class Details
    :   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Details(wxWindow* parent, Timeline* timeline);
    virtual ~Details();

    void update();

    //////////////////////////////////////////////////////////////////////////
    // GET A CERTAIN DETAILS TYPE
    //////////////////////////////////////////////////////////////////////////

    template <typename DETAILS>
    DETAILS* get()
    {
        BOOST_FOREACH(DetailsPanel* details, mDetails)
        {
            DETAILS* result = dynamic_cast<DETAILS*>(details);
            if (result)
            {
                return result;
            }
        }
        FATAL("Not found");
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    wxWindow* getCurrent() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxStaticText* mHeader;
    wxWindow* mCurrent;
    std::list<DetailsPanel*> mDetails;

};

}} // namespace

#endif // DETAILS_H