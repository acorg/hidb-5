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

std::string hidb::bin::Antigen::date() const
{
    if (date_offset == table_index_offset)
        return {};
    const std::string brief = std::to_string(*reinterpret_cast<const date_t*>(_start() + date_offset));
    if (brief.size() != 8)
        return brief;
    return string::join("-", {brief.substr(0, 4), brief.substr(4, 2), brief.substr(6, 2)});

} // hidb::bin::Antigen::date

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
