#pragma once

#include <vector>
#include <set>
#include <functional>
#include <memory>

#include "acmacs-base/string.hh"

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
    constexpr inline bool operator()(const std::unique_ptr<T>& lhs, const std::unique_ptr<T>& rhs) const
        {
            return *lhs < *rhs;
        }

}; // class less_unique_ptr<>

template <typename T> class set_unique_ptr : public std::set<std::unique_ptr<T>, less_unique_ptr<T>>
{
}; // class set_unique_ptr <>

// ----------------------------------------------------------------------

class Virus : public std::string
{
 public:
    inline Virus(std::string aSource) : std::string{aSource == "influenza" ? std::string{} : aSource} {}

}; // class Virus

// ----------------------------------------------------------------------

class Lineage : public std::string
{
 public:
    inline Lineage(std::string aSource) : std::string{aSource == "UNKNOWN" ? std::string{} : aSource} {}

}; // class Lineage

// ----------------------------------------------------------------------

class Titers : public std::vector<std::vector<std::string>>
{
}; // class Titers

// ----------------------------------------------------------------------

class Table
{
 public:
    Virus virus;
    std::string virus_type;
    std::string subset;
    std::string assay;
    std::string date;
    std::string lab;
    std::string rbc_species;
    Indexes antigens;
    Indexes sera;
    Titers titers;

    AntigenPtrs antigen_ptrs;
    SerumPtrs serum_ptrs;

    Table(const acmacs::chart::Info& aInfo);

    inline bool operator==(const Table& rhs) const { return string::compare(
        {    virus,     virus_type,     subset,     assay,     lab,     rbc_species,     date},
        {rhs.virus, rhs.virus_type, rhs.subset, rhs.assay, rhs.lab, rhs.rbc_species, rhs.date}) == 0; }

    inline bool operator<(const Table& rhs) const { return string::compare(
        {    virus,     virus_type,     subset,     assay,     lab,     rbc_species,     date},
        {rhs.virus, rhs.virus_type, rhs.subset, rhs.assay, rhs.lab, rhs.rbc_species, rhs.date}) < 0; }

    void set_titers(const acmacs::chart::Titers& aTiters);
    inline void add_antigen(Antigen* aAntigen) { antigen_ptrs.insert(aAntigen); }
    inline void add_serum(Serum* aSerum) { serum_ptrs.insert(aSerum); }

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

    virtual ~AntigenSerum();
    virtual std::string to_string() const = 0;
    virtual std::string type_name() const = 0;

    void add_table(Table *aTable);

}; // class AntigenSerum

namespace acmacs
{
    inline std::string to_string(const AntigenSerum& aAntigenSerum) { return aAntigenSerum.to_string(); }
}

// ----------------------------------------------------------------------

class Antigen : public AntigenSerum
{
 public:
    std::string virus_type;    // empty for cdc name
    std::string host;          // empty if HUMAN
    std::string location;      // cdc_abbreviation in case of cdc name
    std::string isolation;     // name in case of cdc name
    std::string year;          // empty for cdc name
    std::string reassortant;
    std::string passage;
    Annotations annotations;
    Dates dates;
    LabIds lab_ids;
    Lineage lineage;

    Antigen(const acmacs::chart::Antigen& aAntigen);

    inline bool operator==(const Antigen& rhs) const { return string::compare(
        {    virus_type,     host,     location,     isolation,     year, string::join(" ",     annotations),     reassortant,     passage},
        {rhs.virus_type, rhs.host, rhs.location, rhs.isolation, rhs.year, string::join(" ", rhs.annotations), rhs.reassortant, rhs.passage}) == 0; }
    inline bool operator!=(const Antigen& rhs) const { return !operator==(rhs); }

    inline bool operator<(const Antigen& rhs) const { return string::compare(
        {    virus_type,     host,     location,     isolation,     year, string::join(" ",     annotations),     reassortant,     passage},
        {rhs.virus_type, rhs.host, rhs.location, rhs.isolation, rhs.year, string::join(" ", rhs.annotations), rhs.reassortant, rhs.passage}) < 0; }

    template <typename Iter> inline void add_lab_id(Iter first, Iter last) { for (; first != last; ++first) lab_ids.insert(*first); }
    inline void add_date(std::string aSource) { if (!aSource.empty()) dates.insert(aSource); }

    inline std::string type_name() const override { return "Antigen"; }
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
    std::string virus_type;
    std::string host;          // empty if HUMAN
    std::string location;
    std::string isolation;
    std::string year;
    std::string reassortant;
    std::string passage;
    Annotations annotations;
    std::string serum_id;
    std::string serum_sepcies;
    std::string lineage;

    Serum(const acmacs::chart::Serum& aSerum);

    inline bool operator==(const Serum& rhs) const { return string::compare(
        {    virus_type,     host,     location,     isolation,     year, string::join(" ",     annotations),     reassortant,     serum_id},
        {rhs.virus_type, rhs.host, rhs.location, rhs.isolation, rhs.year, string::join(" ", rhs.annotations), rhs.reassortant, rhs.serum_id}) == 0; }
    inline bool operator!=(const Serum& rhs) const { return !operator==(rhs); }

    inline bool operator<(const Serum& rhs) const { return string::compare(
        {    virus_type,     host,     location,     isolation,     year, string::join(" ",     annotations),     reassortant,     serum_id},
        {rhs.virus_type, rhs.host, rhs.location, rhs.isolation, rhs.year, string::join(" ", rhs.annotations), rhs.reassortant, rhs.serum_id}) < 0; }

    inline std::string type_name() const override { return "Serum"; }
    std::string to_string() const override;

}; // class Serum

// ----------------------------------------------------------------------

class Sera : public set_unique_ptr<Serum>
{
 public:

}; // class Sera

// ----------------------------------------------------------------------

class HidbMaker
{
 public:
    inline HidbMaker() = default;

    void add(const acmacs::chart::Chart& aChart);
    void save(std::string aFilename);

 private:
    Antigens mAntigens;
    Sera mSera;
    Tables mTables;

}; // class HidbMaker

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
