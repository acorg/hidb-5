#include <fstream>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/date.hh"
#include "acmacs-base/csv.hh"
#include "locationdb/locdb.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/hidb-set.hh"

using namespace std::string_literals;

static const char* sDesc = "for each subtype, each antigen make a table with isolation date, first table date, country, cdcid; separate files for subtype, lab, HI and Neut;";

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv,
                       {
                           {"--db-dir", ""},
                           {"-v", false},
                           {"--verbose", false},
                           {"-h", false},
                           {"--help", false},
                       });
        if (args["-h"] || args["--help"] || args.number_of_arguments() != 1) {
            throw std::runtime_error("Usage: "s + args.program() + " [options] <output-prefix>\n" + sDesc + '\n' + args.usage_options());
        }
        const bool verbose = args["-v"] || args["--verbose"];
        hidb::setup(std::string(args["--db-dir"]), {}, verbose);
        auto& locdb = get_locdb();
        std::map<std::string, std::vector<std::tuple<std::string, std::string, std::string, std::string, std::string, std::string, std::string>>> data;
        for (std::string_view subtype : {"B", "H1", "H3"}) {
            auto& hidb = hidb::get(subtype, report_time::yes);
            auto antigens = hidb.antigens();
            auto tables = hidb.tables();
            for (auto ag_no = 0UL; ag_no < antigens->size(); ++ag_no) {
                auto antigen = antigens->at(ag_no);
                const Date date = antigen->date_compact();
                const auto lineage = antigen->lineage();
                const auto country = antigen->country(locdb);
                std::string lab_id;
                if (const auto lab_ids = antigen->lab_ids(); !lab_ids.empty())
                    lab_id = lab_ids[0];
                auto first_table = tables->oldest(antigen->tables());
                const auto first_table_date = first_table->date().substr(0, 8);
                const int days = date.empty() ? -1 : days_between_dates(date, Date(first_table_date));
                const std::string tag = std::string(subtype) + '-' + std::string(first_table->lab()) + '-' + std::string(first_table->assay());
                data[tag].emplace_back(antigen->name(), date, first_table_date, days >= 0 ? std::to_string(days) : std::string{}, country, lab_id, lineage);
            }
        }
        for (auto [tag, entries] : data) {
            acmacs::CsvWriter csv;
            for (auto field : {"Name", "Isolation", "Table", "Days", "Country", "Lab Id", "Lineage"})
                csv.add_field(field);
            csv.new_row();
            for (const auto& entry: entries) {
                csv.add_field(std::get<0>(entry));
                csv.add_field(std::get<1>(entry));
                csv.add_field(std::get<2>(entry));
                csv.add_field(std::get<3>(entry));
                csv.add_field(std::get<4>(entry));
                csv.add_field(std::get<5>(entry));
                csv.add_field(std::get<6>(entry));
                csv.new_row();
            }
            std::ofstream out(string::concat(args[0], tag, ".csv"));
            out << static_cast<std::string_view>(csv);
        }

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
