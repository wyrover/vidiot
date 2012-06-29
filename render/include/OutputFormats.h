#ifndef RENDER_OUTPUT_FORMATS_H
#define RENDER_OUTPUT_FORMATS_H

#include <wx/string.h>
#include <list>
#include <boost/shared_ptr.hpp>

namespace model { namespace render {

class OutputFormat;
typedef boost::shared_ptr<OutputFormat> OutputFormatPtr;

typedef std::list<OutputFormatPtr> OutputFormatList;

class OutputFormats
{
public:
    static void initialize();
    static void add(OutputFormat format);

    static OutputFormatList getList();

    static std::list<wxString> getNames();
    static OutputFormatPtr getByName(wxString name);
    static OutputFormatPtr getByExtension(wxString extension);

    static OutputFormatPtr getDefault();

private:
    static OutputFormatList sOutputFormats;
};

}} // namespace

#endif // RENDER_OUTPUT_FORMATS_H