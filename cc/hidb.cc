#include <algorithm>

#include "acmacs-base/string.hh"
#include "acmacs-base/virus-name.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/hidb-bin.hh"
#include "hidb-5/hidb-json.hh"

// ----------------------------------------------------------------------

hidb::HiDb::HiDb(std::string aFilename, report_time /*timer*/)
{
    // Timeit ti("reading hidb from " + aFilename + ": ", timer);
    acmacs::file::read_access access(aFilename);
    if (hidb::bin::has_signature(access.data())) {
        mAccess = std::move(access);
        mData = mAccess.data();
    }
    else if (std::string data = access; data.find("\"  version\": \"hidb-v5\"") != std::string::npos) {
        mDataStorage = hidb::json::read(data);
        mData = mDataStorage.data();
    }
    else
        throw std::runtime_error("[hidb] unrecognized file: " + aFilename);

} // hidb::HiDb::HiDb

// ----------------------------------------------------------------------

void hidb::HiDb::save(std::string aFilename) const
{
    if (!mDataStorage.empty())
        acmacs::file::write(aFilename, mDataStorage);
    else if (mAccess.valid())
        acmacs::file::write(aFilename, {mAccess.data(), mAccess.size()});

} // hidb::HiDb::save

// ----------------------------------------------------------------------

std::shared_ptr<hidb::Antigens> hidb::HiDb::antigens() const
{
    const auto* antigens = mData + reinterpret_cast<const hidb::bin::Header*>(mData)->antigen_offset;
    const auto number_of_antigens = *reinterpret_cast<const hidb::bin::ast_number_t*>(antigens);
    return std::make_shared<hidb::Antigens>(static_cast<size_t>(number_of_antigens),
                                            antigens + sizeof(hidb::bin::ast_number_t),
                                            antigens + sizeof(hidb::bin::ast_number_t) + sizeof(hidb::bin::ast_offset_t) * (number_of_antigens + 1));

} // hidb::HiDb::antigens

// ----------------------------------------------------------------------

std::shared_ptr<acmacs::chart::Antigen> hidb::Antigens::operator[](size_t aIndex) const
{
    return std::make_shared<hidb::Antigen>(mAntigen0 + reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex)[aIndex]);

} // hidb::Antigens::operator[]

// ----------------------------------------------------------------------

acmacs::chart::Name hidb::Antigen::name() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->name();

} // hidb::Antigen::name

// ----------------------------------------------------------------------

acmacs::chart::Date hidb::Antigen::date() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->date();

} // hidb::Antigen::date

// ----------------------------------------------------------------------

acmacs::chart::Passage hidb::Antigen::passage() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->passage();

} // hidb::Antigen::passage

// ----------------------------------------------------------------------

acmacs::chart::BLineage hidb::Antigen::lineage() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->lineage;

} // hidb::Antigen::lineage

// ----------------------------------------------------------------------

acmacs::chart::Reassortant hidb::Antigen::reassortant() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->reassortant();

} // hidb::Antigen::reassortant

// ----------------------------------------------------------------------

acmacs::chart::LabIds hidb::Antigen::lab_ids() const
{
    const auto lab_ids = reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->lab_ids();
    acmacs::chart::LabIds result(lab_ids.size());
    std::transform(lab_ids.begin(), lab_ids.end(), result.begin(), [](const auto& li) { return std::string(li); });
    return result;

} // hidb::Antigen::lab_ids

// ----------------------------------------------------------------------

acmacs::chart::Annotations hidb::Antigen::annotations() const
{
    const auto annotations = reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->annotations();
    acmacs::chart::Annotations result(annotations.size());
    std::transform(annotations.begin(), annotations.end(), result.begin(), [](const auto& anno) { return std::string(anno); });
    return result;

} // hidb::Antigen::annotations

// ----------------------------------------------------------------------

hidb::indexes_t hidb::Antigen::tables() const
{
    const auto [size, ptr] = reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->tables();
    indexes_t result(size);
    std::transform(ptr, ptr + size, result.begin(), [](const auto& index) -> size_t { return static_cast<size_t>(index); });
    return result;

} // hidb::Antigen::tables

// ----------------------------------------------------------------------

std::shared_ptr<hidb::Sera> hidb::HiDb::sera() const
{
    const auto* sera = mData + reinterpret_cast<const hidb::bin::Header*>(mData)->serum_offset;
    const auto number_of_sera = *reinterpret_cast<const hidb::bin::ast_number_t*>(sera);
    return std::make_shared<hidb::Sera>(static_cast<size_t>(number_of_sera),
                                        sera + sizeof(hidb::bin::ast_number_t),
                                        sera + sizeof(hidb::bin::ast_number_t) + sizeof(hidb::bin::ast_offset_t) * (number_of_sera + 1));

} // hidb::HiDb::sera

// ----------------------------------------------------------------------

std::shared_ptr<acmacs::chart::Serum> hidb::Sera::operator[](size_t aIndex) const
{
    return std::make_shared<hidb::Serum>(mSerum0 + reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex)[aIndex]);

} // hidb::Sera::operator[]

// ----------------------------------------------------------------------

acmacs::chart::Name hidb::Serum::name() const
{
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->name();

} // hidb::Serum::name

// ----------------------------------------------------------------------

acmacs::chart::Passage hidb::Serum::passage() const
{
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->passage();

} // hidb::Serum::passsre

// ----------------------------------------------------------------------

acmacs::chart::BLineage hidb::Serum::lineage() const
{
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->lineage;

} // hidb::Serum::lineage

// ----------------------------------------------------------------------

acmacs::chart::Reassortant hidb::Serum::reassortant() const
{
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->reassortant();

} // hidb::Serum::reassortant

// ----------------------------------------------------------------------

acmacs::chart::Annotations hidb::Serum::annotations() const
{
    const auto annotations = reinterpret_cast<const hidb::bin::Serum*>(mSerum)->annotations();
    acmacs::chart::Annotations result(annotations.size());
    std::transform(annotations.begin(), annotations.end(), result.begin(), [](const auto& anno) { return std::string(anno); });
    return result;

} // hidb::Serum::annotations

// ----------------------------------------------------------------------

acmacs::chart::SerumId hidb::Serum::serum_id() const
{
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->serum_id();

} // hidb::Serum::serum_id

// ----------------------------------------------------------------------

acmacs::chart::SerumSpecies hidb::Serum::serum_species() const
{
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->serum_species();

} // hidb::Serum::serum_species

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList hidb::Serum::homologous_antigens() const
{
    const auto [size, ptr] = reinterpret_cast<const hidb::bin::Serum*>(mSerum)->homologous_antigens();
    acmacs::chart::PointIndexList result(size);
    std::transform(ptr, ptr + size, result.begin(), [](const auto& index) -> size_t { return static_cast<size_t>(index); });
    return result;

} // hidb::Serum::homologous_antigens

// ----------------------------------------------------------------------

hidb::indexes_t hidb::Serum::tables() const
{
    const auto [size, ptr] = reinterpret_cast<const hidb::bin::Serum*>(mSerum)->tables();
    indexes_t result(size);
    std::transform(ptr, ptr + size, result.begin(), [](const auto& index) -> size_t { return static_cast<size_t>(index); });
    return result;

} // hidb::Serum::tables

// ----------------------------------------------------------------------

std::shared_ptr<hidb::Tables> hidb::HiDb::tables() const
{
    const auto* tables = mData + reinterpret_cast<const hidb::bin::Header*>(mData)->table_offset;
    const auto number_of_tables = *reinterpret_cast<const hidb::bin::ast_number_t*>(tables);
    return std::make_shared<hidb::Tables>(static_cast<size_t>(number_of_tables),
                                          tables + sizeof(hidb::bin::ast_number_t),
                                          tables + sizeof(hidb::bin::ast_number_t) + sizeof(hidb::bin::ast_offset_t) * (number_of_tables + 1));

} // hidb::HiDb::tables

// ----------------------------------------------------------------------

std::shared_ptr<hidb::Table> hidb::Tables::operator[](size_t aIndex) const
{
    return std::make_shared<hidb::Table>(mTable0 + reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex)[aIndex]);

} // hidb::Tables::operator[]

// ----------------------------------------------------------------------

std::string hidb::Table::name() const
{
    return string::join(":", {lab(), assay(), acmacs::chart::BLineage(mTable->lineage), rbc(), date()});

} // hidb::Table::name

// ----------------------------------------------------------------------

std::string hidb::Table::assay() const
{
    return mTable->assay();

} // hidb::Table::assay

// ----------------------------------------------------------------------

std::string hidb::Table::lab() const
{
    return mTable->lab();

} // hidb::Table::lab

// ----------------------------------------------------------------------

std::string hidb::Table::date() const
{
    return mTable->date();

} // hidb::Table::date

// ----------------------------------------------------------------------

std::string hidb::Table::rbc() const
{
    return mTable->rbc();

} // hidb::Table::rbc

// ----------------------------------------------------------------------

size_t hidb::Table::number_of_antigens() const
{
    return mTable->number_of_antigens();

} // hidb::Table::number_of_antigens

// ----------------------------------------------------------------------

size_t hidb::Table::number_of_sera() const
{
    return mTable->number_of_sera();

} // hidb::Table::number_of_sera

// ----------------------------------------------------------------------

using offset_t = const hidb::bin::ast_offset_t*;

struct first_last_t
{
    inline first_last_t() : first{nullptr}, last{nullptr} {}
    inline first_last_t(offset_t a, offset_t b) : first(a), last(b) {}
    inline first_last_t(offset_t aFirst, size_t aNumber) : first(aFirst), last(aFirst + aNumber) {}
    inline size_t size() const { return static_cast<size_t>(last - first); }
    inline bool empty() const { return first == last; }
    offset_t first;
    offset_t last;

}; // struct first_last_t

template <typename F> inline first_last_t filter_by(first_last_t first_last, F field, std::string_view look_for)
{
    const auto found = std::lower_bound(
        first_last.first, first_last.last, look_for,
        [field](hidb::bin::ast_offset_t offset, const std::string_view& look_for_2) -> bool { return field(offset) < look_for_2; });
    for (auto end = found; end != first_last.last; ++end) {
        if (field(*end) != look_for)
            return {found, end};
    }
    return {found, first_last.last};
}

template <typename AgSr, typename S> inline first_last_t find_by(first_last_t first_last, const char* aData, S location, S isolation, S year)
{
    first_last = filter_by(first_last, [aData](hidb::bin::ast_offset_t offset) { return reinterpret_cast<const AgSr*>(aData + offset)->location(); }, location);
    if (!isolation.empty())
        first_last = filter_by(first_last, [aData](hidb::bin::ast_offset_t offset) { return reinterpret_cast<const AgSr*>(aData + offset)->isolation(); }, isolation);
    if (!year.empty())
        first_last = filter_by(first_last, [aData](hidb::bin::ast_offset_t offset) { return reinterpret_cast<const AgSr*>(aData + offset)->year(); }, year);
    return first_last;
}

// ----------------------------------------------------------------------

hidb::indexes_t hidb::Antigens::find(std::string aName) const
{
    const first_last_t all_antigens(reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex), mNumberOfAntigens);
    first_last_t first_last;
    try {
        std::string virus_type, host, location, isolation, year, passage;
        virus_name::split(aName, virus_type, host, location, isolation, year, passage);
        first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, location, isolation, year);
    }
    catch (virus_name::Unrecognized&) {
        if (aName.size() > 3 && aName[2] == ' ') // cdc name?
            first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, std::string_view(aName.data(), 2), std::string_view(aName.data() + 3), std::string_view{});
        if (first_last.empty()) {
            const auto parts = string::split(aName, "/");
            switch (parts.size()) {
              case 1:           // just location?
                  first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, parts[0], std::string_view{}, std::string_view{});
                  break;
              case 2:           // location/isolation
                  first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, parts[0], parts[1], std::string_view{});
                  break;
              case 3:          // host/location/isolation?
                  first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, parts[1], parts[2], std::string_view{});
                  break;
              default:          // ?
                  std::cerr << "WARNING: don't know how to split: " << aName << '\n';
                  break;
            }
        }
    }

    indexes_t result(first_last.size());
    std::transform(first_last.first, first_last.last, result.begin(), [index_begin=all_antigens.first](const hidb::bin::ast_offset_t& offset_ptr) -> size_t { return static_cast<size_t>(&offset_ptr - index_begin); });
    return result;

} // hidb::Antigens::find

// ----------------------------------------------------------------------

hidb::indexes_t hidb::Sera::find(std::string aName) const
{
    const first_last_t all_sera(reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex), mNumberOfSera);
    first_last_t first_last;
    try {
        std::string virus_type, host, location, isolation, year, passage;
        virus_name::split(aName, virus_type, host, location, isolation, year, passage);
        first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, location, isolation, year);
    }
    catch (virus_name::Unrecognized&) {
        const auto parts = string::split(aName, "/");
        switch (parts.size()) {
          case 1:           // just location?
              first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, parts[0], std::string_view{}, std::string_view{});
              break;
          case 2:           // location/isolation
              first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, parts[0], parts[1], std::string_view{});
              break;
          case 3:          // host/location/isolation?
              first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, parts[1], parts[2], std::string_view{});
              break;
          default:          // ?
              std::cerr << "WARNING: don't know how to split: " << aName << '\n';
              break;
        }
    }

    indexes_t result(first_last.size());
    std::transform(first_last.first, first_last.last, result.begin(), [index_begin=all_sera.first](const hidb::bin::ast_offset_t& offset_ptr) -> size_t { return static_cast<size_t>(&offset_ptr - index_begin); });
    return result;

} // hidb::Sera::find

// ----------------------------------------------------------------------

hidb::indexes_t hidb::Antigens::find_labid(std::string labid) const
{
    indexes_t result;

    auto find = [&result,this](std::string look_for) -> void {
        const first_last_t all_antigens(reinterpret_cast<const hidb::bin::ast_offset_t*>(this->mIndex), this->mNumberOfAntigens);
        for (auto ag = all_antigens.first; ag != all_antigens.last; ++ag) {
            const auto lab_ids = reinterpret_cast<const hidb::bin::Antigen*>(this->mAntigen0 + *ag)->lab_ids();
            if (const auto found = std::find(lab_ids.begin(), lab_ids.end(), look_for); found != lab_ids.end())
                result.push_back(static_cast<size_t>(ag - all_antigens.first));
        }
    };

    if (labid.find('#') == std::string::npos) {
        find("CDC#" + labid);
        if (result.empty())
            find("MELB#" + labid);
        if (result.empty())
            find("NIID#" + labid);
        if (result.empty())
            find(labid);
    }
    else {
        find(labid);
    }

    return result;

} // hidb::Antigens::find_labid

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
