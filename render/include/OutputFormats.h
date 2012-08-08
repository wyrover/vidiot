#ifndef RENDER_OUTPUT_FORMATS_H
#define RENDER_OUTPUT_FORMATS_H

namespace model { namespace render {

class OutputFormats
{
public:
    static void initialize();
    static void add(OutputFormat format);

    static OutputFormatList getList();

    static std::list<wxString> getNames();
    static OutputFormatPtr getByName(wxString name);  ///< \return cloned default output format
    static OutputFormatPtr getByExtension(wxString extension); ///< \return cloned output format to be used for given extension

    static OutputFormatPtr getDefault();

private:
    static OutputFormatList sOutputFormats;
};

}} // namespace

#endif // RENDER_OUTPUT_FORMATS_H