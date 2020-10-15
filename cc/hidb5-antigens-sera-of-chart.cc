#include "acmacs-base/argv.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/report.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<bool> antigens_only{*this, 'a', "antigens-only"};
    option<bool> sera_only{*this, 's', "sera-only"};
    option<bool> first_table{*this, "first-table"};
    option<str> db_dir{*this, "db-dir"};
    option<str> virus_type{*this, "flu"};
    option<bool> relaxed_passage{*this, "relaxed", desc{"ignore passage, if antigen with that passage not found"}};
    option<bool> verbose{*this, 'v', "verbose"};

    argument<str> chart{*this, arg_name{"chart"}, mandatory};
};

int main(int argc, char* const argv[])
{
    using namespace std::string_view_literals;
    try {
        Options opt(argc, argv);
        hidb::setup(opt.db_dir, {}, opt.verbose);

        auto chart = acmacs::chart::import_from_file(opt.chart);
        acmacs::virus::type_subtype_t virus_type{*opt.virus_type};
        if (virus_type.empty())
            virus_type = chart->info()->virus_type();
        auto& hidb = hidb::get(virus_type);

        const auto prefix = "    "sv;
        if (!opt.sera_only) {
            auto antigens = chart->antigens();
            for (auto ag : *antigens) {
                fmt::print("{}\n", ag->full_name());
                if (const auto found = hidb.antigens()->find(*ag, opt.relaxed_passage ? hidb::passage_strictness::ignore : hidb::passage_strictness::yes); found.has_value())
                    hidb::report_antigen(hidb, *found->first, hidb::report_tables::all, prefix);
                else
                    fmt::print("{}*not found*\n\n", prefix);
            }
        }

        if (!opt.antigens_only) {
            auto sera = chart->sera();
            for (auto sr : *sera) {
                fmt::print("{}\n", sr->full_name());
                if (const auto found = hidb.sera()->find(*sr); found.has_value())
                    hidb::report_serum(hidb, *found->first, hidb::report_tables::all, prefix);
                else
                    fmt::print("{}*not found*\n\n", prefix);
            }
        }
        return 0;
    }
    catch (std::exception& err) {
        AD_ERROR("{}", err);
        return 1;
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
