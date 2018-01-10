#pragma once

#include <string>

// ----------------------------------------------------------------------

namespace hidb::json
{

    [[nodiscard]] std::string read(std::string aData, bool verbose);

} // namespace hidb::json

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
