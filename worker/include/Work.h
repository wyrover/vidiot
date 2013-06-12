#ifndef WORK_H
#define WORK_H

typedef boost::function< void() > Callable;

namespace worker {

class Work
    : public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    , boost::noncopyable
    , public boost::enable_shared_from_this<Work>
{
public:
    explicit Work(Callable work);
    virtual ~Work();

    void execute();

private:

    Callable mCallable;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const Work& obj);

};

std::ostream& operator<< (std::ostream& os, const WorkPtr& obj);

} // namespace

#endif // WORK_H