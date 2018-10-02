#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/stream.hh"
#include "hidb-5/hidb.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {
                {"-v", false},
                {"--verbose", false},
                {"-h", false},
                {"--help", false},
            });
        if (args["-h"] || args["--help"] || args.number_of_arguments() != 1) {
            throw std::runtime_error("Usage: "s + args.program() + " [options] <hidb5.json.xz>\n" + args.usage_options());
        }
        const bool verbose = args["-v"] || args["--verbose"];

        hidb::HiDb hidb(std::string(args[0]), verbose);
        auto antigens = hidb.antigens();
        std::vector<std::string> dates;
        std::map<std::string, size_t> years;
        for (size_t ag_no = 0; ag_no < antigens->size(); ++ag_no) {
            auto antigen = antigens->at(ag_no);
            const std::string date = antigen->date_compact().substr(0, 6);
            if (!date.empty()) {
                dates.push_back(date);
                ++years[date.substr(0, 4)];
            }
        }
        std::sort(dates.begin(), dates.end());
          // std::cout << "N: " << dates.size() << '\n';
        std::cout << "Dates: " << dates.front() << " .. " << dates.back() << '\n';
        std::cout << "Years: " << years << '\n';

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
