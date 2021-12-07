#include <cstdlib>
#include <cmath>

#include "acmacs-base/argv.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-base/filesystem.hh"
#include "locationdb/locdb.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/report.hh"

// ----------------------------------------------------------------------

struct Options;

static void list_all_antigens(const hidb::HiDb& hidb);
static void list_all_sera(const hidb::HiDb& hidb, const Options& opt);
static void list_all_tables(const hidb::HiDb& hidb);
static void find_antigens(const hidb::HiDb& hidb, std::string_view aName);
static void find_antigens_by_labid(const hidb::HiDb& hidb, std::string_view aLabId);
static void find_sera(const hidb::HiDb& hidb, std::string_view aName);
[[noreturn]] static void find_tables(const hidb::HiDb& hidb, std::string_view aName);
static void find(const hidb::HiDb& hidb, const Options& opt);

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<bool> find_sera{*this, 's', desc{"find sera"}};
    option<bool> find_table{*this, 't', desc{"find table"}};
    option<bool> first_table{*this, "first-table"};
    option<str>  lab{*this, "lab"};
    option<bool> find_by_lab_id{*this, "lab-id", desc{"find by lab id"}};
    // option<str>  db_dir{*this, "db-dir"};

    argument<str> virus_type{*this, arg_name{"virus-type: B, H1, H3|hidb-file"}, mandatory};
    argument<str_array> names{*this, arg_name{"name|all"}, mandatory};
};

int main(int argc, char* const argv[])
{
    try {
        Options opt(argc, argv);
        // hidb::setup(opt.db_dir);

        if (fs::is_regular_file(*opt.virus_type))
            find(hidb::HiDb(opt.virus_type), opt);
        else
            find(hidb::get(acmacs::virus::type_subtype_t{string::upper(*opt.virus_type)}, report_time::no), opt);
        return 0;
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        return 1;
    }
}

// ----------------------------------------------------------------------

void find(const hidb::HiDb& hidb, const Options& opt)
{
    if (opt.names->at(0) == "all") {
        if (opt.find_sera)
            list_all_sera(hidb, opt);
        else if (opt.find_table)
            list_all_tables(hidb);
        else
            list_all_antigens(hidb);
    }
    else {
        for (const auto& name : *opt.names) {
            if (opt.find_sera)
                find_sera(hidb, name);
            else if (opt.find_table)
                find_tables(hidb, name);
            else if (opt.find_by_lab_id)
                find_antigens_by_labid(hidb, name);
            else
                find_antigens(hidb, name);
        }
    }

} // find

// ----------------------------------------------------------------------

void find_antigens(const hidb::HiDb& hidb, std::string_view aName)
{
    try {
        std::string prefix;
        auto antigen_index_list = hidb.antigens()->find(string::upper(aName), hidb::fix_location::yes, hidb::find_fuzzy::no);
        if (antigen_index_list.empty()) {
            antigen_index_list = hidb.antigens()->find(string::upper(aName), hidb::fix_location::yes, hidb::find_fuzzy::yes);
            prefix = "*** ";
        }
        for (auto antigen_index: antigen_index_list)
            report_antigen(hidb, *hidb.antigens()->at(antigen_index), hidb::report_tables::all, prefix);
    }
    catch (acmacs::locationdb::LocationNotFound& err) {
        throw std::runtime_error(fmt::format("location not found: {}", err));
    }

} // find_antigens

// ----------------------------------------------------------------------

void find_antigens_by_labid(const hidb::HiDb& hidb, std::string_view aLabId)
{
    const auto antigens = hidb.antigens()->find_labid(string::upper(aLabId));
    for (auto antigen: antigens)
        report_antigen(hidb, *antigen, hidb::report_tables::all);

} // find_antigens_by_labid

// ----------------------------------------------------------------------

void find_sera(const hidb::HiDb& hidb, std::string_view aName)
{
    try {
        std::string prefix;
        auto serum_index_list = hidb.sera()->find(string::upper(aName), hidb::fix_location::yes, hidb::find_fuzzy::no);
        if (serum_index_list.empty()) {
            serum_index_list = hidb.sera()->find(string::upper(aName), hidb::fix_location::yes, hidb::find_fuzzy::yes);
            prefix = "*** ";
        }
        for (auto serum_index: serum_index_list)
            report_serum(hidb, *hidb.sera()->at(serum_index), hidb::report_tables::all);
    }
    catch (acmacs::locationdb::LocationNotFound& err) {
        throw std::runtime_error(fmt::format("location not found: {}", err));
    }

} // find_sera

// ----------------------------------------------------------------------

void find_tables(const hidb::HiDb& /*hidb*/, std::string_view /*aName*/)
{
    throw std::runtime_error("Not implemented");

} // find_tables

// ----------------------------------------------------------------------

void list_all_tables(const hidb::HiDb& hidb)
{
    auto tables = hidb.tables();
    fmt::print("Tables: {}\n", tables->size());
    for (auto table: *tables)
        fmt::print("{} A:{} S:{}\n", table->name(), table->number_of_antigens(), table->number_of_sera());

} // list_all_tables

// ----------------------------------------------------------------------

void list_all_antigens(const hidb::HiDb& hidb)
{
    auto antigens = hidb.antigens();
    fmt::print("Antigens: {}\n", antigens->size());
    for (auto antigen: *antigens)
        report_antigen(hidb, dynamic_cast<const hidb::Antigen&>(*antigen), hidb::report_tables::all);

} // list_all_antigens

// ----------------------------------------------------------------------

void list_all_sera(const hidb::HiDb& hidb, const Options& opt)
{
    auto sera = hidb.sera();
    auto tables = hidb.tables();
    auto has_lab = [tables,lab=*opt.lab](const auto& sr) -> bool {
                       if (lab.empty())
                           return true;
                       const auto labs = sr.labs(*tables);
                       return std::find(labs.begin(), labs.end(), lab) != labs.end();
                   };

    fmt::print("Sera: {}\n", sera->size());
    for (auto serum: *sera) {
        const auto& sr = dynamic_cast<const hidb::Serum&>(*serum);
        if (has_lab(sr))
            report_serum(hidb, sr, opt.first_table ? hidb::report_tables::oldest : hidb::report_tables::all);
    }

} // list_all_sera

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
