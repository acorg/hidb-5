#include "hidb-5/report.hh"

// ----------------------------------------------------------------------

void hidb::report_antigens(const hidb::HiDb& hidb, const indexes_t& aIndexes, bool aReportTables, std::string aPrefix)
{
    for (auto index: aIndexes)
        report_antigen(hidb, index, aReportTables, aPrefix);

} // hidb::report_antigens

// ----------------------------------------------------------------------

void hidb::report_antigen(const hidb::HiDb& hidb, const hidb::Antigen& aAntigen, bool aReportTables, std::string aPrefix)
{
    std::cout << aPrefix << aAntigen.name();
    if (const auto annotations = aAntigen.annotations(); !annotations.empty())
        std::cout << ' ' << annotations;
    if (const auto reassortant = aAntigen.reassortant(); !reassortant.empty())
        std::cout << ' ' << reassortant;
    if (const auto passage = aAntigen.passage(); !passage.empty())
        std::cout << ' ' << passage;
    if (const auto date = aAntigen.date(); !date.empty())
        std::cout << " [" << date << ']';
    if (const auto lab_ids = aAntigen.lab_ids(); !lab_ids.empty())
        std::cout << ' ' << lab_ids;
    if (const auto lineage = aAntigen.lineage(); lineage != acmacs::chart::BLineage::Unknown)
        std::cout << ' ' << static_cast<std::string>(lineage);
    if (aReportTables) {
        std::cout << '\n';
        report_tables(std::cout, hidb, aAntigen.tables(), report_tables::all, aPrefix + "    ");
        std::cout << '\n';
    }
    else
        std::cout << '\n';

} // hidb::report_antigen

// ----------------------------------------------------------------------

void hidb::report_sera(const hidb::HiDb& hidb, const indexes_t& aIndexes, enum hidb::report_tables aReportTables, std::string aPrefix)
{
    for (auto index: aIndexes)
        report_serum(hidb, index, aReportTables, aPrefix);

} // hidb::report_sera

// ----------------------------------------------------------------------

void hidb::report_serum(const hidb::HiDb& hidb, const hidb::Serum& aSerum, enum hidb::report_tables aReportTables, std::string aPrefix)
{
    std::cout << aPrefix << aSerum.name();
    if (const auto annotations = aSerum.annotations(); !annotations.empty())
        std::cout << ' ' << annotations;
    if (const auto reassortant = aSerum.reassortant(); !reassortant.empty())
        std::cout << ' ' << reassortant;
    if (const auto serum_id = aSerum.serum_id(); !serum_id.empty())
        std::cout << ' ' << serum_id;
    if (const auto serum_species = aSerum.serum_species(); !serum_species.empty())
        std::cout << ' ' << serum_species;
    if (const auto passage = aSerum.passage(); !passage.empty())
        std::cout << ' ' << passage;
    if (const auto lineage = aSerum.lineage(); lineage != acmacs::chart::BLineage::Unknown)
        std::cout << ' ' << static_cast<std::string>(lineage);
    std::cout << '\n';
    for (size_t ag_no: aSerum.homologous_antigens())
        report_antigen(hidb, ag_no, false, aPrefix + "    ");
    report_tables(std::cout, hidb, aSerum.tables(), aReportTables, aPrefix + "    ");
    std::cout << '\n';

} // hidb::report_serum

// ----------------------------------------------------------------------

void hidb::report_tables(std::ostream& out, const hidb::HiDb& hidb, const indexes_t& aTables, enum hidb::report_tables aReportTables, std::string aPrefix)
{
    if (aReportTables != report_tables::none) {
        auto hidb_tables = hidb.tables();
        std::vector<std::shared_ptr<hidb::Table>> tables(aTables.size());
        std::transform(aTables.begin(), aTables.end(), tables.begin(), [hidb_tables](size_t aIndex) { return (*hidb_tables)[aIndex]; });
        if (!tables.empty()) {
            std::sort(tables.begin(), tables.end(), [](auto a, auto b) -> bool { return a->date() > b->date(); });
            std::map<std::pair<std::string_view, std::string_view>, std::vector<std::shared_ptr<hidb::Table>>> by_lab_assay;
            for (auto table : tables)
                by_lab_assay[{table->lab(), table->assay()}].push_back(table);
            switch (aReportTables) {
                case report_tables::all:
                    if (by_lab_assay.size() > 1)
                        out << aPrefix << "Tables:" << tables.size() << "  Recent: " << tables[0]->name() << '\n';
                    for (auto entry : by_lab_assay) {
                        out << aPrefix << entry.first.first << ':' << entry.first.second << ' ' << entry.second.size();
                        for (auto table : entry.second)
                            out << ' ' << string::join(":", {table->date(), table->rbc()});
                    }
                    break;
                case report_tables::oldest:
                    out << aPrefix << "Tables:" << tables.size();
                    for (auto entry : by_lab_assay)
                        out << aPrefix << "  " << entry.first.first << ':' << entry.first.second << ' ' << entry.second.size() << " oldest:" << entry.second.back()->name();
                    break;
                case report_tables::recent:
                    out << aPrefix << "Tables:" << tables.size();
                    for (auto entry : by_lab_assay)
                        out << aPrefix << "  " << entry.first.first << ':' << entry.first.second << ' ' << entry.second.size() << " recent:" << entry.second.front()->name();
                    break;
                case report_tables::none:
                    break;
            }
        }
        else
            std::cerr << "WARNING: hidb::report_tables: no tables!\n";
    }

} // hidb::report_tables

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
