#pragma once

#include <string>
#include <optional>

#include "acmacs-base/timeit.hh"

namespace hidb
{
    class HiDb;

    class get_error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    void setup(std::string aHiDbDir, std::optional<std::string> aLocDbFilename = {}, bool aVerbose = false);
    [[nodiscard]] const HiDb& get(const std::string& aVirusType, report_time timer = report_time::Yes); // throws get_error

} // namespace hidb

// ----------------------------------------------------------------------
