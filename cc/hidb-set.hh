#pragma once

#include <string>
#include <optional>

#include "acmacs-base/timeit.hh"

namespace hidb
{

    class HiDb;

    void setup(std::string aHiDbDir, std::optional<std::string> aLocDbFilename = {}, bool aVerbose = false);
    [[nodiscard]] const HiDb& get(std::string aVirusType, report_time timer = report_time::Yes);

} // namespace hidb

// ----------------------------------------------------------------------
