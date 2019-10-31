#include <fstream>

#include "acmacs-base/argv.hh"
#include "acmacs-base/date.hh"
#include "acmacs-base/csv.hh"
#include "locationdb/locdb.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/hidb-set.hh"

using namespace std::string_literals;

// static const char* sDesc = "for each subtype, each antigen make a table with isolation date, first table date, country, cdcid; separate files for subtype, lab, HI and Neut;";

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<bool> verbose{*this, 'v', "verbose"};
    option<str>  db_dir{*this, "db-dir", dflt{""}};

    argument<str> output_prefix{*this, mandatory};

};

int main(int argc, char* const argv[])
{
    try {
        Options opt(argc, argv);
        hidb::setup(opt.db_dir, {}, *opt.verbose);
        auto& locdb = get_locdb();
        std::map<std::string, std::vector<std::tuple<std::string, std::string, std::string, std::string, std::string, std::string, acmacs::virus::lineage_t>>> data;
        for (const std::string_view subtype : {"B", "H1", "H3"}) {
            auto& hidb = hidb::get(acmacs::virus::type_subtype_t{subtype}, report_time::yes);
            auto antigens = hidb.antigens();
            auto tables = hidb.tables();
            for (auto ag_no = 0UL; ag_no < antigens->size(); ++ag_no) {
                auto antigen = antigens->at(ag_no);
                const auto date = date::from_string(antigen->date_compact());
                const auto lineage = antigen->lineage();
                const auto country = antigen->country(locdb);
                std::string lab_id;
                if (const auto lab_ids = antigen->lab_ids(); !lab_ids->empty())
                    lab_id = (*lab_ids)[0];
                auto first_table = tables->oldest(antigen->tables());
                const auto first_table_date = first_table->date().substr(0, 8);
                const int days = date.ok() ? -1 : date::days_between_dates(date, date::from_string(first_table_date));
                const auto tag = fmt::format("{}-{}-{}", subtype, first_table->lab(), first_table->assay());
                data[tag].emplace_back(antigen->name(), date::display(date), first_table_date, days >= 0 ? std::to_string(days) : std::string{}, country, lab_id, acmacs::virus::lineage_t{lineage.to_string()});
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
                csv.add_field(*std::get<6>(entry));
                csv.new_row();
            }
            std::ofstream out(string::concat(*opt.output_prefix, tag, ".csv"));
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
