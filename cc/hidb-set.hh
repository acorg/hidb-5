#pragma once

#include <string>
#include <optional>

#include "acmacs-base/timeit.hh"
#include "acmacs-virus/virus-name.hh"

namespace hidb
{
    class HiDb;

    class get_error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    void setup(std::string_view aHiDbDir, std::optional<std::string> aLocDbFilename = {}, bool aVerbose = false);
    void load_all(report_time timer = report_time::no); // pre-load all hidbs (e.g. for acmacs-api-server)
    [[nodiscard]] const HiDb& get(const acmacs::virus::type_subtype_t& aVirusType, report_time timer = report_time::no); // throws get_error

} // namespace hidb

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
