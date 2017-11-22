#include "acmacs-base/string.hh"
#include "hidb-5/hidb-bin.hh"

// ----------------------------------------------------------------------

std::string hidb::bin::Antigen::name() const
{
    return string::join("/", {host(), location(), isolation(), year()});

} // hidb::bin::Antigen::name

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
