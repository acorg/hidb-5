#include <cstdlib>
#include <cmath>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-base/filesystem.hh"
#include "locationdb/locdb.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/report.hh"

using namespace std::string_literals;

static void list_all_antigens(const hidb::HiDb& hidb, const argc_argv& args);
static void list_all_sera(const hidb::HiDb& hidb, const argc_argv& args);
static void list_all_tables(const hidb::HiDb& hidb, const argc_argv& args);
static void find_antigens(const hidb::HiDb& hidb, std::string aName, const argc_argv& args);
static void find_antigens_by_labid(const hidb::HiDb& hidb, std::string aLabId, const argc_argv& args);
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
                {"--first-table", false},
                {"--lab", ""},
                {"--labid", false},  // find by lab id
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
        hidb::setup(std::string(args["--db-dir"]), {}, verbose);

        if (fs::exists(args[0]))
            find(hidb::HiDb(std::string(args[0]), verbose), args);
        else
            find(hidb::get(acmacs::virus::type_subtype_t{args[0]}, report_time::yes), args);
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
            find_sera(hidb, std::string{args[1]}, args);
        else if (args["-t"])
            find_tables(hidb, std::string{args[1]}, args);
        else if (args["--labid"])
            find_antigens_by_labid(hidb, std::string{args[1]}, args);
        else
            find_antigens(hidb, std::string{args[1]}, args);
    }

} // find

// ----------------------------------------------------------------------

void find_antigens(const hidb::HiDb& hidb, std::string aName, const argc_argv& /*args*/)
{
    try {
        std::string prefix;
        auto antigen_index_list = hidb.antigens()->find(string::upper(aName), hidb::fix_location::yes, hidb::find_fuzzy::no);
        if (antigen_index_list.empty()) {
            antigen_index_list = hidb.antigens()->find(string::upper(aName), hidb::fix_location::yes, hidb::find_fuzzy::yes);
            prefix = "*** ";
        }
        for (auto antigen_index: antigen_index_list)
            report_antigen(hidb, *antigen_index.first, true, prefix);
    }
    catch (LocationNotFound& err) {
        throw std::runtime_error("location not found: "s + err.what());
    }

} // find_antigens

// ----------------------------------------------------------------------

void find_antigens_by_labid(const hidb::HiDb& hidb, std::string aLabId, const argc_argv& /*args*/)
{
    const auto antigens = hidb.antigens()->find_labid(string::upper(aLabId));
    for (auto antigen: antigens)
        report_antigen(hidb, *antigen, true);

} // find_antigens_by_labid

// ----------------------------------------------------------------------

void find_sera(const hidb::HiDb& hidb, std::string aName, const argc_argv& /*args*/)
{
    try {
        std::string prefix;
        auto serum_index_list = hidb.sera()->find(string::upper(aName), hidb::fix_location::yes, hidb::find_fuzzy::no);
        if (serum_index_list.empty()) {
            serum_index_list = hidb.sera()->find(string::upper(aName), hidb::fix_location::yes, hidb::find_fuzzy::yes);
            prefix = "*** ";
        }
        for (auto serum_index: serum_index_list)
            report_serum(hidb, *serum_index.first, hidb::report_tables::all);
    }
    catch (LocationNotFound& err) {
        throw std::runtime_error("location not found: "s + err.what());
    }

} // find_sera

// ----------------------------------------------------------------------

void find_tables(const hidb::HiDb& /*hidb*/, std::string /*aName*/, const argc_argv& /*args*/)
{
    throw std::runtime_error("Not implemented");

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

void list_all_antigens(const hidb::HiDb& hidb, const argc_argv& /*args*/)
{
    auto antigens = hidb.antigens();
    std::cout << "Antigens: " << antigens->size() << '\n';
    for (auto antigen: *antigens)
        report_antigen(hidb, dynamic_cast<const hidb::Antigen&>(*antigen), true);

} // list_all_antigens

// ----------------------------------------------------------------------

void list_all_sera(const hidb::HiDb& hidb, const argc_argv& args)
{
    const std::string lab(args["--lab"]);
    const bool first_table = args["--first-table"];
    auto sera = hidb.sera();
    auto tables = hidb.tables();
    auto has_lab = [tables,&lab](const auto& sr) -> bool {
                       if (lab.empty())
                           return true;
                       const auto labs = sr.labs(*tables);
                       return std::find(labs.begin(), labs.end(), lab) != labs.end();
                   };

    std::cout << "Sera: " << sera->size() << '\n';
    for (auto serum: *sera) {
        const auto& sr = dynamic_cast<const hidb::Serum&>(*serum);
        if (has_lab(sr))
            report_serum(hidb, sr, first_table ? hidb::report_tables::oldest : hidb::report_tables::all);
    }

} // list_all_sera

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
