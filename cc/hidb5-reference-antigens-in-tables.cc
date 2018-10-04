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
    std::string_view lab;
    std::string_view date;
    std::string_view assay;
    std::string_view rbc;
    std::string virus_name;
    acmacs::chart::Date collection_date;
    acmacs::chart::Passage passage;
      // std::string antigen_name;

    bool operator < (const Record& rhs) const
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
                    records.push_back({virus_type, table->lab(), table->date(), table->assay(), table->rbc(),
                                       string::join(" ", {antigen->name(), string::join(" ", antigen->annotations()), antigen->reassortant()}), antigen->date(), antigen->passage()});
                }
            }
        }
        std::sort(records.begin(), records.end());

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
