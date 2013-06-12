#ifndef HELPER_WORKER_H
#define HELPER_WORKER_H

namespace test {

/// Construct this object before triggering an action that causes
/// work objects to be scheduled and executed. After the trigger
/// is given, call verify() to wait and verify that that exact
/// amount of Work objects was executed. To do the latter, a
/// limit is sent (on the Worker) on the maximum number of
/// executed Work.
struct ExpectExecutedWork
{
    explicit ExpectExecutedWork(int nWork);
    virtual ~ExpectExecutedWork();

    /// Block execution until the given amount of Work has been
    /// scheduled AND executed by the Wworker.
    void wait();

private:

    int mWork;
    bool mWaited;

};

} // namespace

#endif // HELPER_WORKER_H