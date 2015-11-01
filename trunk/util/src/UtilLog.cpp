// Copyright 2013-2015 Eric Raijmakers.
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

#include "UtilLog.h"

#include "Dialog.h"
#include "UtilAssert.h"
#include "UtilFifo.h"
#include "UtilPath.h"
#include "UtilStackWalker.h"
#include "UtilThread.h"
#ifdef _MSC_VER
#include <share.h> // _SH_DENYWR
#endif

namespace boost
{
    void assertion_failed(char const * expr, char const * function, char const * file, long line)
    {
        LogVar(expr, file, line, function);
    };
    void assertion_failed_msg(char const * expr, char const * msg, char const * function, char const * file, long line)
    {
        LogVar(expr, file, line, function);
    }
}

IMPLEMENTENUM(LogLevel);

std::map<LogLevel, wxString> LogLevelConverter::mapToHumanReadibleString = {
    { LogError, _("Errors") },
    { LogWarning, _("Errors & Warnings") },
    { LogInfo, _("Info (UI actions)") },
    { LogDebug, _("Debugging") },
};

static std::string sFilename("");
static const unsigned int sMaximumBufferedLoglines = 10000;

LogLevel Log::sReportingLevel = LogWarning;

////////////////////////////////////////////////////////////////////////////////
// THREAD SAFETY & WRITING TO DISK
////////////////////////////////////////////////////////////////////////////////

class LogWriter;
static LogWriter* sWriter = 0;

static std::string sEndString("END");

class LogWriter : boost::noncopyable
{
public:
    LogWriter()
        :   mEnabled(false)
        ,   mFifo(sMaximumBufferedLoglines)
        ,   mFile(0)
    {
        #ifdef _MSC_VER
        mFile = _fsopen(sFilename.c_str(),"w",_SH_DENYWR);
        #else
        mFile = fopen(sFilename.c_str(),"w");
        #endif
        if (!mFile)
        {
            // If file open fails, mFile == 0. Then, nothing will be logged.
            // Scenario to test this: First run release version of testmanual. Close application window, but not the dos box. Then run debug version. File is already in use.
            mEnabled = false;
        }
        else
        {
            int result = setvbuf ( mFile , 0 , _IOFBF , 102400 );
            if (result == 0)
            {
                mEnabled = true;
                mThread.reset(new boost::thread(std::bind(&LogWriter::thread,this)));
            }
        }
    }
    ~LogWriter()
    {
        mEnabled = false;
        if (mThread)
        {
            mFifo.push(sEndString); // To avoid deadlock if the thread() code has just passed the 'while (enabled)' guard and is waiting for a new log message
            mThread->join();
        }
        mThread.reset();
        if (mFile != 0)
        {
            static std::string sDone(" CLOSE");
            fwrite(sDone.c_str(), 1, sDone.length(), mFile);
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
    void flush()
    {
        if (mFile)
        {
            fflush(mFile);
        }
    }
private:
    void thread()
    {
        util::thread::setCurrentThreadName("Log");
        std::string s;
        while (mEnabled)
        {
            s = mFifo.pop();
            fwrite(s.c_str(), 1, s.length(), mFile);
            fflush(mFile);
        }
        // Log the remaining lines. This is useful for the
        // case of a failed assertion. breakIntoDebugger()
        // causes the addition of new log lines to stop.
        // (via Log::exit()->~LogWriter()->mEnabled=false)
        // However, the assert message has already been pushed,
        // and must still be logged. This also helps in ensuring
        // that codec issues are logged via the ffmpeg decoding
        // hook.
        while (mFifo.getSize() > 0)
        {
            s = mFifo.pop();
            fprintf(mFile, "%s", s.c_str());
        }
        fflush(mFile);
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

void Log::setReportingLevel(const LogLevel& level)
{
    ASSERT_LESS_THAN(level,LogLevel_MAX);
    sReportingLevel = level;
}

void Log::flush()
{
    sWriter->flush();
}

void Log::init()
{
    sFilename = std::string(util::path::getLogFilePath().GetFullPath());

    // Start the logger
    sWriter = new LogWriter();
}

void Log::exit()
{
    delete sWriter;
    sWriter = 0;
}

#ifdef _DEBUG
static int lastline;
static std::string lastfile;
#endif

std::ostringstream& Log::get(const LogLevel& level, const char* p_szFileName, const size_t& p_lLine, const char* p_szFunction)
{
    // NOTE: Ensure the same width for the strings below.
    static const char* levelstring[] =
    {
        "NONE    ",
        "ERROR   ",
        "WARNING ",
        "INFO    ",
        "DEBUG   ",
        "ASSERT  "
    };
#ifdef _DEBUG
    // This is done for detecting crashes upon shutdown. Often there's a crash because log lines are made AFTER the logging has been closed.
    // That results in out-of-stack-space errors, wxMBConv::FromWChar<->wxMBConv::WC2MB recursively (for converting the log time stamp, see the 'get' method).
    // Where the log line was made can be seen by inspecting these two variables.
    lastline = p_lLine;
    lastfile = p_szFileName;
#endif
    get(levelstring[level])
        << p_szFileName
        << '(' << std::dec << p_lLine  << ") "
        << p_szFunction
        << ' ';
    return os;
}

std::ostringstream& Log::get(const std::string& category)
{
    os << wxDateTime::UNow().Format(wxT("%d-%m-%Y %H:%M:%S.%l "))
        << category
        << "t@" << std::setw(4) << std::setfill('0') << std::hex <<  wxThread::GetCurrentId()
        << ' ';
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
        std::cout << std::endl << std::endl << *mAssert << std::endl << std::endl << osVars.str();
        Log().get(mLevel, mFileName, mLine, mFunction) << *mAssert << osVars.str();
        LOG_STACKTRACE;
        breakIntoDebugger();
        gui::Dialog::get().getDebugReport();
    }
    else
    {
        Log().get(mLevel, mFileName, mLine, mFunction) << osVars.str();
    }
}
