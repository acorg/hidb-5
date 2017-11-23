#include <cstdlib>
#include <cmath>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/enumerate.hh"
#include "hidb-5/hidb.hh"

using namespace std::string_literals;

static void list_all_antigens(const hidb::HiDb& hidb, const argc_argv& args);
static void list_all_sera(const hidb::HiDb& hidb, const argc_argv& args);
static void report_serum(const hidb::HiDb& hidb, const hidb::Serum& aSerum, const argc_argv& args, bool aReportTables);
static void report_antigen(const hidb::HiDb& hidb, size_t aIndex, const argc_argv& args, bool aReportTables, std::string aPrefix = {});
static void report_antigen(const hidb::HiDb& hidb, const hidb::Antigen& aAntigen, const argc_argv& args, bool aReportTables, std::string aPrefix = {});
static void list_all_tables(const hidb::HiDb& hidb, const argc_argv& args);
static void report_tables(const hidb::HiDb& hidb, std::vector<size_t> aTables, const argc_argv& args, std::string aPrefix = {});
static void report_table(const hidb::Table& aTable, const argc_argv& args, std::string aPrefix = {});

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
            throw std::runtime_error("Usage: "s + args.program() + " [options] <virus-type: B, H1, H3> <name|all> ...\n" + args.usage_options());
        }
        const bool verbose = args["-v"] || args["--verbose"];
        hidb::setup(args["--db-dir"], {}, verbose);

        const auto& hidb = hidb::get(string::upper(args[0]), report_time::Yes);
        if (args[1] == "all"s) {
            if (args["-s"])
                list_all_sera(hidb, args);
            else if (args["-t"])
                list_all_tables(hidb, args);
            else
                list_all_antigens(hidb, args);
        }

        // for (auto arg = 2; arg < argc; ++arg) {
        //     Timeit timeit("looking: ");
        //     const auto look_for = string::upper(argv[arg]);
        //     const auto results = hidb.find_antigens_with_score(look_for);
        //     if (!results.empty()) {
        //         const auto num_digits = static_cast<int>(std::log10(results.size())) + 1;
        //         size_t result_no = 1;
        //         for (const auto& result: results) {
        //             std::cout << std::setw(num_digits) << result_no << ' ' << result.second << ' ' << result.first->full_name() << '\n';
        //             ++result_no;
        //         }
        //         if ((arg + 1) < argc)
        //             std::cout << '\n';
        //     }
        // }

        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        return 1;
    }
}

// ----------------------------------------------------------------------

void list_all_tables(const hidb::HiDb& hidb, const argc_argv& args)
{
    auto tables = hidb.tables();
    std::cout << "Tables: " << tables->size() << '\n';
    for (auto table: *tables)
        report_table(*table, args);

} // list_all_tables

// ----------------------------------------------------------------------

void report_tables(const hidb::HiDb& hidb, std::vector<size_t> aTables, const argc_argv& args, std::string aPrefix)
{
    for (size_t t_no: aTables) {
        report_table(*(*hidb.tables())[t_no], args, aPrefix);
    }

} // report_tables

// ----------------------------------------------------------------------

void report_table(const hidb::Table& aTable, const argc_argv& args, std::string aPrefix)
{
    std::cout << aPrefix << aTable.name() << " A:" << aTable.number_of_antigens() << " S:" << aTable.number_of_sera() << '\n';

} // report_table

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
