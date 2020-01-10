#include "acmacs-base/argv.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/report.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<bool> sera_only{*this, "sera-only"};
    option<bool> first_table{*this, "first-table"};
    option<str> db_dir{*this, "db-dir"};
    option<bool> verbose{*this, 'v', "verbose"};

    argument<str> chart{*this, arg_name{"chart"}, mandatory};
};

int main(int argc, char* const argv[])
{
    try {
        Options opt(argc, argv);
        hidb::setup(opt.db_dir, {}, opt.verbose);

        auto chart = acmacs::chart::import_from_file(opt.chart);
        auto& hidb = hidb::get(chart->info()->virus_type());

        if (!opt.sera_only) {
            auto antigens = hidb.antigens()->find(*chart->antigens());
            for (auto ag: antigens) {
                if (ag)
                    hidb::report_antigen(hidb, *ag, true);
            }
            std::cout << '\n';
        }

        auto sera = hidb.sera()->find(*chart->sera());
        for (auto sr: sera) {
            if (sr)
                hidb::report_serum(hidb, *sr, opt.first_table ? hidb::report_tables::oldest : hidb::report_tables::all);
        }

        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        return 1;
    }
}

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
