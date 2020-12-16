#include "hidb-5/report.hh"

// ----------------------------------------------------------------------

void hidb::report_antigens(const hidb::HiDb& hidb, const indexes_t& aIndexes, enum report_tables aReportTables, std::string_view aPrefix)
{
    for (auto index: aIndexes)
        report_antigen(hidb, index, aReportTables, aPrefix);

} // hidb::report_antigens

// ----------------------------------------------------------------------

void hidb::report_antigen(const hidb::HiDb& hidb, const hidb::Antigen& aAntigen, enum report_tables aReportTables, std::string_view aPrefix)
{
    fmt::print("{}{} {} {} {} [{}] {} {}\n", aPrefix, aAntigen.name(), aAntigen.annotations(), aAntigen.reassortant(), aAntigen.passage(), aAntigen.date(), aAntigen.lab_ids(), aAntigen.lineage());
    if (aReportTables != report_tables::none) {
        const std::string pref = std::string{aPrefix} + "    ";
        fmt::print("{}\n", report_tables(hidb, aAntigen.tables(), aReportTables, pref));
    }

} // hidb::report_antigen

// ----------------------------------------------------------------------

void hidb::report_sera(const hidb::HiDb& hidb, const indexes_t& aIndexes, enum report_tables aReportTables, std::string_view aPrefix)
{
    for (auto index: aIndexes)
        report_serum(hidb, index, aReportTables, aPrefix);

} // hidb::report_sera

// ----------------------------------------------------------------------

void hidb::report_serum(const hidb::HiDb& hidb, const hidb::Serum& aSerum, enum report_tables aReportTables, std::string_view aPrefix)
{
    fmt::print("{}{} {} {} {} {} {} {}\n", aPrefix, aSerum.name(), aSerum.annotations(), aSerum.reassortant(), aSerum.serum_id(), aSerum.serum_species(), aSerum.passage(), aSerum.lineage());
    const std::string pref = std::string{aPrefix} + "    ";
    for (size_t ag_no: aSerum.homologous_antigens())
        report_antigen(hidb, ag_no, report_tables::none, pref);
    fmt::print("{}\n", report_tables(hidb, aSerum.tables(), aReportTables, pref));

} // hidb::report_serum

// ----------------------------------------------------------------------

std::string hidb::report_tables(const hidb::HiDb& hidb, const indexes_t& aTables, enum report_tables aReportTables, std::string_view aPrefix)
{
    using namespace std::string_view_literals;
    const auto assay = [](std::string_view src) -> std::string_view {
        if (src == "FOCUS REDUCTION")
            return "FRA"sv;
        else if (src == "HI")
            return "HI "sv;
        else
            return src;
    };
    const auto rbc = [](std::string_view aAssay, std::string_view src) -> std::string_view {
        if (aAssay == "HI") {
            if (src == "guinea-pig")
                return ":gp"sv;
            else if (src == "turkey")
                return ":tu"sv;
            else
                return src;
        }
        else
            return {};
    };

    fmt::memory_buffer out;
    if (aReportTables != report_tables::none) {
        auto hidb_tables = hidb.tables();
        const auto by_lab_assay = hidb_tables->sorted(aTables, lab_assay_rbc_table_t::recent_first);
        if (!by_lab_assay.empty()) {
            switch (aReportTables) {
                case report_tables::all:
                    for (auto entry : by_lab_assay) {
                        fmt::format_to(out, "{}{}:{} ({})", aPrefix, entry.lab, assay(entry.assay), entry.tables.size());
                        for (auto table : entry.tables)
                            fmt::format_to(out, " {}{}", table->date(), rbc(entry.assay, entry.rbc));
                        fmt::format_to(out, "\n");
                    }
                    // if (by_lab_assay.size() > 1)
                    //     fmt::format_to(out, "{}Total tables: {}\n", aPrefix, tables.size());
                    break;
                case report_tables::oldest:
                    // fmt::format_to(out, "{}Tables:{}\n", aPrefix, tables.size());
                    for (auto entry : by_lab_assay)
                        fmt::format_to(out, "{}{}:{} ({})  oldest:{}", aPrefix, entry.lab, assay(entry.assay), entry.tables.size(), entry.tables.back()->name());
                    break;
                case report_tables::recent:
                    // fmt::format_to(out, "{}Tables:{}\n", aPrefix, tables.size());
                    for (auto entry : by_lab_assay)
                        fmt::format_to(out, "{}{}:{} ({})  recent:{}", aPrefix, entry.lab, assay(entry.assay), entry.tables.size(), entry.tables.front()->name());
                    break;
                case report_tables::none:
                    break;
            }
        }
        else
            AD_WARNING("hidb::report_tables: no tables!");

        // std::vector<std::shared_ptr<hidb::Table>> tables(aTables.size());
        // std::transform(aTables.begin(), aTables.end(), tables.begin(), [hidb_tables](size_t aIndex) { return hidb_tables->at(aIndex); });
        // if (!tables.empty()) {
        //     std::sort(tables.begin(), tables.end(), [](auto a, auto b) -> bool { return a->date() > b->date(); });
        //     std::map<std::pair<std::string_view, std::string_view>, std::vector<std::shared_ptr<hidb::Table>>> by_lab_assay;
        //     for (auto table : tables)
        //         by_lab_assay[{table->lab(), table->assay()}].push_back(table);

        //     switch (aReportTables) {
        //         case report_tables::all:
        //             for (auto entry : by_lab_assay) {
        //                 fmt::format_to(out, "{}{}:{} ({})", aPrefix, entry.first.first, assay(entry.first.second), entry.second.size());
        //                 for (auto table : entry.second)
        //                     fmt::format_to(out, " {}{}", table->date(), rbc(entry.first.second, table->rbc()));
        //                 fmt::format_to(out, "\n");
        //             }
        //             if (by_lab_assay.size() > 1)
        //                 fmt::format_to(out, "{}Total tables: {}\n", aPrefix, tables.size());
        //             break;
        //         case report_tables::oldest:
        //             fmt::format_to(out, "{}Tables:{}\n", aPrefix, tables.size());
        //             for (auto entry : by_lab_assay)
        //                 fmt::format_to(out, "{}{}:{} ({})  oldest:{}", aPrefix, entry.first.first, assay(entry.first.second), entry.second.size(), entry.second.back()->name());
        //             break;
        //         case report_tables::recent:
        //             fmt::format_to(out, "{}Tables:{}\n", aPrefix, tables.size());
        //             for (auto entry : by_lab_assay)
        //                 fmt::format_to(out, "{}{}:{} ({})  recent:{}", aPrefix, entry.first.first, assay(entry.first.second), entry.second.size(), entry.second.front()->name());
        //             break;
        //         case report_tables::none:
        //             break;
        //     }
        // }
        // else
        //     AD_WARNING("hidb::report_tables: no tables!");
    }
    return fmt::to_string(out);

} // hidb::report_tables

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
