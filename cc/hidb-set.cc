#include <map>
#include <memory>

#include "acmacs-base/acmacsd.hh"
#include "acmacs-base/filesystem.hh"
#include "hidb-5/hidb-set.hh"
#include "hidb-5/hidb.hh"

// ----------------------------------------------------------------------

class HiDbSet;

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

static bool sVerbose = false;
static std::string sHiDbDir = acmacs::hidb_v5_dir();
static std::unique_ptr<HiDbSet> sHiDbSet;

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

class HiDbSet
{
 public:
    const hidb::HiDb& get(const acmacs::virus::type_subtype_t& aVirusType, report_time timer = report_time::no) const
        {
            using namespace std::string_literals;
            auto h = mPtrs.find(aVirusType);
            if (h == mPtrs.end()) {
                std::string prefix;
                if (aVirusType == acmacs::virus::type_subtype_t{"A(H1N1)"} || aVirusType == acmacs::virus::type_subtype_t{"H1"})
                    prefix = "hidb5.h1";
                else if (aVirusType == acmacs::virus::type_subtype_t{"A(H3N2)"} || aVirusType == acmacs::virus::type_subtype_t{"H3"})
                    prefix = "hidb5.h3";
                else if (aVirusType == acmacs::virus::type_subtype_t{"B"})
                    prefix = "hidb5.b";
                else
                    throw hidb::get_error(fmt::format("Unrecognized virus type: \"{}\"", aVirusType));

                fs::path filename = fs::path(sHiDbDir) / (prefix + ".hidb5b");
                if (!fs::exists(filename))
                    filename = fs::path(sHiDbDir) / (prefix + ".json.xz");
                if (!fs::exists(filename))
                    throw hidb::get_error(fmt::format("Cannot find hidb for {} in {}", aVirusType, sHiDbDir));

                Timeit ti("DEBUG: HiDb loading from " + static_cast<std::string>(filename) + ": ", timer);
                h = mPtrs.emplace(aVirusType, std::make_unique<hidb::HiDb>(static_cast<std::string>(filename), sVerbose || (timer == report_time::yes))).first;
            }
            return *h->second;
        }

 private:
    using Ptrs = std::map<acmacs::virus::type_subtype_t, std::unique_ptr<hidb::HiDb>>;

    mutable Ptrs mPtrs;

}; // class HiDbSet


// ----------------------------------------------------------------------

void hidb::setup(std::string_view aHiDbDir, std::optional<std::string> /*aLocDbFilename*/, bool aVerbose)
{
    sVerbose = aVerbose;
    if (!aHiDbDir.empty())
        sHiDbDir = aHiDbDir;
    // if (aLocDbFilename && !aLocDbFilename->empty())
    //     locdb_setup(*aLocDbFilename, sVerbose);
    // else if (!aHiDbDir.empty())
    //     locdb_setup(aHiDbDir + "/locationdb.json.xz", sVerbose);

} // hidb::setup

// ----------------------------------------------------------------------

const hidb::HiDb& hidb::get(const acmacs::virus::type_subtype_t& aVirusType, report_time timer)
{
    if (!sHiDbSet)
        sHiDbSet = std::make_unique<HiDbSet>();
    return sHiDbSet->get(aVirusType, timer);

} // hidb::get

// ----------------------------------------------------------------------

void hidb::load_all(report_time timer)
{
    using namespace std::string_view_literals;

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wunused-result"
#endif

    (void)get(acmacs::virus::type_subtype_t{"A(H1N1)"}, timer);
    (void)get(acmacs::virus::type_subtype_t{"A(H3N2)"}, timer);
    (void)get(acmacs::virus::type_subtype_t{"B"}, timer);

#pragma GCC diagnostic pop

} // hidb::load_all

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
