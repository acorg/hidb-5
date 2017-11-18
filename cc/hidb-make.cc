#include <iostream>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-chart-2/factory-import.hh"

#include "hidb-maker.hh"

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        argc_argv args(argc, argv, {
                {"-h", false},
                {"--help", false},
                {"-v", false},
                {"--verbose", false}
        });
        if (args["-h"] || args["--help"] || args.number_of_arguments() < 2) {
            std::cerr << "Usage: " << args.program() << " [options] <hidb5.json.xz> <input-chart-file> ...\n" << args.usage_options() << '\n';
            exit_code = 1;
        }
        else {
            HidbMaker maker;
            for (size_t arg_no = 1; arg_no < args.number_of_arguments(); ++arg_no) {
                auto chart = acmacs::chart::import_factory(args[arg_no], acmacs::chart::Verify::All);
                maker.add(*chart);
            }
            maker.save(args[0]);
        }
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
