#include "acmacs-base/argv.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/vaccines.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<str> db_dir{*this, "db-dir"};

    argument<str> chart_file{*this, arg_name{"chart"}, mandatory};
};

int main(int argc, char* const argv[])
{
    try {
        Options opt(argc, argv);
        hidb::setup(opt.db_dir);

        auto chart = acmacs::chart::import_from_file(opt.chart_file);
        if (chart->info()->virus_type(acmacs::chart::Info::Compute::Yes).empty())
            throw std::runtime_error("chart has no virus_type");
        auto vaccines = hidb::vaccines(*chart);
        fmt::print("{}\n", vaccines.report(hidb::Vaccines::ReportConfig{}.vaccine_sep("\n").show_no(false)));
        return 0;
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        return 1;
    }
}

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
