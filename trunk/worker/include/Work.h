#ifndef WORK_H
#define WORK_H

namespace worker {

typedef boost::function< void() > Callable;

class Work
    : public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    , boost::noncopyable
    , public boost::enable_shared_from_this<Work>
{
public:
    explicit Work(Callable work);
    virtual ~Work();

    void execute();

    void abort();

protected:

    bool isAborted() const;

    void showProgressText(wxString text);
    void showProgressBar(int max);
    void showProgress(int value);

    void setThreadName(std::string name);

private:

    Callable mCallable;
    bool mAbort;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const Work& obj);

};

std::ostream& operator<< (std::ostream& os, const WorkPtr& obj);

} // namespace

#endif // WORK_H