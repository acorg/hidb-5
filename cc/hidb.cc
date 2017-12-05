#include <algorithm>

#include "acmacs-base/string.hh"
#include "acmacs-base/string-split.hh"
#include "acmacs-base/virus-name.hh"
#include "locationdb/locdb.hh"
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
                                            antigens + sizeof(hidb::bin::ast_number_t) + sizeof(hidb::bin::ast_offset_t) * (number_of_antigens + 1),
                                            reinterpret_cast<const hidb::bin::Header*>(mData)->virus_type());

} // hidb::HiDb::antigens

// ----------------------------------------------------------------------

hidb::AntigenP hidb::Antigens::at(size_t aIndex) const
{
    return std::make_shared<hidb::Antigen>(mAntigen0 + reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex)[aIndex], mVirusType);
}

// ----------------------------------------------------------------------

acmacs::chart::Name hidb::Antigen::name() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->name();

} // hidb::Antigen::name

// ----------------------------------------------------------------------

std::string_view hidb::Antigen::location() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->location();

} // hidb::Antigen::location

// ----------------------------------------------------------------------

std::string_view hidb::Antigen::isolation() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->isolation();

} // hidb::Antigen::isolation

// ----------------------------------------------------------------------

std::string hidb::Antigen::year() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->year();

} // hidb::Antigen::year

// ----------------------------------------------------------------------

acmacs::chart::Date hidb::Antigen::date() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->date(false);

} // hidb::Antigen::date

// ----------------------------------------------------------------------

std::string hidb::Antigen::date_compact() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->date(true);

} // hidb::Antigen::date_compact

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

size_t hidb::Antigen::number_of_tables() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->tables().first;

} // hidb::Antigen::number_of_tables

// ----------------------------------------------------------------------

std::shared_ptr<hidb::Sera> hidb::HiDb::sera() const
{
    const auto* sera = mData + reinterpret_cast<const hidb::bin::Header*>(mData)->serum_offset;
    const auto number_of_sera = *reinterpret_cast<const hidb::bin::ast_number_t*>(sera);
    return std::make_shared<hidb::Sera>(static_cast<size_t>(number_of_sera),
                                        sera + sizeof(hidb::bin::ast_number_t),
                                        sera + sizeof(hidb::bin::ast_number_t) + sizeof(hidb::bin::ast_offset_t) * (number_of_sera + 1),
                                        reinterpret_cast<const hidb::bin::Header*>(mData)->virus_type());

} // hidb::HiDb::sera

// ----------------------------------------------------------------------

hidb::SerumP hidb::Sera::at(size_t aIndex) const
{
    return std::make_shared<hidb::Serum>(mSerum0 + reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex)[aIndex], mVirusType);

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

size_t hidb::Serum::number_of_tables() const
{
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->tables().first;

} // hidb::Serum::number_of_tables

// ----------------------------------------------------------------------

std::string_view hidb::Serum::location() const
{
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->location();

} // hidb::Serum::location

// ----------------------------------------------------------------------

std::string_view hidb::Serum::isolation() const
{
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->isolation();

} // hidb::Serum::isolation

// ----------------------------------------------------------------------

std::string hidb::Serum::year() const
{
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->year();

} // hidb::Serum::year

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
    const std::string lineage = acmacs::chart::BLineage(mTable->lineage);
    return string::join(":", {lab(), assay(), lineage, rbc(), date()});

} // hidb::Table::name

// ----------------------------------------------------------------------

std::string_view hidb::Table::assay() const
{
    return mTable->assay();

} // hidb::Table::assay

// ----------------------------------------------------------------------

std::string_view hidb::Table::lab() const
{
    return mTable->lab();

} // hidb::Table::lab

// ----------------------------------------------------------------------

std::string_view hidb::Table::date() const
{
    return mTable->date();

} // hidb::Table::date

// ----------------------------------------------------------------------

std::string_view hidb::Table::rbc() const
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

std::shared_ptr<hidb::Table> hidb::Tables::most_recent(const indexes_t& aTables) const
{
    const auto* index = reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex);
    auto table_date_compare = [this,index] (size_t i1, size_t i2) -> bool {
                                  const auto t1{reinterpret_cast<const bin::Table*>(this->mTable0 + index[i1])}, t2{reinterpret_cast<const bin::Table*>(this->mTable0 + index[i2])};
                                  return t1->date() < t2->date();
                              };
    return operator[](*std::max_element(aTables.begin(), aTables.end(), table_date_compare));

} // hidb::Tables::most_recent

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

hidb::AntigenPIndexList hidb::Antigens::find(std::string aName, bool aFixLocation) const
{
    const first_last_t all_antigens(reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex), mNumberOfAntigens);
    first_last_t first_last;
    try {
        std::string virus_type, host, location, isolation, year, passage;
        virus_name::split(aName, virus_type, host, location, isolation, year, passage);
        if (aFixLocation)
            location = get_locdb().find(location).name;
        first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, location, isolation, year);
    }
    catch (virus_name::Unrecognized&) {
        if (aName.size() > 3 && aName[2] == ' ') // cdc name?
            first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, std::string_view(aName.data(), 2), std::string_view(aName.data() + 3), std::string_view{});
        if (first_last.empty()) {
            const auto parts = acmacs::string::split(aName, "/");
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

    AntigenPIndexList result(first_last.size());
    std::transform(first_last.first, first_last.last, result.begin(),
                   [this,index_begin=all_antigens.first](const hidb::bin::ast_offset_t& offset) -> AntigenPIndex { return {std::make_shared<Antigen>(this->mAntigen0 + offset, mVirusType), static_cast<size_t>(&offset - index_begin)}; });
    return result;

} // hidb::Antigens::find

// ----------------------------------------------------------------------

hidb::SerumPIndexList hidb::Sera::find(std::string aName, bool aFixLocation) const
{
    const first_last_t all_sera(reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex), mNumberOfSera);
    first_last_t first_last;
    std::string location;
    try {
        std::string virus_type, host, isolation, year, passage;
        virus_name::split(aName, virus_type, host, location, isolation, year, passage);
        if (aFixLocation)
            location = get_locdb().find(location).name;
        first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, location, isolation, year);
    }
    catch (virus_name::Unrecognized&) {
        const auto parts = acmacs::string::split(aName, "/");
        switch (parts.size()) {
          case 1:           // just location?
              first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, parts[0], std::string_view{}, std::string_view{});
              break;
          case 2:           // location/isolation
              location = aFixLocation ? get_locdb().find(std::string(parts[0])).name : std::string(parts[0]);
              first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, std::string_view(location), parts[1], std::string_view{});
              break;
          case 3:          // host/location/isolation?
              location = aFixLocation ? get_locdb().find(std::string(parts[1])).name : std::string(parts[1]);
              first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, std::string_view(location), parts[2], std::string_view{});
              break;
          default:          // ?
              std::cerr << "WARNING: don't know how to split: " << aName << '\n';
              break;
        }
    }

    SerumPIndexList result(first_last.size());
    std::transform(first_last.first, first_last.last, result.begin(),
                   [this,index_begin=all_sera.first](const hidb::bin::ast_offset_t& offset) -> SerumPIndex { return {std::make_shared<Serum>(this->mSerum0 + offset, mVirusType), static_cast<size_t>(&offset - index_begin)}; });
    return result;

} // hidb::Sera::find

// ----------------------------------------------------------------------

hidb::AntigenPList hidb::Antigens::find_labid(std::string labid) const
{
    AntigenPList result;

    auto find = [&result,this](std::string look_for) -> void {
        const first_last_t all_antigens(reinterpret_cast<const hidb::bin::ast_offset_t*>(this->mIndex), this->mNumberOfAntigens);
        for (auto ag = all_antigens.first; ag != all_antigens.last; ++ag) {
            const auto lab_ids = reinterpret_cast<const hidb::bin::Antigen*>(this->mAntigen0 + *ag)->lab_ids();
            if (const auto found = std::find(lab_ids.begin(), lab_ids.end(), look_for); found != lab_ids.end())
                result.push_back(std::make_shared<Antigen>(this->mAntigen0 + *ag, mVirusType));
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

hidb::AntigenPIndex hidb::Antigens::find(const acmacs::chart::Antigen& aAntigen) const
{
    const auto antigen_index_list = find(aAntigen.name(), false);
    for (auto antigen_index: antigen_index_list) {
        const auto& antigen = antigen_index.first;
        if (antigen->annotations() == aAntigen.annotations() && antigen->reassortant() == aAntigen.reassortant() && antigen->passage() == aAntigen.passage())
            return antigen_index;
    }
    std::cerr << "WARNING: not in hidb: " << aAntigen.full_name() << '\n';
    throw not_found(aAntigen.full_name());

} // hidb::Antigens::find

// ----------------------------------------------------------------------

hidb::AntigenPList hidb::Antigens::find(const acmacs::chart::Antigens& aAntigens) const
{
    hidb::AntigenPList result;
    for (auto antigen: aAntigens) {
        try {
            result.push_back(find(*antigen).first);
        }
        catch (not_found&) {
            result.emplace_back(nullptr);
        }
    }
    return result;

} // hidb::Antigens::find

// ----------------------------------------------------------------------

hidb::SerumPIndex hidb::Sera::find(const acmacs::chart::Serum& aSerum) const
{
    const auto serum_index_list = find(aSerum.name(), false);
    for (auto serum_index: serum_index_list) {
        const auto& serum = serum_index.first;
        if (serum->annotations() == aSerum.annotations() && serum->reassortant() == aSerum.reassortant() && serum->serum_id() == aSerum.serum_id())
            return serum_index;
        // std::cerr << aSerum.full_name() << " A:" << aSerum.annotations() << " R:" << aSerum.reassortant() << " I:" << aSerum.serum_id() << '\n';
        // std::cerr << serum->full_name() << " A:" << serum->annotations() << " R:" << serum->reassortant() << " I:" << serum->serum_id() << '\n';
    }
    std::cerr << "WARNING: not in hidb: " << aSerum.full_name() << '\n';
    throw not_found(aSerum.full_name());

} // hidb::Sera::find

// ----------------------------------------------------------------------

hidb::SerumPList hidb::Sera::find(const acmacs::chart::Sera& aSera) const
{
    hidb::SerumPList result;
    for (auto serum: aSera) {
        try {
            result.push_back(find(*serum).first);
        }
        catch (not_found&) {
            result.emplace_back(nullptr);
        }
    }
    return result;

} // hidb::Sera::find

// ----------------------------------------------------------------------

hidb::AntigenPList hidb::Antigens::date_range(std::string first, std::string after_last) const
{
    hidb::AntigenPList result;
    const auto min_date = !first.empty() ? hidb::bin::Antigen::make_date(first) : hidb::bin::Antigen::min_date();
    const auto max_date = !after_last.empty() ? hidb::bin::Antigen::make_date(after_last) : hidb::bin::Antigen::max_date();
    const first_last_t all_antigens(reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex), mNumberOfAntigens);
    for (auto ag = all_antigens.first; ag != all_antigens.last; ++ag) {
        const auto date = reinterpret_cast<const hidb::bin::Antigen*>(this->mAntigen0 + *ag)->date_raw();
        if (date >= min_date && date < max_date)
            result.push_back(std::make_shared<Antigen>(this->mAntigen0 + *ag, mVirusType));
    }
    return result;

} // hidb::Antigens::date_range

// ----------------------------------------------------------------------

hidb::SerumPList hidb::Sera::find_homologous(size_t aAntigenIndex, const Antigen& aAntigen) const
{
    const first_last_t all_sera(reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex), mNumberOfSera);
    const std::string antigen_year(aAntigen.year());
    const first_last_t first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, aAntigen.location(), aAntigen.isolation(), std::string_view(antigen_year));
    hidb::SerumPList result;
    for (auto offset_p = first_last.first; offset_p != first_last.last; ++offset_p) {
        const auto [num_homologous, first_homologous_p] = reinterpret_cast<const hidb::bin::Serum*>(mSerum0 + *offset_p)->homologous_antigens();
        if (std::find(first_homologous_p, first_homologous_p + num_homologous, aAntigenIndex) != (first_homologous_p + num_homologous)) {
            result.push_back(std::make_shared<hidb::Serum>(mSerum0 + *offset_p, mVirusType));
        }
    }
    // std::cerr << "find_homologous " << aAntigenIndex << ' ' << aAntigen.full_name() << ' ' << result.size() << '\n';
    return result;

} // hidb::Sera::find_homologous

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
