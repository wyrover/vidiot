#include "UtilLog.h"

#include <wx/datetime.h>
#include <wx/thread.h>
#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>
#include <share.h> // _SH_DENYWR
#include "UtilFifo.h"
#include "UtilAssert.h"

IMPLEMENTENUM(LogLevel);

static std::string sFilename("");
static const unsigned int sMaximumBufferedLoglines = 1000;

LogLevel Log::sReportingLevel = logINFO;

////////////////////////////////////////////////////////////////////////////////
// THREAD SAFETY & WRITING TO DISK
////////////////////////////////////////////////////////////////////////////////

class LogWriter;
static LogWriter* sWriter = 0;

class LogWriter : boost::noncopyable
{
public:
    LogWriter()
        :   mEnabled(true)
        ,   mFifo(sMaximumBufferedLoglines)
        ,   mFile(0)
    {
        mFile = _fsopen(sFilename.c_str(),"w",_SH_DENYWR);
        if (!mFile)
        {
            // If file open fails, mFile == 0. Then, nothing will be logged.
            mEnabled = false;
        }
        else
        {
            mThread.reset(new boost::thread(boost::bind(&LogWriter::thread,this)));
        }
    }
    ~LogWriter()
    {
        mEnabled = false;
        if (mThread)
        {
            mFifo.push("END"); // To avoid deadlock if the thread() code has just passed the 'while (enabled)' guard and is waiting for a new log message
            mThread->join();
        }
        mThread.reset();
        if (mFile != 0)
        {
            fclose(mFile);
            mFile = 0;
        }
        mFifo.flush();
    }
    void write(const std::string& logLine)
    {
        if (mEnabled)
        {
            mFifo.push(logLine);
        }
    }
private:
    void thread()
    {
        std::string s;
        while (mEnabled)
        {
            s = mFifo.pop();
            fprintf(mFile, "%s", s.c_str());
        	fflush(mFile);
        }
        // Log the remaining lines. This is useful for the
        // case of a failed assertion. breakIntoDebugger()
        // causes the addition of new log lines to stop.
        // (via Log::terminate()->~LogWriter()->mEnabled=false)
        // However, the assert message has already been pushed,
        // and must still be logged. This also helps in ensuring
        // that codec issues are logged via the ffmpeg decoding
        // hook.
        while (mFifo.getSize() > 0)
        {
            s = mFifo.pop();
            fprintf(mFile, "%s", s.c_str());
            fflush(mFile);
        }
    }

    bool mEnabled;
    boost::scoped_ptr<boost::thread> mThread;
    Fifo<std::string> mFifo;
    FILE* mFile;
};

////////////////////////////////////////////////////////////////////////////////
// LOG CLASS
////////////////////////////////////////////////////////////////////////////////

 Log::Log()
{
}

Log::~Log()
{
    os << std::endl;
    sWriter->write(os.str());
}

void Log::SetReportingLevel(LogLevel level)
{
	if (level > logDETAIL)
	{
		sReportingLevel = logDETAIL;
	}
	else
	{
		sReportingLevel = level;
	}
}

void Log::setFileName(std::string path)
{
    sFilename = path;
}

std::string Log::getFileName()
{
    return sFilename;
}

void Log::Init()
{
    sWriter = new LogWriter();
}

void Log::Terminate()
{
    delete sWriter;
    sWriter = 0;
}

std::ostringstream& Log::Get(LogLevel level, const char* p_szFileName, size_t p_lLine, const char* p_szFunction)
{
	static const char* levelstring[] = {"NONE", "ERROR", "WARNING", "INFO", "DEBUG", "VIDEO", "AUDIO", "DETAIL", "ASSERT"};
    os << boost::format("%s % 7s: t@%04x %s(%d): %s: ") % wxDateTime::UNow().Format("%d-%m-%Y %H:%M:%S.%l") % levelstring[level] % wxThread::GetCurrentId() % p_szFileName % p_lLine % p_szFunction;
	return os;
}

////////////////////////////////////////////////////////////////////////////////
// CLASS LOGVARS
////////////////////////////////////////////////////////////////////////////////

LogVar::~LogVar()
{
    if (mAssert)
    {
        // NOTE: In this case, the debug break may cause the log line not to be written to file.
        //       Not an issue, since this is for developers only.
        Log().Get(mLevel, mFileName, mLine, mFunction) << *mAssert << osVars.str() ;
        IAssert::breakIntoDebugger(*mAssert);
    }
    else
    {
        Log().Get(mLevel, mFileName, mLine, mFunction) << osVars.str();
    }
}
