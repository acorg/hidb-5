#include <cstdlib>
#include <tuple>
#include <map>
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

static void make(std::string aStart, std::string aEnd, std::string aFilename);
static data_t scan_antigens(std::string aStart, std::string aEnd);
static std::pair<data_t, data_t> scan_sera(std::string aStart, std::string aEnd);
static std::string make_json(const data_t& data_antigens, const data_t& data_sera, const data_t& data_sera_unique);
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

        make(get_date(args["--start"]), get_date(args["--end"]), args[0]);

        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        return 1;
    }
}

// ----------------------------------------------------------------------

void make(std::string aStart, std::string aEnd, std::string aFilename)
{
    auto data_antigens = scan_antigens(aStart, aEnd);
    auto [data_sera, data_sera_unique] = scan_sera(aStart, aEnd);
    acmacs::file::write(aFilename, make_json(data_antigens, data_sera, data_sera_unique));

    std::cout << "\nAntigens:\n";
    for (std::string virus_type: {"A(H1N1)", "A(H3N2)", "B"}) {
        std::cout << std::setw(9) << std::left << virus_type << ": " << data_antigens[std::make_tuple(virus_type, "all",  "all",   "all")] << '\n';
        if (virus_type == "B") {
            for (std::string lineage: {"VICTORIA", "YAMAGATA", "UNKNOWN"}) {
                auto vtl = virus_type + lineage;
                std::cout << std::setw(9) << std::left << vtl << ": " << data_antigens[std::make_tuple(vtl, "all",  "all",   "all")] << '\n';
            }
        }
    }

    std::cout << "\nSera:\n";
    for (std::string virus_type: {"A(H1N1)", "A(H3N2)", "B"}) {
        std::cout << std::setw(9) << std::left << virus_type << ": " << data_sera[std::make_tuple(virus_type, "all",  "all",   "all")] << '\n';
        if (virus_type == "B") {
            for (std::string lineage: {"VICTORIA", "YAMAGATA", "UNKNOWN"}) {
                auto vtl = virus_type + lineage;
                std::cout << std::setw(9) << std::left << vtl << ": " << data_sera[std::make_tuple(vtl, "all",  "all",   "all")] << '\n';
            }
        }
    }

} // make

// ----------------------------------------------------------------------

data_t scan_antigens(std::string aStart, std::string aEnd)
{
    auto& locdb = get_locdb();
    const std::string all = "all";

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
                std::string continent = locdb.continent(std::string(antigen->location()), "UNKNOWN");
                std::string year = date.substr(0, 4);
                std::string lab(hidb.lab(*antigen, *tables));

                ++data_antigens[std::make_tuple(virus_type, lab, date, continent)];
                ++data_antigens[std::make_tuple(virus_type, lab, year, continent)];
                ++data_antigens[std::make_tuple(virus_type, lab, date, all)];
                ++data_antigens[std::make_tuple(virus_type, lab, year, all)];
                ++data_antigens[std::make_tuple(virus_type, lab, all,  continent)];
                ++data_antigens[std::make_tuple(virus_type, lab, all,  all)];
                ++data_antigens[std::make_tuple(virus_type, all, date, continent)];
                ++data_antigens[std::make_tuple(virus_type, all, year, continent)];
                ++data_antigens[std::make_tuple(virus_type, all, date, all)];
                ++data_antigens[std::make_tuple(virus_type, all, year, all)];
                ++data_antigens[std::make_tuple(virus_type, all, all,  continent)];
                ++data_antigens[std::make_tuple(virus_type, all, all,  all)];
                ++data_antigens[std::make_tuple(all,        lab, date, continent)];
                ++data_antigens[std::make_tuple(all,        lab, year, continent)];
                ++data_antigens[std::make_tuple(all,        lab, date, all)];
                ++data_antigens[std::make_tuple(all,        lab, year, all)];
                ++data_antigens[std::make_tuple(all,        lab, all,  continent)];
                ++data_antigens[std::make_tuple(all,        lab, all,  all)];
                ++data_antigens[std::make_tuple(all,        all, date, continent)];
                ++data_antigens[std::make_tuple(all,        all, year, continent)];
                ++data_antigens[std::make_tuple(all,        all, date, all)];
                ++data_antigens[std::make_tuple(all,        all, year, all)];
                ++data_antigens[std::make_tuple(all,        all, all,  continent)];
                ++data_antigens[std::make_tuple(all,        all, all,  all)];

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
                          std::cerr << "WARNING: no lineage for " << antigen->full_name() << '\n';
                          break;
                    }
                    ++data_antigens[std::make_tuple(vtl, lab, date, continent)];
                    ++data_antigens[std::make_tuple(vtl, lab, year, continent)];
                    ++data_antigens[std::make_tuple(vtl, lab, date, all)];
                    ++data_antigens[std::make_tuple(vtl, lab, year, all)];
                    ++data_antigens[std::make_tuple(vtl, lab, all,  continent)];
                    ++data_antigens[std::make_tuple(vtl, lab, all,  all)];
                    ++data_antigens[std::make_tuple(vtl, all, date, continent)];
                    ++data_antigens[std::make_tuple(vtl, all, year, continent)];
                    ++data_antigens[std::make_tuple(vtl, all, date, all)];
                    ++data_antigens[std::make_tuple(vtl, all, year, all)];
                    ++data_antigens[std::make_tuple(vtl, all, all,  continent)];
                    ++data_antigens[std::make_tuple(vtl, all, all,  all)];
                }

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
                std::string continent = locdb.continent(std::string(serum->location()), "UNKNOWN");
                std::string year = date.substr(0, 4);
                std::string lab(hidb.lab(*serum, *tables));

                ++data_sera[std::make_tuple(virus_type, lab, date, continent)];
                ++data_sera[std::make_tuple(virus_type, lab, year, continent)];
                ++data_sera[std::make_tuple(virus_type, lab, date, all)];
                ++data_sera[std::make_tuple(virus_type, lab, year, all)];
                ++data_sera[std::make_tuple(virus_type, lab, all,  continent)];
                ++data_sera[std::make_tuple(virus_type, lab, all,  all)];
                ++data_sera[std::make_tuple(virus_type, all, date, continent)];
                ++data_sera[std::make_tuple(virus_type, all, year, continent)];
                ++data_sera[std::make_tuple(virus_type, all, date, all)];
                ++data_sera[std::make_tuple(virus_type, all, year, all)];
                ++data_sera[std::make_tuple(virus_type, all, all,  continent)];
                ++data_sera[std::make_tuple(virus_type, all, all,  all)];
                ++data_sera[std::make_tuple(all,        lab, date, continent)];
                ++data_sera[std::make_tuple(all,        lab, year, continent)];
                ++data_sera[std::make_tuple(all,        lab, date, all)];
                ++data_sera[std::make_tuple(all,        lab, year, all)];
                ++data_sera[std::make_tuple(all,        lab, all,  continent)];
                ++data_sera[std::make_tuple(all,        lab, all,  all)];
                ++data_sera[std::make_tuple(all,        all, date, continent)];
                ++data_sera[std::make_tuple(all,        all, year, continent)];
                ++data_sera[std::make_tuple(all,        all, date, all)];
                ++data_sera[std::make_tuple(all,        all, year, all)];
                ++data_sera[std::make_tuple(all,        all, all,  continent)];
                ++data_sera[std::make_tuple(all,        all, all,  all)];

                if (virus_type == "B") {
                    std::string vtl;
                    switch (serum->lineage()) {
                      case acmacs::chart::BLineage::Victoria:
                          vtl = "BVICTORIA";
                          break;
                      case acmacs::chart::BLineage::Yamagata:
                          vtl = "BYAMAGATA";
                          break;
                      case acmacs::chart::BLineage::Unknown:
                          vtl = "BUNKNOWN";
                          std::cerr << "WARNING: no lineage for " << serum->full_name() << '\n';
                          break;
                    }
                    ++data_sera[std::make_tuple(vtl, lab, date, continent)];
                    ++data_sera[std::make_tuple(vtl, lab, year, continent)];
                    ++data_sera[std::make_tuple(vtl, lab, date, all)];
                    ++data_sera[std::make_tuple(vtl, lab, year, all)];
                    ++data_sera[std::make_tuple(vtl, lab, all,  continent)];
                    ++data_sera[std::make_tuple(vtl, lab, all,  all)];
                    ++data_sera[std::make_tuple(vtl, all, date, continent)];
                    ++data_sera[std::make_tuple(vtl, all, year, continent)];
                    ++data_sera[std::make_tuple(vtl, all, date, all)];
                    ++data_sera[std::make_tuple(vtl, all, year, all)];
                    ++data_sera[std::make_tuple(vtl, all, all,  continent)];
                    ++data_sera[std::make_tuple(vtl, all, all,  all)];
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
    rjson::object data{{"antigens", rjson::object{}},
                       {"sera", rjson::object{}},
                       {"sera_unique", rjson::object{}},
                       {"date", rjson::string{acmacs::date_format()}}};

    for (auto [key, value]: data_antigens) {
        auto [virus_type, lab, date, continent] = key;
        rjson::object& antigens = data["antigens"];
        rjson::object& for_vt = antigens.get_or_add(virus_type, rjson::object{});
        rjson::object& for_lab = for_vt.get_or_add(lab, rjson::object{});
        rjson::object& for_date = for_lab.get_or_add(date, rjson::object{});
        for_date.set_field(continent, rjson::integer{value});
    }

    for (auto [key, value]: data_sera) {
        auto [virus_type, lab, date, continent] = key;
        rjson::object& sera = data["sera"];
        rjson::object& for_vt = sera.get_or_add(virus_type, rjson::object{});
        rjson::object& for_lab = for_vt.get_or_add(lab, rjson::object{});
        rjson::object& for_date = for_lab.get_or_add(date, rjson::object{});
        for_date.set_field(continent, rjson::integer{value});
    }

    for (auto [key, value]: data_sera_unique) {
        auto [virus_type, lab, date, continent] = key;
        rjson::object& sera = data["sera_unique"];
        rjson::object& for_vt = sera.get_or_add(virus_type, rjson::object{});
        rjson::object& for_lab = for_vt.get_or_add(lab, rjson::object{});
        rjson::object& for_date = for_lab.get_or_add(date, rjson::object{});
        for_date.set_field(continent, rjson::integer{value});
    }

    return data.to_json_pp(1);

} // make_json

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
