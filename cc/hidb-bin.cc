#include <iostream>

#include "acmacs-base/string.hh"
#include "hidb-5/hidb-bin.hh"

// ----------------------------------------------------------------------

std::string hidb::bin::signature()
{
    return {"HIDB0500", 8};

} // hidb::bin::signature

// ----------------------------------------------------------------------

bool hidb::bin::has_signature(const char* data)
{
    const std::string sig = signature();
    return !std::memcmp(data, sig.data(), sig.size());

} // hidb::bin::has_signature

// ----------------------------------------------------------------------

std::string hidb::bin::Antigen::name() const
{
    const auto y = year();
    if (y[0])
        return string::join("/", {std::string{host()}, std::string{location()}, std::string{isolation()}, y});
    else
        return string::join(" ", {std::string{location()}, std::string{isolation()}}); // cdc name

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

std::vector<std::string> hidb::bin::Antigen::lab_ids() const
{
    std::vector<std::string> result;
    for (size_t no = 0; no < sizeof(lab_id_offset); ++no) {
          // ignore padding after lab id
        const auto* start = _start() + lab_id_offset[no];
        auto* end = _start() + lab_id_offset[no+1];
        while (end > start && !end[-1])
            --end;
        if (end > start)
            result.emplace_back(start, static_cast<size_t>(end - start));
    }
    return result;

} // hidb::bin::Antigen::lab_ids

// ----------------------------------------------------------------------

std::vector<std::string> hidb::bin::Antigen::annotations() const
{
    std::vector<std::string> result;
    for (size_t no = 0; no < sizeof(annotation_offset); ++no) {
        if (const auto size = annotation_offset[no+1] - annotation_offset[no]; size > 0)
            result.emplace_back(_start() + annotation_offset[no], static_cast<size_t>(size));
    }
    return result;

} // hidb::bin::Antigen::annotations

// ----------------------------------------------------------------------

std::string hidb::bin::Serum::name() const
{
      // host, location, year are empty if name was not recognized
    return string::join("/", {std::string{host()}, std::string{location()}, std::string{isolation()}, year()});

} // hidb::bin::Serum::name

// ----------------------------------------------------------------------

std::vector<std::string> hidb::bin::Serum::annotations() const
{
    std::vector<std::string> result;
    for (size_t no = 0; no < sizeof(annotation_offset); ++no) {
        if (const auto size = annotation_offset[no+1] - annotation_offset[no]; size > 0)
            result.emplace_back(_start() + annotation_offset[no], static_cast<size_t>(size));
    }
    return result;

} // hidb::bin::Serum::annotations

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
