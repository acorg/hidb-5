#pragma once

#include <string>
#include <optional>

#include "acmacs-base/timeit.hh"

namespace hidb
{
    class HiDb;

    class get_error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    void setup(std::string_view aHiDbDir, std::optional<std::string> aLocDbFilename = {}, bool aVerbose = false);
    [[nodiscard]] const HiDb& get(std::string_view aVirusType, report_time timer = report_time::yes); // throws get_error
    [[nodiscard]] inline const HiDb& get(std::string aVirusType, report_time timer = report_time::yes) { return get(std::string_view(aVirusType), timer); }

} // namespace hidb

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
