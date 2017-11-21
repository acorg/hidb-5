#include "acmacs-base/string.hh"
#include "acmacs-base/read-file.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/hidb-bin.hh"
#include "hidb-5/hidb-json.hh"

// ----------------------------------------------------------------------

hidb::HiDb::HiDb(std::string aFilename, report_time timer)
{
    Timeit ti("reading hidb from " + aFilename + ": ", timer);
    const std::string data = acmacs::file::read(aFilename);
    if (data.find("\"  version\": \"hidb-v5\"") != std::string::npos)
        hidb::json::read(data);
    else
        throw std::runtime_error("[hidb] unrecognized file: " + aFilename);

} // hidb::HiDb::HiDb

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
