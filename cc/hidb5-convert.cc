#include <cstdlib>
#include <cmath>

#include "acmacs-base/argv.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/enumerate.hh"
#include "hidb-5/hidb.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<bool> report_time{*this, "time", desc{"report time of loading chart"}};

    argument<str> source{*this, arg_name{"hidb5.json.xz"}, mandatory};
    argument<str> output{*this, arg_name{"output.hidb5b"}, mandatory};
};

int main(int argc, char* const argv[])
{
    try {
        Options opt(argc, argv);
        hidb::HiDb hidb(opt.source);
        hidb.save(opt.output);
        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        return 1;
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
