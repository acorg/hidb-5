#include <cstdlib>
#include <cmath>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/enumerate.hh"
#include "hidb-5/hidb.hh"

using namespace std::string_literals;

static void list_all_antigens(const hidb::HiDb& hidb);
static void list_all_sera(const hidb::HiDb& hidb);

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {
                {"--db-dir", ""},
                {"-v", false},
                {"--verbose", false},
                {"-h", false},
                {"--help", false},
            });
        if (args["-h"] || args["--help"] || args.number_of_arguments() < 2) {
            throw std::runtime_error("Usage: "s + args.program() + " [options] <virus-type: B, H1, H3> <name> ...\n" + args.usage_options());
        }
        const bool verbose = args["-v"] || args["--verbose"];
        hidb::setup(args["--db-dir"], {}, verbose);

        const auto& hidb = hidb::get(string::upper(args[0]), report_time::Yes);
          // list_all_antigens(hidb);
        // list_all_sera(hidb);

        // for (auto arg = 2; arg < argc; ++arg) {
        //     Timeit timeit("looking: ");
        //     const auto look_for = string::upper(argv[arg]);
        //     const auto results = hidb.find_antigens_with_score(look_for);
        //     if (!results.empty()) {
        //         const auto num_digits = static_cast<int>(std::log10(results.size())) + 1;
        //         size_t result_no = 1;
        //         for (const auto& result: results) {
        //             std::cout << std::setw(num_digits) << result_no << ' ' << result.second << ' ' << result.first->full_name() << '\n';
        //             ++result_no;
        //         }
        //         if ((arg + 1) < argc)
        //             std::cout << '\n';
        //     }
        // }

        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        return 1;
    }
}

// ----------------------------------------------------------------------

void list_all_sera(const hidb::HiDb& hidb)
{
    auto sera = hidb.sera();
    std::cerr << "Sera: " << sera->size() << '\n';
    for (auto [sr_no, serum]: acmacs::enumerate(*sera)) {
        std::cout << sr_no << ' ' << serum->name();
        // if (const auto annotations = serum->annotations(); !annotations.empty())
        //     std::cout << " A:" << annotations;
        // if (const auto reassortant = serum->reassortant(); !reassortant.empty())
        //     std::cout << " R:" << reassortant;
        // if (const auto passage = serum->passage(); !passage.empty())
        //     std::cout << " P:" << passage;
        // if (const auto lineage = serum->lineage(); lineage != acmacs::chart::BLineage::Unknown)
        //     std::cout << ' ' << static_cast<std::string>(lineage);
        // std::cout << " T:" << dynamic_cast<const hidb::Serum&>(*serum).tables();
        std::cout << '\n';
    }

} // list_all_sera

// ----------------------------------------------------------------------

void list_all_antigens(const hidb::HiDb& hidb)
{
    auto antigens = hidb.antigens();
    std::cerr << "Antigens: " << antigens->size() << '\n';
    for (auto [ag_no, antigen]: acmacs::enumerate(*antigens)) {
        std::cout << ag_no << ' ' << antigen->name();
        if (const auto annotations = antigen->annotations(); !annotations.empty())
            std::cout << " A:" << annotations;
        if (const auto reassortant = antigen->reassortant(); !reassortant.empty())
            std::cout << " R:" << reassortant;
        if (const auto passage = antigen->passage(); !passage.empty())
            std::cout << " P:" << passage;
        if (const auto date = antigen->date(); !date.empty())
            std::cout << " D:" << date;
        if (const auto lineage = antigen->lineage(); lineage != acmacs::chart::BLineage::Unknown)
            std::cout << ' ' << static_cast<std::string>(lineage);
        if (const auto lab_ids = antigen->lab_ids(); !lab_ids.empty())
            std::cout << " I:" << lab_ids;
        std::cout << " T:" << dynamic_cast<const hidb::Antigen&>(*antigen).tables();
        std::cout << '\n';
    }

} // list_all_antigens

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
