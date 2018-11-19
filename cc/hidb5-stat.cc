#include <cstdlib>
#include <tuple>
#include <map>
#include <set>
#include <regex>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/rjson.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/time.hh"
#include "locationdb/locdb.hh"
#include "hidb-5/hidb.hh"

using namespace std::string_literals;

using data_key_t = std::tuple<std::string, std::string, std::string, std::string>; // virus_type, lab, date, continent
using data_t = std::map<data_key_t, size_t>;

static void make(std::string aStart, std::string aEnd, std::string_view aFilename);
static data_t scan_antigens(std::string aStart, std::string aEnd);
static std::pair<data_t, data_t> scan_sera(std::string aStart, std::string aEnd);
static void update(data_t& data, std::string virus_type, std::string lab, std::string date, std::string continent, acmacs::chart::BLineage lineage, std::string full_name);
static void report(const data_t& data, std::string name);
static std::string make_json(const data_t& data_antigens, const data_t& data_sera, const data_t& data_sera_unique);
static std::string get_date(std::string_view aDate);

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
        hidb::setup(std::string(args["--db-dir"]), {}, verbose);

        make(get_date(std::string(args["--start"])), get_date(std::string(args["--end"])), std::string(args[0]));

        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        return 1;
    }
}

// ----------------------------------------------------------------------

void make(std::string aStart, std::string aEnd, std::string_view aFilename)
{
    auto data_antigens = scan_antigens(aStart, aEnd);
    auto [data_sera, data_sera_unique] = scan_sera(aStart, aEnd);
    acmacs::file::write(aFilename, make_json(data_antigens, data_sera, data_sera_unique));

    report(data_antigens, "Antigens");
    report(data_sera, "Sera");
    report(data_sera_unique, "Sera unique");

    // std::cout << "\nAntigens:\n";
    // for (std::string virus_type: {"A(H1N1)", "A(H3N2)", "B"}) {
    //     std::cout << std::setw(9) << std::left << virus_type << ": " << data_antigens[std::make_tuple(virus_type, "all",  "all",   "all")] << '\n';
    //     if (virus_type == "B") {
    //         for (std::string lineage: {"VICTORIA", "YAMAGATA", "UNKNOWN"}) {
    //             auto vtl = virus_type + lineage;
    //             std::cout << "  " << std::setw(9) << std::left << vtl << ": " << data_antigens[std::make_tuple(vtl, "all",  "all",   "all")] << '\n';
    //         }
    //     }
    // }

    // std::cout << "\nSera:\n";
    // for (std::string virus_type: {"A(H1N1)", "A(H3N2)", "B"}) {
    //     std::cout << std::setw(9) << std::left << virus_type << ": " << data_sera[std::make_tuple(virus_type, "all",  "all",   "all")] << '\n';
    //     if (virus_type == "B") {
    //         for (std::string lineage: {"VICTORIA", "YAMAGATA", "UNKNOWN"}) {
    //             auto vtl = virus_type + lineage;
    //             std::cout << "  " << std::setw(9) << std::left << vtl << ": " << data_sera[std::make_tuple(vtl, "all",  "all",   "all")] << '\n';
    //         }
    //     }
    // }

} // make

// ----------------------------------------------------------------------

void report(const data_t& data, std::string name)
{
    std::cout << '\n' << name << ":\n";
    for (std::string virus_type: {"A(H1N1)", "A(H3N2)", "B"}) {
        if (const auto found = data.find(std::make_tuple(virus_type, "all",  "all",   "all")); found != data.end()) {
            std::cout << "  " << std::setw(9) << std::left << virus_type << ": " << found->second << '\n';
            if (virus_type == "B") {
                for (std::string lineage: {"VICTORIA", "YAMAGATA", "UNKNOWN"}) {
                    const auto vtl = virus_type + lineage;
                    if (const auto found2 = data.find(std::make_tuple(vtl, "all",  "all",   "all")); found2 != data.end())
                        std::cout << "  " << std::setw(9) << std::left << vtl << ": " << found2->second << '\n';
                }
            }
        }
    }

} // report

// ----------------------------------------------------------------------

void update(data_t& data, std::string virus_type, std::string lab, std::string date, std::string continent, acmacs::chart::BLineage lineage, std::string full_name)
{
    const std::string all = "all";
    std::string year = date.substr(0, 4);

    ++data[std::make_tuple(virus_type, lab, date, continent)];
    ++data[std::make_tuple(virus_type, lab, year, continent)];
    ++data[std::make_tuple(virus_type, lab, date, all)];
    ++data[std::make_tuple(virus_type, lab, year, all)];
    ++data[std::make_tuple(virus_type, lab, all,  continent)];
    ++data[std::make_tuple(virus_type, lab, all,  all)];
    ++data[std::make_tuple(virus_type, all, date, continent)];
    ++data[std::make_tuple(virus_type, all, year, continent)];
    ++data[std::make_tuple(virus_type, all, date, all)];
    ++data[std::make_tuple(virus_type, all, year, all)];
    ++data[std::make_tuple(virus_type, all, all,  continent)];
    ++data[std::make_tuple(virus_type, all, all,  all)];
    ++data[std::make_tuple(all,        lab, date, continent)];
    ++data[std::make_tuple(all,        lab, year, continent)];
    ++data[std::make_tuple(all,        lab, date, all)];
    ++data[std::make_tuple(all,        lab, year, all)];
    ++data[std::make_tuple(all,        lab, all,  continent)];
    ++data[std::make_tuple(all,        lab, all,  all)];
    ++data[std::make_tuple(all,        all, date, continent)];
    ++data[std::make_tuple(all,        all, year, continent)];
    ++data[std::make_tuple(all,        all, date, all)];
    ++data[std::make_tuple(all,        all, year, all)];
    ++data[std::make_tuple(all,        all, all,  continent)];
    ++data[std::make_tuple(all,        all, all,  all)];

    if (virus_type == "B") {
        std::string vtl;
        switch (lineage) {
          case acmacs::chart::BLineage::Victoria:
              vtl = "BVICTORIA";
              break;
          case acmacs::chart::BLineage::Yamagata:
              vtl = "BYAMAGATA";
              break;
          case acmacs::chart::BLineage::Unknown:
              vtl = "BUNKNOWN";
              std::cerr << "WARNING: no lineage for " << full_name << '\n';
              break;
        }
        ++data[std::make_tuple(vtl, lab, date, continent)];
        ++data[std::make_tuple(vtl, lab, year, continent)];
        ++data[std::make_tuple(vtl, lab, date, all)];
        ++data[std::make_tuple(vtl, lab, year, all)];
        ++data[std::make_tuple(vtl, lab, all,  continent)];
        ++data[std::make_tuple(vtl, lab, all,  all)];
        ++data[std::make_tuple(vtl, all, date, continent)];
        ++data[std::make_tuple(vtl, all, year, continent)];
        ++data[std::make_tuple(vtl, all, date, all)];
        ++data[std::make_tuple(vtl, all, year, all)];
        ++data[std::make_tuple(vtl, all, all,  continent)];
        ++data[std::make_tuple(vtl, all, all,  all)];
    }

} // update

// ----------------------------------------------------------------------

data_t scan_antigens(std::string aStart, std::string aEnd)
{
    auto& locdb = get_locdb();

    data_t data_antigens;
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
                update(data_antigens, virus_type, std::string(hidb.lab(*antigen, *tables)), date, locdb.continent(std::string(antigen->location()), "UNKNOWN"), antigen->lineage(), antigen->full_name());
                min_date = std::min(min_date, date);
                max_date = std::max(max_date, date);
            }
        }
    }
    std::cout << "Antigen dates: " << min_date << " - " << max_date << '\n';

    return data_antigens;

} // scan_antigens

// ----------------------------------------------------------------------

std::pair<data_t, data_t> scan_sera(std::string aStart, std::string aEnd)
{
    auto& locdb = get_locdb();
    const std::string all = "all";

    data_t data_sera, data_sera_unique;

    std::string min_date{"3000"}, max_date{"1000"};
    for (std::string virus_type: {"A(H1N1)", "A(H3N2)", "B"}) {
        const auto& hidb = hidb::get(virus_type, report_time::No);
        auto sera = hidb.sera();
        auto antigens = hidb.antigens();
        auto tables = hidb.tables();
        std::set<std::string> names;
        for (size_t sr_no = 0; sr_no < sera->size(); ++sr_no) {
            auto serum = sera->at(sr_no);

            std::string date;
            for (auto ag_no: serum->homologous_antigens()) {
                date = antigens->at(ag_no)->date_compact().substr(0, 6);
                if (!date.empty())
                    break;
            }
            if (date.empty())
                date = serum->year() + "99";
            else if (date.size() == 4)
                date += "99";

            if (date >= aStart && date < aEnd) {
                update(data_sera_unique, virus_type, std::string(hidb.lab(*serum, *tables)), date, locdb.continent(std::string(serum->location()), "UNKNOWN"), serum->lineage(), serum->full_name());
                const auto name = serum->name();
                if (names.find(name) == names.end()) {
                    names.insert(name);
                    update(data_sera, virus_type, std::string(hidb.lab(*serum, *tables)), date, locdb.continent(std::string(serum->location()), "UNKNOWN"), serum->lineage(), serum->full_name());
                }
                min_date = std::min(min_date, date);
                max_date = std::max(max_date, date);
            }
        }
    }
    std::cout << "Serum dates: " << min_date << " - " << max_date << '\n';

    return {data_sera, data_sera_unique};

} // scan_sera

// ----------------------------------------------------------------------

std::string make_json(const data_t& data_antigens, const data_t& data_sera, const data_t& data_sera_unique)
{
    rjson::value data{rjson::object{{"antigens", rjson::object{}}, {"sera", rjson::object{}}, {"sera_unique", rjson::object{}}, {"date", acmacs::date_format()}}};

    for (auto [key, value] : data_antigens) {
        const auto [virus_type, lab, date, continent] = key;
        data.set("antigens", virus_type, lab, date, continent) = value;
    }

    for (auto [key, value] : data_sera) {
        const auto [virus_type, lab, date, continent] = key;
        data.set("sera", virus_type, lab, date, continent) = value;
    }

    for (auto [key, value] : data_sera_unique) {
        const auto [virus_type, lab, date, continent] = key;
        data.set("sera_unique", virus_type, lab, date, continent) = value;
    }

    return rjson::pretty(data, rjson::emacs_indent::yes, rjson::PrettyHandler(1));

} // make_json

// ----------------------------------------------------------------------

std::string get_date(std::string_view aDate)
{
    std::smatch m;
    std::string date(aDate);
    if (std::regex_match(date, m, std::regex("(19[2-9][0-9]|20[01][0-9]|1000|3000)-?(0[0-9]|1[0-2])?(-?[0-3][0-9])?"))) {
        return m[1].str() + m[2].str();
    }
    throw std::runtime_error("unrecognized date " + date);

} // get_date

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
