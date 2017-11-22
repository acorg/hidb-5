#include "acmacs-base/string.hh"
#include "hidb-5/hidb-bin.hh"

// ----------------------------------------------------------------------

std::string hidb::bin::Antigen::name() const
{
    const auto y = year();
    if (y[0])
        return string::join("/", {host(), location(), isolation(), y});
    else
        return string::join(" ", {location(), isolation()}); // cdc name

} // hidb::bin::Antigen::name

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
