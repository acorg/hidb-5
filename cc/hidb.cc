#include "acmacs-base/string.hh"
#include "acmacs-base/read-file.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/hidb-bin.hh"
#include "hidb-5/hidb-json.hh"

// ----------------------------------------------------------------------

hidb::HiDb::HiDb(std::string aFilename, report_time timer)
{
    Timeit ti("reading hidb from " + aFilename + ": ", timer);
    const std::string data = acmacs::file::read(aFilename);
    if (data.find("\"  version\": \"hidb-v5\"") != std::string::npos) {
        mDataStorage = hidb::json::read(data);
        mData = mDataStorage.data();
    }
    else
        throw std::runtime_error("[hidb] unrecognized file: " + aFilename);

} // hidb::HiDb::HiDb

// ----------------------------------------------------------------------

void hidb::HiDb::save(std::string aFilename) const
{
    acmacs::file::write(aFilename, mDataStorage);

} // hidb::HiDb::save

// ----------------------------------------------------------------------

std::shared_ptr<hidb::Antigens> hidb::HiDb::antigens() const
{
    const auto* antigens = mData + reinterpret_cast<const hidb::bin::Header*>(mData)->antigen_offset;
    const auto number_of_antigens = *reinterpret_cast<const hidb::bin::ast_number_t*>(antigens);
    return std::make_shared<hidb::Antigens>(static_cast<size_t>(number_of_antigens),
                                            antigens + sizeof(hidb::bin::ast_number_t),
                                            antigens + sizeof(hidb::bin::ast_number_t) + sizeof(hidb::bin::ast_offset_t) * number_of_antigens);

} // hidb::HiDb::antigens

// ----------------------------------------------------------------------

std::shared_ptr<acmacs::chart::Antigen> hidb::Antigens::operator[](size_t aIndex) const
{
    if (aIndex == 0) {
        return std::make_shared<hidb::Antigen>(mAntigen0);
    }
    else {
        return std::make_shared<hidb::Antigen>(mAntigen0 + reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex)[aIndex - 1]);
    }

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
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->lab_ids();

} // hidb::Antigen::lab_ids

// ----------------------------------------------------------------------

acmacs::chart::Annotations hidb::Antigen::annotations() const
{
    return reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->annotations();

} // hidb::Antigen::annotations

// ----------------------------------------------------------------------

std::vector<size_t> hidb::Antigen::tables() const
{
    const auto [size, ptr] = reinterpret_cast<const hidb::bin::Antigen*>(mAntigen)->tables();
    std::vector<size_t> result(size);
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
                                            sera + sizeof(hidb::bin::ast_number_t) + sizeof(hidb::bin::ast_offset_t) * number_of_sera);

} // hidb::HiDb::sera

// ----------------------------------------------------------------------

std::shared_ptr<acmacs::chart::Serum> hidb::Sera::operator[](size_t aIndex) const
{
    if (aIndex == 0) {
        return std::make_shared<hidb::Serum>(mSerum0);
    }
    else {
        return std::make_shared<hidb::Serum>(mSerum0 + reinterpret_cast<const hidb::bin::ast_offset_t*>(mIndex)[aIndex - 1]);
    }

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
    return reinterpret_cast<const hidb::bin::Serum*>(mSerum)->annotations();

} // hidb::Serum::annotations

// ----------------------------------------------------------------------

acmacs::chart::SerumId hidb::Serum::serum_id() const
{

} // hidb::Serum::serum_id

// ----------------------------------------------------------------------

acmacs::chart::SerumSpecies hidb::Serum::serum_species() const
{

} // hidb::Serum::serum_species

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList hidb::Serum::homologous_antigens() const
{

} // hidb::Serum::homologous_antigens

// ----------------------------------------------------------------------

std::vector<size_t> hidb::Serum::tables() const
{
    const auto [size, ptr] = reinterpret_cast<const hidb::bin::Serum*>(mSerum)->tables();
    std::vector<size_t> result(size);
    std::transform(ptr, ptr + size, result.begin(), [](const auto& index) -> size_t { return static_cast<size_t>(index); });
    return result;

} // hidb::Serum::tables

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: