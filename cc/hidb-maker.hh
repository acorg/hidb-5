#pragma once

#include <vector>
#include <set>
#include <functional>
#include <memory>

#include "acmacs-base/string.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-base/rjson-forward.hh"
#include "acmacs-chart-2/chart.hh"

namespace acmacs::chart
{
    class Chart;
    class Info;
    class Titers;
    class Antigen;
    class Serum;

} // namespace acmacs::chart

// ----------------------------------------------------------------------

using Indexes = std::vector<size_t>;

class Table;
class Antigen;
class Serum;

using AntigenPtrs = std::set<const Antigen*>;
using SerumPtrs = std::set<const Serum*>;
using TablePtrs = std::set<const Table*>;

// ----------------------------------------------------------------------

template <typename T> class less_unique_ptr
{
 public:
    constexpr bool operator()(const std::unique_ptr<T>& lhs, const std::unique_ptr<T>& rhs) const
        {
            return *lhs < *rhs;
        }

}; // class less_unique_ptr<>

template <typename T> class set_unique_ptr : public std::set<std::unique_ptr<T>, less_unique_ptr<T>>
{
  public:
    void make_index()
    {
        for (auto [index, entry] : acmacs::enumerate(*this))
            entry->index = index;
    }

    void make_indexes()
    {
        for (auto& entry : *this)
            entry->make_indexes();
    }

    const T* operator[](size_t index) const { return std::next(this->begin(), static_cast<ssize_t>(index))->get(); }

}; // class set_unique_ptr <>

// ----------------------------------------------------------------------

using Virus = acmacs::chart::Virus;

// ----------------------------------------------------------------------

class Assay : public acmacs::chart::Assay
{
 public:
    Assay(const acmacs::chart::Assay& aSource) : acmacs::chart::Assay{aSource}
        {
            if (aSource == acmacs::chart::Assay{"PLAQUE REDUCTION NEUTRALISATION"})
                assign("PRN");
            else if (aSource == acmacs::chart::Assay{"FOCUS REDUCTION"})
                assign("FR");
        }

}; // class Assay

// ----------------------------------------------------------------------

class Titers : public std::vector<std::vector<std::string>>
{
}; // class Titers

// ----------------------------------------------------------------------

class Table
{
 public:
    Virus virus;
    acmacs::virus::type_subtype_t virus_type;
    std::string subset;
    acmacs::chart::Assay assay;
    std::string date;
    acmacs::Lab lab;
    acmacs::chart::RbcSpecies rbc_species;
    acmacs::virus::lineage_t lineage;
    Indexes antigens;
    Indexes sera;
    Titers titers;

    AntigenPtrs antigen_ptrs;
    SerumPtrs serum_ptrs;
    size_t index;

    Table(const acmacs::chart::Info& aInfo);

    bool operator==(const Table& rhs) const { return string::compare(
        {    *virus,     *virus_type,     subset,     lineage, *assay,     *lab,     *rbc_species,     date},
        {*rhs.virus, *rhs.virus_type, rhs.subset, rhs.lineage, *rhs.assay, *rhs.lab, *rhs.rbc_species, rhs.date}) == 0; }

    bool operator<(const Table& rhs) const { return string::compare(
        {    *virus,     *virus_type,     subset,     lineage,     *assay,     *lab,     *rbc_species,     date},
        {*rhs.virus, *rhs.virus_type, rhs.subset, rhs.lineage, *rhs.assay, *rhs.lab, *rhs.rbc_species, rhs.date}) < 0; }

    void set_titers(const acmacs::chart::Titers& aTiters);
    void add_antigen(Antigen* aAntigen) { antigen_ptrs.insert(aAntigen); }
    void add_serum(Serum* aSerum) { serum_ptrs.insert(aSerum); }
    void make_indexes();

}; // class Table

namespace acmacs
{
    std::string to_string(const Table& aTable);
}

// ----------------------------------------------------------------------

class Tables : public set_unique_ptr<Table>
{
 public:
    Table* add(const acmacs::chart::Chart& aChart);

}; // class Tables

// ----------------------------------------------------------------------

class Annotations : public std::vector<std::string>
{
}; // class Annotations

// ----------------------------------------------------------------------

class Dates : public std::set<std::string>
{
}; // class Dates

// ----------------------------------------------------------------------

class LabIds : public std::set<std::string>
{
}; // class LabIds

// ----------------------------------------------------------------------

class AntigenSerum
{
 public:
    Indexes tables;
    TablePtrs table_ptrs;
    size_t index;

    std::string host;          // empty if HUMAN
    std::string virus_type;    // empty for cdc name
    std::string location;      // cdc_abbreviation in case of cdc name
    std::string isolation;     // name in case of cdc name
    std::string year;          // empty for cdc name
    std::string reassortant;
    std::string passage;
    Annotations annotations;
    acmacs::virus::lineage_t lineage{};

    AntigenSerum(std::string_view a_reassortant, std::string_view a_passage, const std::vector<std::string>& a_annotations, std::string_view aLineage)
        : reassortant{a_reassortant}, passage{a_passage}, annotations{a_annotations} { update_lineage(acmacs::virus::lineage_t{aLineage}); }
    virtual ~AntigenSerum();
    virtual std::string to_string() const = 0;
    virtual std::string type_name() const = 0;

    void add_table(Table *aTable);
    virtual void make_indexes();

    void update_lineage(const acmacs::virus::lineage_t& aLineage)
        {
            if (!aLineage.empty()) {
                const acmacs::virus::lineage_t new_lineage{aLineage->substr(0, 1)};
                if (lineage.empty())
                    lineage = new_lineage;
                else if (lineage != new_lineage)
                    fmt::print(stderr, "WARNING: conflicting lineages for {}/{}/{}/{}: {} vs. {}\n", virus_type, location, isolation , year, lineage, aLineage);
            }
        }

}; // class AntigenSerum

namespace acmacs
{
    inline std::string to_string(const AntigenSerum& aAntigenSerum) { return aAntigenSerum.to_string(); }
}

// ----------------------------------------------------------------------

class Antigen : public AntigenSerum
{
 public:
    Dates dates;
    LabIds lab_ids;

    Antigen(const acmacs::chart::Antigen& aAntigen);

    bool operator==(const Antigen& rhs) const { return string::compare(
        {    virus_type,     host,     location,     isolation,     year, acmacs::string::join(acmacs::string::join_space,     annotations),     reassortant,     passage},
        {rhs.virus_type, rhs.host, rhs.location, rhs.isolation, rhs.year, acmacs::string::join(acmacs::string::join_space, rhs.annotations), rhs.reassortant, rhs.passage}) == 0; }
    bool operator!=(const Antigen& rhs) const { return !operator==(rhs); }

    bool operator<(const Antigen& rhs) const { return string::compare(
        {    location,     isolation,     year,     host, acmacs::string::join(acmacs::string::join_space,     annotations),     reassortant,     passage},
        {rhs.location, rhs.isolation, rhs.year, rhs.host, acmacs::string::join(acmacs::string::join_space, rhs.annotations), rhs.reassortant, rhs.passage}) < 0; }

    template <typename Iter> void add_lab_id(Iter first, Iter last) { for (; first != last; ++first) lab_ids.insert(*first); }
    void add_date(std::string aSource) { if (!aSource.empty()) dates.insert(aSource); }

    std::string type_name() const override { return "Antigen"; }
    std::string to_string() const override;

}; // class Antigen

// ----------------------------------------------------------------------

class Antigens : public set_unique_ptr<Antigen>
{
 public:
    Antigen* add(const acmacs::chart::Antigen& aAntigen);

}; // class Antigens

// ----------------------------------------------------------------------

class Serum : public AntigenSerum
{
 public:
    std::string serum_id;
    std::string serum_species;
    Indexes homologous;
    AntigenPtrs homologous_ptrs;

    Serum(const acmacs::chart::Serum& aSerum);

    bool operator==(const Serum& rhs) const { return string::compare(
        {    virus_type,     host,     location,     isolation,     year, acmacs::string::join(acmacs::string::join_space,     annotations),     reassortant,     serum_id},
        {rhs.virus_type, rhs.host, rhs.location, rhs.isolation, rhs.year, acmacs::string::join(acmacs::string::join_space, rhs.annotations), rhs.reassortant, rhs.serum_id}) == 0; }
    bool operator!=(const Serum& rhs) const { return !operator==(rhs); }

    bool operator<(const Serum& rhs) const { return string::compare(
        {    location,     isolation,     year,     host, acmacs::string::join(acmacs::string::join_space,     annotations),     reassortant,     serum_id},
        {rhs.location, rhs.isolation, rhs.year, rhs.host, acmacs::string::join(acmacs::string::join_space, rhs.annotations), rhs.reassortant, rhs.serum_id}) < 0; }

    std::string type_name() const override { return "Serum"; }
    std::string to_string() const override;
    void make_indexes() override;

}; // class Serum

// ----------------------------------------------------------------------

class Sera : public set_unique_ptr<Serum>
{
 public:
    Serum* add(const acmacs::chart::Serum& aSerum);

}; // class Sera

// ----------------------------------------------------------------------

class HidbMaker
{
 public:
    HidbMaker() = default;

    void add(const acmacs::chart::Chart& aChart);
    void save(std::string_view aFilename);

 private:
    Antigens mAntigens;
    Sera mSera;
    Tables mTables;

    void make_index();
    void export_antigens(rjson::value& target) const;
    void export_sera(rjson::value& target) const;
    void export_tables(rjson::value& target) const;

}; // class HidbMaker

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
