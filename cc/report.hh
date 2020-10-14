#pragma once

#include "hidb-5/hidb.hh"

// ----------------------------------------------------------------------

namespace hidb
{
    enum class report_tables { none, all, oldest, recent };
    void report_antigens(const hidb::HiDb& hidb, const indexes_t& aIndexes, bool aReportTables, std::string_view aPrefix = {});
    void report_antigen(const hidb::HiDb& hidb, const hidb::Antigen& aAntigen, bool aReportTables, std::string_view aPrefix = {});
    inline void report_antigen(const hidb::HiDb& hidb, size_t aIndex, bool aReportTables, std::string_view aPrefix = {}) { report_antigen(hidb, *hidb.antigens()->at(aIndex), aReportTables, aPrefix); }
    void report_sera(const hidb::HiDb& hidb, const indexes_t& aIndexes, report_tables aReportTables, std::string_view aPrefix = {});
    void report_serum(const hidb::HiDb& hidb, const hidb::Serum& aSerum, report_tables aReportTables, std::string_view aPrefix = {});
    inline void report_serum(const hidb::HiDb& hidb, size_t aIndex, report_tables aReportTables, std::string_view aPrefix = {}) { report_serum(hidb, *hidb.sera()->at(aIndex), aReportTables, aPrefix); }
    std::string report_tables(const hidb::HiDb& hidb, const indexes_t& aIndexes, enum hidb::report_tables aReportTables, std::string_view aPrefix = {});

} // namespace hidb

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
