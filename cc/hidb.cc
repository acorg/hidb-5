#include <algorithm>
#include <optional>

#include "acmacs-base/log.hh"
#include "acmacs-base/fmt.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/string-split.hh"
#include "acmacs-base/string-join.hh"
#include "acmacs-virus/virus-name-v1.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/hidb-bin.hh"
#include "hidb-5/hidb-json.hh"

// ----------------------------------------------------------------------

hidb::HiDb::HiDb(std::string_view aFilename, bool verbose)
{
    acmacs::file::read_access access(aFilename);
    if (hidb::bin::has_signature(access.data())) {
        mAccess = std::move(access);
        mData = mAccess.data();
    }
    else if (std::string data = access; data.find("\"  version\": \"hidb-v5\"") != std::string::npos) {
        mDataStorage = hidb::json::read(data, verbose);
        mData = mDataStorage.data();
    }
    else
        throw std::runtime_error(fmt::format("[hidb] unrecognized file: {}", aFilename));

} // hidb::HiDb::HiDb

// ----------------------------------------------------------------------

void hidb::HiDb::save(std::string_view aFilename) const
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
                                            *this);

} // hidb::HiDb::antigens

// ----------------------------------------------------------------------

std::string_view hidb::HiDb::virus_type() const
{
    return reinterpret_cast<const hidb::bin::Header*>(mData)->virus_type();

} // hidb::HiDb::virus_type

// ----------------------------------------------------------------------

hidb::AntigenP hidb::Antigens::at(AntigenIndex aIndex) const
{
    return std::make_shared<hidb::Antigen>(mAntigen0 + reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex)[*aIndex], mHiDb);
}

// ----------------------------------------------------------------------

hidb::AntigenIndex hidb::Antigens::index(const hidb::bin::Antigen* antigen_bin) const
{
    const auto antigen_record_offset = static_cast<hidb::bin::ast_offset_t>(reinterpret_cast<const char*>(antigen_bin) - mAntigen0);
    const auto* index = reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex);
    if (const auto* ind_p = std::lower_bound(index, index + mNumberOfAntigens, antigen_record_offset); ind_p != (index + mNumberOfAntigens) && *ind_p == antigen_record_offset)
        return AntigenIndex{ind_p - index};
    else
        throw std::runtime_error{fmt::format("internal error in hidb::Antigens::index ind_p==end:{} ind_p:{} last:{} antigen_record_offset:{}",
                                             ind_p == (index + mNumberOfAntigens), *ind_p, *(index + mNumberOfAntigens - 1), antigen_record_offset)};
}

// ----------------------------------------------------------------------

hidb::AntigenPList hidb::Antigens::list(const AntigenIndexList& indexes) const
{
    hidb::AntigenPList result(indexes.size());
    std::transform(std::begin(indexes), std::end(indexes), std::begin(result), [this](const auto index) { return at(index); });
    return result;
}

// ----------------------------------------------------------------------

acmacs::virus::name_t hidb::Antigen::name() const
{
    auto antigen = reinterpret_cast<const hidb::bin::Antigen*>(mAntigen);
    if (antigen->cdc_name())
        return acmacs::virus::name_t{antigen->name()};
    else
        return acmacs::virus::name_t{std::string(mHiDb.virus_type()) + "/" + antigen->name()};

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
    return acmacs::chart::Date{reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->date(false)};

} // hidb::Antigen::date

// ----------------------------------------------------------------------

std::string hidb::Antigen::date_compact() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->date(true);

} // hidb::Antigen::date_compact

// ----------------------------------------------------------------------

acmacs::virus::Passage hidb::Antigen::passage() const
{
    return acmacs::virus::Passage{reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->passage()};

} // hidb::Antigen::passage

// ----------------------------------------------------------------------

acmacs::chart::BLineage hidb::Antigen::lineage() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->lineage;

} // hidb::Antigen::lineage

// ----------------------------------------------------------------------

acmacs::virus::Reassortant hidb::Antigen::reassortant() const
{
    return acmacs::virus::Reassortant{reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->reassortant()};

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

hidb::TableIndexList hidb::Antigen::tables() const
{
    const auto [size, ptr] = reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->tables();
    TableIndexList result(size);
    std::transform(ptr, ptr + size, result.begin(), [](const auto& index) { return TableIndex{index}; });
    return result;

} // hidb::Antigen::tables

// ----------------------------------------------------------------------

size_t hidb::Antigen::number_of_tables() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->tables().first;

} // hidb::Antigen::number_of_tables

// ----------------------------------------------------------------------

std::string_view hidb::Antigen::country(const LocDb& locdb) const noexcept
{
    using namespace std::string_view_literals;
    const auto loc = location();
    if (const auto country = locdb.country(loc); !country.empty())
        return country;
    else if (loc.size() == 2) {
        try {
            return locdb.find_cdc_abbreviation(loc).country();
        }
        catch (std::exception&) {
            return "UNKNOWN"sv;
        }
    }
    else
        return "UNKNOWN"sv;

} // hidb::Antigen::country

// ----------------------------------------------------------------------

std::string hidb::Antigen::full_name() const
{
    return acmacs::string::join(acmacs::string::join_space, name(), acmacs::string::join(acmacs::string::join_space, annotations()), reassortant(), passage());

} // hidb::Antigen::full_name

// ----------------------------------------------------------------------

std::shared_ptr<hidb::Sera> hidb::HiDb::sera() const
{
    const auto* sera = mData + reinterpret_cast<const hidb::bin::Header*>(mData)->serum_offset;
    const auto number_of_sera = *reinterpret_cast<const hidb::bin::ast_number_t*>(sera);
    return std::make_shared<hidb::Sera>(static_cast<size_t>(number_of_sera),
                                        sera + sizeof(hidb::bin::ast_number_t),
                                        sera + sizeof(hidb::bin::ast_number_t) + sizeof(hidb::bin::ast_offset_t) * (number_of_sera + 1),
                                        *this);

} // hidb::HiDb::sera

// ----------------------------------------------------------------------

hidb::SerumP hidb::Sera::at(SerumIndex aIndex) const
{
    return std::make_shared<hidb::Serum>(mSerum0 + reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex)[*aIndex], mHiDb);

} // hidb::Sera::at

// ----------------------------------------------------------------------

acmacs::virus::name_t hidb::Serum::name() const
{
    return acmacs::virus::name_t{std::string(mHiDb.virus_type()) + "/" + reinterpret_cast<const hidb::bin::Serum*>(mSerum)->name()};

} // hidb::Serum::name

// ----------------------------------------------------------------------

acmacs::virus::Passage hidb::Serum::passage() const
{
    return acmacs::virus::Passage{reinterpret_cast<const hidb::bin::Serum*>(mSerum)->passage()};

} // hidb::Serum::passsre

// ----------------------------------------------------------------------

acmacs::chart::BLineage hidb::Serum::lineage() const
{
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->lineage;

} // hidb::Serum::lineage

// ----------------------------------------------------------------------

acmacs::virus::Reassortant hidb::Serum::reassortant() const
{
    return acmacs::virus::Reassortant{reinterpret_cast<const hidb::bin::Serum*>(mSerum)->reassortant()};

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
    return acmacs::chart::SerumId{reinterpret_cast<const hidb::bin::Serum*>(mSerum)->serum_id()};

} // hidb::Serum::serum_id

// ----------------------------------------------------------------------

acmacs::chart::SerumSpecies hidb::Serum::serum_species() const
{
    return acmacs::chart::SerumSpecies{reinterpret_cast<const hidb::bin::Serum*>(mSerum)->serum_species()};

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

hidb::TableIndexList hidb::Serum::tables() const
{
    const auto [size, ptr] = reinterpret_cast<const hidb::bin::Serum*>(mSerum)->tables();
    TableIndexList result(size);
    std::transform(ptr, ptr + size, result.begin(), [](const auto& index) { return TableIndex{index}; });
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

std::vector<std::string> hidb::Serum::labs(const Tables& all_tables) const
{
    std::vector<std::string> result;
    for (auto table_index : tables()) {
        const std::string lab{all_tables[table_index]->lab()};
        if (std::find(result.begin(), result.end(), lab) == result.end())
            result.push_back(lab);
    }
    return result;

} // hidb::Serum::labs

// ----------------------------------------------------------------------

std::string hidb::Serum::full_name() const
{
    return acmacs::string::join(acmacs::string::join_space, name(), acmacs::string::join(acmacs::string::join_space, annotations()), reassortant(), serum_id());

} // hidb::Serum::full_name

// ----------------------------------------------------------------------

std::shared_ptr<hidb::Tables> hidb::HiDb::tables() const
{
    if (!tables_) {
        const auto* tables = mData + reinterpret_cast<const hidb::bin::Header*>(mData)->table_offset;
        const auto number_of_tables = *reinterpret_cast<const hidb::bin::ast_number_t*>(tables);
        tables_ = std::make_shared<hidb::Tables>(TableIndex{number_of_tables}, tables + sizeof(hidb::bin::ast_number_t),
                                                 tables + sizeof(hidb::bin::ast_number_t) + sizeof(hidb::bin::ast_offset_t) * (number_of_tables + 1));
    }
    return tables_;

} // hidb::HiDb::tables

// ----------------------------------------------------------------------

std::shared_ptr<hidb::Table> hidb::Tables::at(TableIndex aIndex) const
{
    return std::make_shared<hidb::Table>(mTable0 + reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex)[*aIndex]);

} // hidb::Tables::at

// ----------------------------------------------------------------------

std::vector<hidb::lab_assay_rbc_table_t> hidb::Tables::sorted(const TableIndexList& indexes, lab_assay_rbc_table_t::sort_by_date_order order) const
{
    std::vector<std::shared_ptr<Table>> tables_of_indexes(indexes.size());
    std::transform(std::begin(indexes), std::end(indexes), tables_of_indexes.begin(), [this](auto aIndex) { return at(aIndex); });

    std::sort(std::begin(tables_of_indexes), std::end(tables_of_indexes), [order](const auto& t1, const auto& t2) {
        const auto k1 = fmt::format("{}:{}:{}", t1->lab(), t1->assay(), t1->rbc()), k2 = fmt::format("{}:{}:{}", t2->lab(), t2->assay(), t2->rbc());
        if (k1 == k2) {
            switch (order) {
                case lab_assay_rbc_table_t::oldest_first:
                    return t1->date() < t2->date();
                case lab_assay_rbc_table_t::recent_first:
                    return t1->date() > t2->date();
            }
            return t1->date() < t2->date(); // g++-10
        }
        else
            return k1 < k2;
    });

    std::vector<hidb::lab_assay_rbc_table_t> by_lab;
    for (const auto& table : tables_of_indexes) {
        const acmacs::chart::Assay assay{table->assay()};
        const acmacs::chart::RbcSpecies rbc{table->rbc()};
        if (by_lab.empty() || by_lab.back().lab != table->lab() || by_lab.back().assay != assay || by_lab.back().rbc != rbc)
            by_lab.emplace_back(table->lab(), assay, rbc, table);
        else
            by_lab.back().tables.push_back(table);
    }
    return by_lab;

} // hidb::Tables::sorted

// ----------------------------------------------------------------------

std::string hidb::Table::name() const
{
    return acmacs::string::join(acmacs::string::join_colon, lab(), assay(), acmacs::chart::BLineage{mTable->lineage}.to_string(), rbc(), date());

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

hidb::AntigenIndexList hidb::Table::antigens() const
{
    return AntigenIndexList{mTable->antigen_begin(), mTable->antigen_end()};

} // hidb::Table::antigens

// ----------------------------------------------------------------------

hidb::SerumIndexList hidb::Table::sera() const
{
    return SerumIndexList{mTable->serum_begin(), mTable->serum_end()};

} // hidb::Table::sera

// ----------------------------------------------------------------------

hidb::AntigenIndexList hidb::Table::reference_antigens(const HiDb& aHidb) const
{
      // antigens with names (without annotations and reassortant) that match serum name (without annotations and reassortant) in the same table are reference
      // there is minor possibility that test antigen with the same name present, it becomes false positive
    auto sera = aHidb.sera();
    std::vector<std::string> serum_names(mTable->number_of_sera());
    std::transform(mTable->serum_begin(), mTable->serum_end(), serum_names.begin(),
                   [&sera](size_t serum_index) -> std::string {
                       auto serum = sera->at(SerumIndex{serum_index});
                       return *serum->name();
                         // return string::join(acmacs::string::join_space, {serum->name(), string::join(acmacs::string::join_space, serum->annotations()), serum->reassortant()});
                   });
      // AD_DEBUG("sera: {}", serum_names);

    AntigenIndexList result;
    auto antigens = aHidb.antigens();
    for (const auto* antigen_index = mTable->antigen_begin(); antigen_index != mTable->antigen_end(); ++antigen_index) {
        auto antigen = antigens->at(AntigenIndex{*antigen_index});
        const auto antigen_name = antigen->name();
          // const auto antigen_name = string::join(acmacs::string::join_space, {antigen->name(), string::join(acmacs::string::join_space, antigen->annotations()), antigen->reassortant()});
        if (std::find(serum_names.begin(), serum_names.end(), *antigen_name) != serum_names.end())
            result.push_back(AntigenIndex{*antigen_index});
    }
    return result;

} // hidb::Table::reference_antigens

// ----------------------------------------------------------------------

std::shared_ptr<hidb::Table> hidb::Tables::most_recent(const TableIndexList& aTables) const
{
    const auto* index = reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex);
    auto table_date_compare = [this, index](TableIndex i1, TableIndex i2) -> bool {
        const auto t1{reinterpret_cast<const bin::Table*>(this->mTable0 + index[*i1])}, t2{reinterpret_cast<const bin::Table*>(this->mTable0 + index[*i2])};
        return t1->date() < t2->date();
    };
    return operator[](*std::max_element(aTables.begin(), aTables.end(), table_date_compare));

} // hidb::Tables::most_recent

// ----------------------------------------------------------------------

std::shared_ptr<hidb::Table> hidb::Tables::oldest(const TableIndexList& aTables) const
{
    const auto* index = reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex);
    auto table_date_compare = [this, index](TableIndex i1, TableIndex i2) -> bool {
        const auto t1{reinterpret_cast<const bin::Table*>(this->mTable0 + index[*i1])}, t2{reinterpret_cast<const bin::Table*>(this->mTable0 + index[*i2])};
        return t1->date() < t2->date();
    };
    return operator[](*std::min_element(aTables.begin(), aTables.end(), table_date_compare));

} // hidb::Tables::oldest

// ----------------------------------------------------------------------

std::vector<hidb::TableStat> hidb::Tables::stat(const TableIndexList& tables) const
{
    std::vector<hidb::TableStat> result;
    for (auto table_no : tables) {
        auto table = operator[](table_no);
        const auto assay = table->assay(), lab = table->lab(), rbc = table->rbc();
        if (auto found = std::find_if(std::begin(result), std::end(result), [assay, lab, rbc](const auto& entry) { return assay == entry.assay && lab == entry.lab && rbc == entry.rbc; }); found != std::end(result)) {
            ++found->number;
            if (table->date() > found->most_recent->date())
                found->most_recent = table;
            else if (table->date() < found->oldest->date())
                found->oldest = table;
        }
        else
            result.emplace_back(assay, lab, rbc, table);
    }
    return result;

} // hidb::Tables::stat

// ----------------------------------------------------------------------

std::string hidb::TableStat::title() const noexcept
{
    std::string result = acmacs::string::concat(lab, ':', assay);
    if (assay == "HI") {
        if (rbc == "turkey")
            result += ":tu";
        else if (rbc == "guinea-pig")
            result += ":gp";
        else
            result.append(1, ':').append(rbc);
    }
    else
        result += "   ";
    return result;

} // hidb::TableStat::title

// ----------------------------------------------------------------------

using offset_t = const hidb::bin::ast_offset_t*;

struct first_last_t
{
        first_last_t() : first{nullptr}, last{nullptr} {}
        first_last_t(offset_t a, offset_t b) : first(a), last(b) {}
        first_last_t(offset_t aFirst, size_t aNumber) : first(aFirst), last(aFirst + aNumber) {}
        constexpr size_t size() const { return static_cast<size_t>(last - first); }
        constexpr bool empty() const { return first == last; }
        constexpr auto begin() const { return first; }
        constexpr auto end() const { return last; }
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

template <typename AgSr, typename S> inline first_last_t find_by(first_last_t first_last, const char* aData, S location, S isolation, S year, hidb::find_fuzzy fuzzy)
{
    first_last = filter_by(first_last, [aData](hidb::bin::ast_offset_t offset) { return reinterpret_cast<const AgSr*>(aData + offset)->location(); }, location);
    if (!isolation.empty()) {
        const auto first_last_saved = first_last;
        first_last = filter_by(first_last, [aData](hidb::bin::ast_offset_t offset) { return reinterpret_cast<const AgSr*>(aData + offset)->isolation(); }, isolation);
        if (first_last.empty() && fuzzy == hidb::find_fuzzy::yes) // try isolation as prefix
            first_last = filter_by(first_last_saved, [aData,size=isolation.size()](hidb::bin::ast_offset_t offset) { return reinterpret_cast<const AgSr*>(aData + offset)->isolation().substr(0, size); }, isolation);
    }
    if (!year.empty())
        first_last = filter_by(first_last, [aData](hidb::bin::ast_offset_t offset) { return reinterpret_cast<const AgSr*>(aData + offset)->year(); }, year);
    return first_last;
}

// ----------------------------------------------------------------------

hidb::AntigenIndexList hidb::Antigens::find(std::string_view aName, fix_location aFixLocation, find_fuzzy fuzzy) const
{
    const first_last_t all_antigens(reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex), mNumberOfAntigens);
    first_last_t first_last;
    try {
        std::string virus_type, host, location, isolation, year, passage, extra;
        virus_name::split_with_extra(aName, virus_type, host, location, isolation, year, passage, extra);
        if (aFixLocation == fix_location::yes)
            location = acmacs::locationdb::get().find_or_throw(location).name;
        first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, location, isolation, year, fuzzy);
    }
    catch (virus_name::Unrecognized&) {
        if (aName.size() > 3 && aName[2] == ' ') // cdc name?
            first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, std::string_view(aName.data(), 2), std::string_view(aName.data() + 3), std::string_view{}, fuzzy);
        if (first_last.empty()) {
            const auto parts = acmacs::string::split(aName, "/");
            switch (parts.size()) {
                case 1: // just location?
                    first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, parts[0], std::string_view{}, std::string_view{}, fuzzy);
                    break;
                case 2: // location/isolation
                    first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, parts[0], parts[1], std::string_view{}, fuzzy);
                    break;
                case 3: // host/location/isolation?
                    first_last = find_by<hidb::bin::Antigen>(all_antigens, mAntigen0, parts[1], parts[2], std::string_view{}, fuzzy);
                    break;
                default: // ?
                    AD_WARNING("don't know how to split: {}", aName);
                    break;
            }
        }
    }

    AntigenIndexList result(first_last.size());
    std::transform(first_last.first, first_last.last, result.begin(),
                   [index_begin = all_antigens.first](const hidb::bin::ast_offset_t& offset) { return AntigenIndex{&offset - index_begin}; });
    return result;

} // hidb::Antigens::find

// ----------------------------------------------------------------------

hidb::SerumIndexList hidb::Sera::find(std::string_view aName, fix_location aFixLocation, find_fuzzy fuzzy) const
{
    const first_last_t all_sera(reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex), mNumberOfSera);
    first_last_t first_last;
    std::string location;
    try {
        std::string virus_type, host, isolation, year, passage, extra;
        virus_name::split_with_extra(aName, virus_type, host, location, isolation, year, passage, extra);
        if (aFixLocation == fix_location::yes)
            location = acmacs::locationdb::get().find_or_throw(location).name;
        first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, location, isolation, year, fuzzy);
    }
    catch (virus_name::Unrecognized&) {
        const auto parts = acmacs::string::split(aName, "/");
        switch (parts.size()) {
          case 1:           // just location?
              first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, parts[0], std::string_view{}, std::string_view{}, fuzzy);
              break;
          case 2:           // location/isolation
              location = aFixLocation == hidb::fix_location::yes ? acmacs::locationdb::get().find_or_throw(std::string(parts[0])).name : std::string(parts[0]);
              first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, std::string_view(location), parts[1], std::string_view{}, fuzzy);
              break;
          case 3:          // host/location/isolation?
              location = aFixLocation == hidb::fix_location::yes ? acmacs::locationdb::get().find_or_throw(std::string(parts[1])).name : std::string(parts[1]);
              first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, std::string_view(location), parts[2], std::string_view{}, fuzzy);
              break;
          default:          // ?
              AD_WARNING("don't know how to split: {}", aName);
              break;
        }
    }

    SerumIndexList result(first_last.size());
    std::transform(first_last.first, first_last.last, result.begin(),
                   [index_begin=all_sera.first](const hidb::bin::ast_offset_t& offset) { return SerumIndex{&offset - index_begin}; });
    return result;

} // hidb::Sera::find

// ----------------------------------------------------------------------

hidb::AntigenPList hidb::Antigens::find_labid(std::string_view labid) const
{
    AntigenPList result;

    auto find = [&result, this](std::string look_for) -> void {
        const first_last_t all_antigens(reinterpret_cast<const hidb::bin::ast_offset_t*>(this->mIndex), this->mNumberOfAntigens);
        for (auto ag = all_antigens.first; ag != all_antigens.last; ++ag) {
            const auto lab_ids = reinterpret_cast<const hidb::bin::Antigen*>(this->mAntigen0 + *ag)->lab_ids();
            if (const auto found = std::find(lab_ids.begin(), lab_ids.end(), look_for); found != lab_ids.end())
                result.push_back(std::make_shared<Antigen>(this->mAntigen0 + *ag, mHiDb));
        }
    };

    if (labid.find('#') == std::string_view::npos) {
        find(fmt::format("CDC#{}", labid));
        if (result.empty())
            find(fmt::format("MELB#{}", labid));
        if (result.empty())
            find(fmt::format("NIID#{}", labid));
    }
    if (result.empty())
        find(std::string{labid});

    return result;

} // hidb::Antigens::find_labid

// ----------------------------------------------------------------------

// for seqdb-3, to speed up looking by lab_id
std::vector<std::pair<std::string_view, const hidb::bin::Antigen*>> hidb::Antigens::sorted_by_labid() const
{
    std::vector<std::pair<std::string_view, const hidb::bin::Antigen*>> result;
    const first_last_t all_antigens(reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex), mNumberOfAntigens);
    for (const auto& ag : all_antigens) {
        const auto* antigen = reinterpret_cast<const hidb::bin::Antigen*>(mAntigen0 + ag);
        for (const auto& lab_id : antigen->lab_ids())
            result.emplace_back(lab_id, antigen);
    }
    std::sort(std::begin(result), std::end(result), [](const auto& e1, const auto& e2) { return e1.first < e2.first; });
    return result;

} // hidb::Antigens::sorted_by_labid

// ----------------------------------------------------------------------

std::optional<hidb::AntigenPIndex> hidb::Antigens::find(const acmacs::chart::Antigen& aAntigen, passage_strictness aPassageStrictness) const
{
    if (aAntigen.annotations().distinct()) // distinct antigens are not stored
        throw not_found(aAntigen.full_name());
    const auto antigen_index_list = find(aAntigen.name(), hidb::fix_location::no);
    const auto ignore_passage = [aPassageStrictness, &aAntigen]() -> bool {
        switch (aPassageStrictness) {
            case passage_strictness::yes:
                return false;
            case passage_strictness::ignore_if_empty:
                return aAntigen.passage().empty();
            case passage_strictness::ignore:
                return true;
        }
        return false; // hey g++-10
    }();
    for (auto antigen_index : antigen_index_list) {
        auto antigen = at(antigen_index);
        if (antigen->annotations() == aAntigen.annotations() && antigen->reassortant() == aAntigen.reassortant() && (ignore_passage || antigen->passage() == aAntigen.passage()))
            return std::make_pair(antigen, antigen_index);
    }
    return std::nullopt;

} // hidb::Antigens::find

// ----------------------------------------------------------------------

hidb::AntigenPList hidb::Antigens::find(const acmacs::chart::Antigens& aAntigens) const
{
    hidb::AntigenPList result;
    for (auto antigen : aAntigens) {
        if (const auto found = find(*antigen); found.has_value())
            result.push_back(found->first);
        else
            result.emplace_back(nullptr);
    }
    return result;

} // hidb::Antigens::find

// ----------------------------------------------------------------------

hidb::AntigenPList hidb::Antigens::find(const acmacs::chart::Antigens& aAntigens, const acmacs::chart::Indexes& indexes) const
{
    hidb::AntigenPList result;
    for (auto antigen_no: indexes) {
        if (const auto found = find(*aAntigens[antigen_no]); found.has_value())
            result.push_back(found->first);
        else
            result.emplace_back(nullptr);
    }
    return result;

} // hidb::Antigens::find

// ----------------------------------------------------------------------

std::optional<hidb::SerumPIndex> hidb::Sera::find(const acmacs::chart::Serum& aSerum) const
{
    const auto serum_index_list = find(aSerum.name(), hidb::fix_location::no);
    std::optional<SerumPIndex> with_unknown_serum_id;
    for (auto serum_index: serum_index_list) {
        auto serum = at(serum_index);
        if (serum->annotations() == aSerum.annotations() && serum->reassortant() == aSerum.reassortant()) {
            if (serum->serum_id() == aSerum.serum_id())
                return hidb::SerumPIndex{serum, serum_index};
            else if (serum->serum_id().empty() && aSerum.serum_id() == acmacs::chart::SerumId{"UNKNOWN"}) // old tables may have UNKNOWN serum id, but database stores empty serum_id
                with_unknown_serum_id = hidb::SerumPIndex{serum, serum_index};
        }
    }
    if (with_unknown_serum_id.has_value())
        return *with_unknown_serum_id;
    return std::nullopt;

} // hidb::Sera::find

// ----------------------------------------------------------------------

hidb::SerumPList hidb::Sera::find(const acmacs::chart::Sera& aSera) const
{
    hidb::SerumPList result;
    for (auto serum: aSera) {
        if (const auto found = find(*serum); found)
            result.push_back(found->first);
        else
            result.emplace_back(nullptr);
    }
    return result;

} // hidb::Sera::find

// ----------------------------------------------------------------------

hidb::SerumPList hidb::Sera::find(const acmacs::chart::Sera& aSera, const acmacs::chart::Indexes& indexes) const
{
    hidb::SerumPList result;
    for (auto serum_no: indexes) {
        if (const auto found = find(*aSera[serum_no]); found)
            result.push_back(found->first);
        else
            result.emplace_back(nullptr);
    }
    return result;

} // hidb::Sera::find

// ----------------------------------------------------------------------

hidb::AntigenPList hidb::Antigens::date_range(std::string_view first, std::string_view after_last) const
{
    hidb::AntigenPList result;
    const auto min_date = !first.empty() ? hidb::bin::Antigen::make_date(first) : hidb::bin::Antigen::min_date();
    const auto max_date = !after_last.empty() ? hidb::bin::Antigen::make_date(after_last) : hidb::bin::Antigen::max_date();
    const first_last_t all_antigens(reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex), mNumberOfAntigens);
    for (auto ag = all_antigens.first; ag != all_antigens.last; ++ag) {
        const auto date = reinterpret_cast<const hidb::bin::Antigen*>(this->mAntigen0 + *ag)->date_raw();
        if (date >= min_date && date < max_date)
            result.push_back(std::make_shared<Antigen>(this->mAntigen0 + *ag, mHiDb));
    }
    return result;

} // hidb::Antigens::date_range

// ----------------------------------------------------------------------

hidb::SerumPList hidb::Sera::find_homologous(size_t aAntigenIndex, const Antigen& aAntigen) const
{
    const first_last_t all_sera(reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex), mNumberOfSera);
    const std::string antigen_year(aAntigen.year());
    const first_last_t first_last = find_by<hidb::bin::Serum>(all_sera, mSerum0, aAntigen.location(), aAntigen.isolation(), std::string_view(antigen_year), find_fuzzy::no);
    hidb::SerumPList result;
    for (auto offset_p = first_last.first; offset_p != first_last.last; ++offset_p) {
        const auto [num_homologous, first_homologous_p] = reinterpret_cast<const hidb::bin::Serum*>(mSerum0 + *offset_p)->homologous_antigens();
        if (std::find(first_homologous_p, first_homologous_p + num_homologous, aAntigenIndex) != (first_homologous_p + num_homologous)) {
            result.push_back(std::make_shared<hidb::Serum>(mSerum0 + *offset_p, mHiDb));
        }
    }
    // AD_DEBUG("find_homologous {} \"{}\" {}", aAntigenIndex, aAntigen.full_name(), result.size());
    return result;

} // hidb::Sera::find_homologous

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
