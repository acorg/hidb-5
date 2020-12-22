#include <iostream>

#include "acmacs-base/argv.hh"
#include "acmacs-chart-2/factory-import.hh"

#include "hidb-maker.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    // option<bool> report_time{*this, "time", desc{"report time of loading chart"}};

    argument<str> output_hidb{*this, arg_name{"hidb5.json.xz"}, mandatory};
    argument<str_array> charts{*this, arg_name{"input-chart-file"}, mandatory};
};

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        Options opt(argc, argv);
        HidbMaker maker;
        for (const auto& source : *opt.charts) {
            auto chart = acmacs::chart::import_from_file(source); // , acmacs::chart::Verify::All, do_report_time(opt.report_time));
            maker.add(*chart);
        }
        maker.save(opt.output_hidb);
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
