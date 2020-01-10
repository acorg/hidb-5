#include "acmacs-base/argv.hh"
#include "acmacs-base/stream.hh"
#include "hidb-5/hidb.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<bool> verbose{*this, 'v', "verbose"};

    argument<str> hidb_file{*this, arg_name{"hidb5.json.xz"}, mandatory};
};

int main(int argc, char* const argv[])
{
    try {
        Options opt(argc, argv);
        hidb::HiDb hidb(opt.hidb_file, opt.verbose);
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
        fmt::print("Dates: {} .. {}\nYears: {}\n", dates.front(), dates.back(), years);

        return 0;
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        return 1;
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
