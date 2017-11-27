#include <cstdlib>
#include <tuple>
#include <map>
#include <regex>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/string.hh"
#include "locationdb/locdb.hh"
#include "hidb-5/hidb.hh"

using namespace std::string_literals;

static void make(std::string aStart, std::string aEnd);
static std::string get_date(std::string aDate);

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {
                {"--start", "1000"},
                {"--end", "3000"},
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

        make(get_date(args["--start"]), get_date(args["--end"]));

        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        return 1;
    }
}

// ----------------------------------------------------------------------

void make(std::string aStart, std::string aEnd)
{
    using key = std::tuple<std::string, std::string, std::string, std::string>; // virus_type, lab, date, continent
    auto& locdb = get_locdb();

    std::map<key, size_t> data_antigens;
    std::string min_date{"3000"}, max_date{"1000"};
    for (std::string virus_type: {"A(H1N1)", "A(H3N2)", "B"}) {
        const auto& hidb = hidb::get(virus_type, report_time::No);
        auto antigens = hidb.antigens();
        auto tables = hidb.tables();
        for (size_t ag_no = 0; ag_no < antigens->size(); ++ag_no) {
            auto antigen = antigens->at(ag_no);
            std::string date = antigen->date_compact().substr(0, 6);
            if (date.empty())
                date = antigen->year() + "99";
            else if (date.size() == 4)
                date += "99";
            if (date >= aStart && date < aEnd) {
                std::string continent = locdb.continent(std::string(antigen->location()), "UNKNOWN");
                std::string year = date.substr(0, 4);
                std::string lab(hidb.lab(*antigen, *tables));

                ++data_antigens[std::make_tuple(virus_type, lab, date, continent)];
                ++data_antigens[std::make_tuple(virus_type, lab, year, continent)];
                ++data_antigens[std::make_tuple(virus_type, lab, date, "")];
                ++data_antigens[std::make_tuple(virus_type, lab, year, "")];
                ++data_antigens[std::make_tuple(virus_type, lab, "",   continent)];
                ++data_antigens[std::make_tuple(virus_type, lab, "",   "")];
                ++data_antigens[std::make_tuple(virus_type, "",  date, continent)];
                ++data_antigens[std::make_tuple(virus_type, "",  year, continent)];
                ++data_antigens[std::make_tuple(virus_type, "",  date, "")];
                ++data_antigens[std::make_tuple(virus_type, "",  year, "")];
                ++data_antigens[std::make_tuple(virus_type, "",  "",   continent)];
                ++data_antigens[std::make_tuple(virus_type, "",  "",   "")];
                ++data_antigens[std::make_tuple("",         lab, date, continent)];
                ++data_antigens[std::make_tuple("",         lab, year, continent)];
                ++data_antigens[std::make_tuple("",         lab, date, "")];
                ++data_antigens[std::make_tuple("",         lab, year, "")];
                ++data_antigens[std::make_tuple("",         lab, "",   continent)];
                ++data_antigens[std::make_tuple("",         lab, "",   "")];
                ++data_antigens[std::make_tuple("",         "",  date, continent)];
                ++data_antigens[std::make_tuple("",         "",  year, continent)];
                ++data_antigens[std::make_tuple("",         "",  date, "")];
                ++data_antigens[std::make_tuple("",         "",  year, "")];
                ++data_antigens[std::make_tuple("",         "",  "",   continent)];
                ++data_antigens[std::make_tuple("",         "",  "",   "")];

                if (virus_type == "B") {
                    std::string vtl;
                    switch (antigen->lineage()) {
                      case acmacs::chart::BLineage::Victoria:
                          vtl = "BVICTORIA";
                          break;
                      case acmacs::chart::BLineage::Yamagata:
                          vtl = "BYAMAGATA";
                          break;
                      case acmacs::chart::BLineage::Unknown:
                          vtl = "BUNKNOWN";
                          break;
                    }
                    ++data_antigens[std::make_tuple(vtl, lab, date, continent)];
                    ++data_antigens[std::make_tuple(vtl, lab, year, continent)];
                    ++data_antigens[std::make_tuple(vtl, lab, date, "")];
                    ++data_antigens[std::make_tuple(vtl, lab, year, "")];
                    ++data_antigens[std::make_tuple(vtl, lab, "",   continent)];
                    ++data_antigens[std::make_tuple(vtl, lab, "",   "")];
                    ++data_antigens[std::make_tuple(vtl, "",  date, continent)];
                    ++data_antigens[std::make_tuple(vtl, "",  year, continent)];
                    ++data_antigens[std::make_tuple(vtl, "",  date, "")];
                    ++data_antigens[std::make_tuple(vtl, "",  year, "")];
                    ++data_antigens[std::make_tuple(vtl, "",  "",   continent)];
                    ++data_antigens[std::make_tuple(vtl, "",  "",   "")];
                }

                min_date = std::min(min_date, date);
                max_date = std::max(max_date, date);
            }
        }
    }

    std::cout << "Dates: " << min_date << " - " << max_date << '\n';
    for (std::string virus_type: {"A(H1N1)", "A(H3N2)", "B"}) {
        std::cout << std::setw(9) << std::left << virus_type << ": " << data_antigens[std::make_tuple(virus_type, "",  "",   "")] << '\n';
        if (virus_type == "B") {
            for (std::string lineage: {"VICTORIA", "YAMAGATA", "UNKNOWN"}) {
                auto vtl = virus_type + lineage;
                std::cout << std::setw(9) << std::left << vtl << ": " << data_antigens[std::make_tuple(vtl, "",  "",   "")] << '\n';
            }
        }
    }

} // make

// ----------------------------------------------------------------------

std::string get_date(std::string aDate)
{
    std::smatch m;
    if (std::regex_match(aDate, m, std::regex("(19[2-9][0-9]|20[01][0-9]|1000|3000)-?(0[0-9]|1[0-2])?(-?[0-3][0-9])?"))) {
        return m[1].str() + m[2].str();
    }
    throw std::runtime_error("unrecognized date " + aDate);

} // get_date

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
