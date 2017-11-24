#include "acmacs-base/argc-argv.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/report.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {
                {"-s", false},  // find sera
                {"-t", false},  // find table
                {"--labid", false},  // find by lab id
                {"--db-dir", ""},
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

        auto chart = acmacs::chart::import_factory(args[0], acmacs::chart::Verify::None);
        auto& hidb = hidb::get(chart->info()->virus_type());
        auto antigens = hidb.antigens()->find(*chart->antigens());
        for (auto ag: antigens) {
            if (ag)
                hidb::report_antigen(hidb, *ag, true);
        }
        std::cout << '\n';
        auto sera = hidb.sera()->find(*chart->sera());
        for (auto sr: sera) {
            if (sr)
                hidb::report_serum(hidb, *sr, true);
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
