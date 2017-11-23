#include <cstdlib>
#include <cmath>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-base/filesystem.hh"
#include "hidb-5/hidb.hh"

using namespace std::string_literals;

static void list_all_antigens(const hidb::HiDb& hidb, const argc_argv& args);
static void list_all_sera(const hidb::HiDb& hidb, const argc_argv& args);
static void report_serum(const hidb::HiDb& hidb, const hidb::Serum& aSerum, const argc_argv& args, bool aReportTables);
static void report_antigen(const hidb::HiDb& hidb, size_t aIndex, const argc_argv& args, bool aReportTables, std::string aPrefix = {});
static void report_antigen(const hidb::HiDb& hidb, const hidb::Antigen& aAntigen, const argc_argv& args, bool aReportTables, std::string aPrefix = {});
static void list_all_tables(const hidb::HiDb& hidb, const argc_argv& args);
static void report_tables(const hidb::HiDb& hidb, std::vector<size_t> aTables, const argc_argv& args, std::string aPrefix = {});
// static void report_table(const hidb::Table& aTable, const argc_argv& args, std::string aPrefix = {});
static void find_antigens(const hidb::HiDb& hidb, std::string aName, const argc_argv& args);
static void find_sera(const hidb::HiDb& hidb, std::string aName, const argc_argv& args);
static void find_tables(const hidb::HiDb& hidb, std::string aName, const argc_argv& args);
static void find(const hidb::HiDb& hidb, const argc_argv& args);

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {
                {"-s", false},  // find sera
                {"-t", false},  // find table
                {"--db-dir", ""},
                {"-v", false},
                {"--verbose", false},
                {"-h", false},
                {"--help", false},
            });
        if (args["-h"] || args["--help"] || args.number_of_arguments() < 2) {
            throw std::runtime_error("Usage: "s + args.program() + " [options] <virus-type: B, H1, H3|hidb-file> <name|all> ...\n" + args.usage_options());
        }
        const bool verbose = args["-v"] || args["--verbose"];
        hidb::setup(args["--db-dir"], {}, verbose);

        if (fs::exists(args[0]))
            find(hidb::HiDb(args[0], report_time::Yes), args);
        else
            find(hidb::get(string::upper(args[0]), report_time::Yes), args);
        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        return 1;
    }
}

// ----------------------------------------------------------------------

void find(const hidb::HiDb& hidb, const argc_argv& args)
{
    if (args[1] == "all"s) {
        if (args["-s"])
            list_all_sera(hidb, args);
        else if (args["-t"])
            list_all_tables(hidb, args);
        else
            list_all_antigens(hidb, args);
    }
    else {
        if (args["-s"])
            find_sera(hidb, args[1], args);
        else if (args["-t"])
            find_tables(hidb, args[1], args);
        else
            find_antigens(hidb, args[1], args);
    }

} // find

// ----------------------------------------------------------------------

void find_antigens(const hidb::HiDb& hidb, std::string aName, const argc_argv& args)
{
    const auto indexes = hidb.antigens()->find(string::upper(aName));
    for (auto index: indexes)
        report_antigen(hidb, index, args, true);

} // find_antigens

// ----------------------------------------------------------------------

void find_sera(const hidb::HiDb& hidb, std::string aName, const argc_argv& args)
{

} // find_sera

// ----------------------------------------------------------------------

void find_tables(const hidb::HiDb& hidb, std::string aName, const argc_argv& args)
{

} // find_tables

// ----------------------------------------------------------------------

void list_all_tables(const hidb::HiDb& hidb, const argc_argv& /*args*/)
{
    auto tables = hidb.tables();
    std::cout << "Tables: " << tables->size() << '\n';
    for (auto table: *tables)
        std::cout << table->name() << " A:" << table->number_of_antigens() << " S:" << table->number_of_sera() << '\n';

} // list_all_tables

// ----------------------------------------------------------------------

void report_tables(const hidb::HiDb& hidb, std::vector<size_t> aTables, const argc_argv& /*args*/, std::string aPrefix)
{
    auto hidb_tables = hidb.tables();
    std::vector<std::shared_ptr<hidb::Table>> tables(aTables.size());
    std::transform(aTables.begin(), aTables.end(), tables.begin(), [hidb_tables](size_t aIndex) { return (*hidb_tables)[aIndex]; });
    if (tables.size() > 1) {
        std::sort(tables.begin(), tables.end(), [](auto a, auto b) -> bool { return a->date() > b->date(); });
        std::map<std::pair<std::string, std::string>, std::vector<std::shared_ptr<hidb::Table>>> by_lab_assay;
        for (auto table: tables)
            by_lab_assay[{table->lab(), table->assay()}].push_back(table);
        if (by_lab_assay.size() > 1)
            std::cout << aPrefix << "Tables:" << tables.size() << "  Recent: " << tables[0]->name() << '\n';
        for (auto entry: by_lab_assay) {
            std::cout << aPrefix << entry.first.first << ':' << entry.first.second << ' ' << entry.second.size();
            for (auto table: entry.second)
                std::cout << ' ' << string::join(":", {table->date(), table->rbc()});
        }
        std::cout << '\n';
    }
    else
        std::cout << aPrefix << "Tables:" << tables.size() << "  Recent: " << tables[0]->name() << '\n';

} // report_tables

// ----------------------------------------------------------------------

// void report_table(const hidb::Table& aTable, const argc_argv& /*args*/, std::string aPrefix)
// {
//     std::cout << aPrefix << aTable.name() << " A:" << aTable.number_of_antigens() << " S:" << aTable.number_of_sera() << '\n';

// } // report_table

// ----------------------------------------------------------------------

void list_all_antigens(const hidb::HiDb& hidb, const argc_argv& args)
{
    auto antigens = hidb.antigens();
    std::cout << "Antigens: " << antigens->size() << '\n';
    for (auto antigen: *antigens)
        report_antigen(hidb, dynamic_cast<const hidb::Antigen&>(*antigen), args, true);

} // list_all_antigens

// ----------------------------------------------------------------------

void report_antigen(const hidb::HiDb& hidb, size_t aIndex, const argc_argv& args, bool aReportTables, std::string aPrefix)
{
    report_antigen(hidb, dynamic_cast<const hidb::Antigen&>(*(*hidb.antigens())[aIndex]), args, aReportTables, aPrefix);

} // report_antigen

// ----------------------------------------------------------------------

void report_antigen(const hidb::HiDb& hidb, const hidb::Antigen& aAntigen, const argc_argv& args, bool aReportTables, std::string aPrefix)
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
            report_tables(hidb, aAntigen.tables(), args, aPrefix + "    ");
        }
        else
            std::cout << '\n';

} // report_antigen

// ----------------------------------------------------------------------

void list_all_sera(const hidb::HiDb& hidb, const argc_argv& args)
{
    auto sera = hidb.sera();
    std::cout << "Sera: " << sera->size() << '\n';
    for (auto serum: *sera)
        report_serum(hidb, dynamic_cast<const hidb::Serum&>(*serum), args, true);

} // list_all_sera

// ----------------------------------------------------------------------

void report_serum(const hidb::HiDb& hidb, const hidb::Serum& aSerum, const argc_argv& args, bool aReportTables)
{
    std::cout << aSerum.name();
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
        report_antigen(hidb, ag_no, args, false, "    ");
    if (aReportTables)
        report_tables(hidb, aSerum.tables(), args, "    ");

} // report_serum

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
