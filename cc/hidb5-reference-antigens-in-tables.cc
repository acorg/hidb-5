// CDC (Catherine B. Smith) requested to produce a report:
// for all subtype, for all tables, all reference antigens in the table:
// Type (subtype), institution (lab), date of test, TEST TYPE (turkey, GP, FR, FRA, PRNT), virus (name), collection date, passage, antigen (lab_name_passage)

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/csv.hh"
#include "hidb-5/hidb.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

struct Record
{
    std::string subtype;
    std::string lab;
    std::string date;
    std::string test_type;
    std::string virus_name;
    acmacs::chart::Date collection_date;
    acmacs::chart::Passage passage;
    // std::string antigen_name;

    Record(std::string a_subtype, std::string_view a_lab, std::string_view a_date, std::string_view a_assay, std::string_view a_rbc, std::string a_virus_name,
           const acmacs::chart::Date& a_collection_date, const acmacs::chart::Passage& a_passage)
        : subtype(fix_subtype(a_subtype)), lab(fix_lab(a_lab)), date(fix_date(a_date)), test_type(fix_test_type(a_assay, a_rbc)), virus_name(fix_virus_name(a_virus_name)),
          collection_date(a_collection_date), passage(a_passage)
    {
    }

    bool operator<(const Record& rhs) const
    {
        if (subtype != rhs.subtype)
            return subtype < rhs.subtype;
        if (lab != rhs.lab)
            return lab < rhs.lab;
        if (date != rhs.date)
            return date > rhs.date; // most recent first
        if (virus_name != rhs.virus_name)
            return virus_name < rhs.virus_name;
        return passage < rhs.passage;
    }

    static inline std::string fix_subtype(std::string a_subtype)
        {
            return a_subtype;
        }

    static inline std::string fix_lab(std::string_view a_lab) { return std::string(a_lab); }

    static inline std::string fix_date(std::string_view a_date) { return std::string(a_date); }

    static inline std::string fix_test_type(std::string_view a_assay, std::string_view a_rbc)
        {
            if (a_rbc.empty())
                return std::string(a_assay);
            else
                return std::string(a_rbc);
        }

    static inline std::string fix_virus_name(std::string_view a_virus_name) { return std::string(a_virus_name); }
};

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
        if (args["-h"] || args["--help"] || args.number_of_arguments() != 0) {
            throw std::runtime_error("Usage: "s + args.program() + " [options]\n" + args.usage_options());
        }
        const bool verbose = args["-v"] || args["--verbose"];
        hidb::setup(std::string(args["--db-dir"]), {}, verbose);

        std::vector<Record> records;
        for (auto virus_type : {"A(H1N1)", "A(H3N2)", "B"}) {
            auto& hidb = hidb::get(virus_type);
            auto tables = hidb.tables();
            auto antigens = hidb.antigens();
            for (size_t ag_no = 0; ag_no < antigens->size(); ++ag_no) {
                auto antigen = antigens->at(ag_no);
                for (auto table_no : antigen->tables()) {
                    auto table = (*tables)[table_no];
                    // if (antigen->reference())
                    records.emplace_back(virus_type, table->lab(), table->date(), table->assay(), table->rbc(),
                                       string::join(" ", {antigen->name(), string::join(" ", antigen->annotations()), antigen->reassortant()}), antigen->date(), antigen->passage());
                }
            }
        }
        std::sort(records.begin(), records.end());
        std::cerr << "DEBUG: records: " << records.size() << '\n';

        acmacs::CsvWriter csv;
        csv << "Type" << "institution" << "date of test" << "TEST TYPE" << "virus" << "collection date" << "passage" << "antigen" << acmacs::CsvWriter::end_of_row;
        for (auto [r_no, record] : acmacs::enumerate(records)) {
            csv << record.subtype << record.lab << record.date << record.test_type << record.virus_name
                << static_cast<std::string>(record.collection_date)
                << static_cast<std::string>(record.passage)
                << string::join("_", {record.lab, record.virus_name, record.passage}) << acmacs::CsvWriter::end_of_row;
            if (r_no > 100)
                break;
        }

        std::cout << csv << '\n';

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
