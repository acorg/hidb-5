#include "acmacs-base/argc-argv.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/vaccines.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {
                {"--db-dir", ""},
                {"--time", false, "report time of loading chart"},
                {"-v", false},
                {"--verbose", false},
                {"-h", false},
                {"--help", false},
            });
        if (args["-h"] || args["--help"] || args.number_of_arguments() != 1) {
            throw std::runtime_error("Usage: "s + args.program() + " [options] <chart>\n" + args.usage_options());
        }
        const bool verbose = args["-v"] || args["--verbose"];
        hidb::setup(args["--db-dir"], {}, verbose);

        auto chart = acmacs::chart::import_from_file(args[0], acmacs::chart::Verify::None, args["--time"] ? report_time::Yes : report_time::No);
        if (chart->info()->virus_type(acmacs::chart::Info::Compute::Yes).empty())
            throw std::runtime_error("chart has no virus_type");
        auto vaccines = hidb::vaccines(*chart, verbose);
        std::cout << vaccines.report(hidb::Vaccines::ReportConfig{}.vaccine_sep("\n").show_no(false)) << '\n';
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
