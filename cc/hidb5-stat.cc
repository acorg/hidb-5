#include <cstdlib>
#include <tuple>
#include <map>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/string.hh"
#include "locationdb/locdb.hh"
#include "hidb-5/hidb.hh"

using namespace std::string_literals;

static void make();

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {
                {"--db-dir", ""},
                {"-v", false},
                {"--verbose", false},
                {"-h", false},
                {"--help", false},
            });
        if (args["-h"] || args["--help"] || args.number_of_arguments() != 1) {
            throw std::runtime_error("Usage: "s + args.program() + " [options] <output.json>\n" + args.usage_options());
        }
        const bool verbose = args["-v"] || args["--verbose"];
        hidb::setup(args["--db-dir"], {}, verbose);

        make();

        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        return 1;
    }
}

// ----------------------------------------------------------------------

void make()
{
    using key = std::tuple<std::string, std::string, std::string, std::string>; // virus_type, date, lab, continent
    std::map<key, size_t> data_antigens;
    auto& locdb = get_locdb();
    for (std::string virus_type: {"A(H1N1)", "A(H3N2)", "B"}) {
        const auto& hidb = hidb::get(virus_type, report_time::No);
        auto antigens = hidb.antigens();
        for (size_t ag_no = 0; ag_no < antigens->size(); ++ag_no) {
            auto antigen = antigens->at(ag_no);
            std::string date = antigen->date_compact();
            if (date.empty())
                date = antigen->year() + "9900";
            std::string continent = locdb.continent(std::string(antigen->location()), "UNKNOWN");
        }
    }

} // make

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
