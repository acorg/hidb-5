#pragma once

#include <string>

namespace acmacs::chart { class Chart; }

// ----------------------------------------------------------------------

class HidbMaker
{
 public:
    inline HidbMaker() = default;

    void add(const acmacs::chart::Chart& aChart);
    void save(std::string aFilename);

 private:

}; // class HidbMaker

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
