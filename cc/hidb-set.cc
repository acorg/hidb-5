#include <map>
#include <memory>

#include "hidb-5/hidb-set.hh"
#include "hidb-5/hidb.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

class HiDbSet;

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

static bool sVerbose = false;
static std::string sHiDbDir = std::getenv("HOME") + "/AD/data"s;
static std::unique_ptr<HiDbSet> sHiDbSet;

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

class HiDbSet
{
 public:
    const hidb::HiDb& get(std::string aVirusType, report_time timer = report_time::No) const
        {
            auto h = mPtrs.find(aVirusType);
            if (h == mPtrs.end()) {
                std::string filename;
                if (aVirusType == "A(H1N1)" || aVirusType == "H1")
                    filename = sHiDbDir + "/hidb5.h1.json.xz";
                else if (aVirusType == "A(H3N2)" || aVirusType == "H3")
                    filename = sHiDbDir + "/hidb5.h3.json.xz";
                else if (aVirusType == "B")
                    filename = sHiDbDir + "/hidb5.b.json.xz";
                else
                      //throw NoHiDb{};
                    throw std::runtime_error("No HiDb for " + aVirusType);

                h = mPtrs.emplace(aVirusType, std::make_unique<hidb::HiDb>(filename, sVerbose ? report_time::Yes : timer)).first;
            }
            return *h->second;
        }

 private:
    using Ptrs = std::map<std::string, std::unique_ptr<hidb::HiDb>>;

    mutable Ptrs mPtrs;

}; // class HiDbSet


// ----------------------------------------------------------------------

void hidb::setup(std::string aHiDbDir, std::optional<std::string> aLocDbFilename, bool aVerbose)
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

const hidb::HiDb& hidb::get(std::string aVirusType, report_time timer)
{
    if (!sHiDbSet)
        sHiDbSet = std::make_unique<HiDbSet>();
    return sHiDbSet->get(aVirusType, timer);

} // hidb::get

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
