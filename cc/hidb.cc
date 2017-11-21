#include "acmacs-base/string.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/rjson.hh"
#include "hidb-5/hidb.hh"

// ----------------------------------------------------------------------

hidb::HiDb::HiDb(std::string aFilename, report_time timer)
{
    Timeit ti("reading hidb from " + aFilename + ": ", timer);
    const std::string data = acmacs::file::read(aFilename);
    if (data.find("\"  version\": \"hidb-v5\"") != std::string::npos)
        read_json(data);
    else
        throw std::runtime_error("[hidb] unrecognized file: " + aFilename);

} // hidb::HiDb::HiDb

// ----------------------------------------------------------------------

void hidb::HiDb::read_json(std::string aData)
{
    const auto val = rjson::parse_string(aData);

} // hidb::HiDb::read_json

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
