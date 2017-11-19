#pragma once

#include <vector>
#include <memory>

#include "acmacs-base/string.hh"

namespace acmacs::chart { class Chart; }

// ----------------------------------------------------------------------

using Indexes = std::vector<size_t>;

class Table;
class Antigen;
class Serum;

using AntigenPtrs = std::vector<const Antigen*>;
using SerumPtrs = std::vector<const Serum*>;
using TablePtrs = std::vector<const Table*>;

// ----------------------------------------------------------------------

class Titers : public std::vector<std::vector<std::string>>
{
}; // class Titers

// ----------------------------------------------------------------------

class Table
{
 public:
    std::string virus;
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

    inline bool operator==(const Table& rhs) const { return string::compare(
        {    virus,     virus_type,     subset,     assay,     lab,     rbc_species,     date},
        {rhs.virus, rhs.virus_type, rhs.subset, rhs.assay, rhs.lab, rhs.rbc_species, rhs.date}) == 0; }

    inline bool operator<(const Table& rhs) const { return string::compare(
        {    virus,     virus_type,     subset,     assay,     lab,     rbc_species,     date},
        {rhs.virus, rhs.virus_type, rhs.subset, rhs.assay, rhs.lab, rhs.rbc_species, rhs.date}) < 0; }

}; // class Table

// ----------------------------------------------------------------------

class Tables : public std::vector<std::unique_ptr<Table>>
{
 public:
    inline void sort() { std::sort(begin(), end(), [](const auto& a, const auto& b) -> bool { return *a < *b; }); }

}; // class Tables

// ----------------------------------------------------------------------

class Annotations : public std::vector<std::string>
{
}; // class Annotations

// ----------------------------------------------------------------------

class Dates : public std::vector<std::string>
{
}; // class Dates

// ----------------------------------------------------------------------

class LabIds : public std::vector<std::string>
{
}; // class LabIds

// ----------------------------------------------------------------------

class Antigen
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
    std::string lineage;
    Indexes tables;

    TablePtrs table_ptrs;

    inline bool operator==(const Antigen& rhs) const { return string::compare(
        {    virus_type,     host,     location,     isolation,     year, string::join(" ",     annotations),     reassortant,     passage},
        {rhs.virus_type, rhs.host, rhs.location, rhs.isolation, rhs.year, string::join(" ", rhs.annotations), rhs.reassortant, rhs.passage}) == 0; }

    inline bool operator<(const Antigen& rhs) const { return string::compare(
        {    virus_type,     host,     location,     isolation,     year, string::join(" ",     annotations),     reassortant,     passage},
        {rhs.virus_type, rhs.host, rhs.location, rhs.isolation, rhs.year, string::join(" ", rhs.annotations), rhs.reassortant, rhs.passage}) < 0; }

}; // class Antigen

// ----------------------------------------------------------------------

class Antigens : public std::vector<std::unique_ptr<Antigen>>
{
 public:
    inline void sort() { std::sort(begin(), end(), [](const auto& a, const auto& b) -> bool { return *a < *b; }); }

}; // class Antigens

// ----------------------------------------------------------------------

class Serum
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
    Indexes tables;

    TablePtrs table_ptrs;

    inline bool operator==(const Serum& rhs) const { return string::compare(
        {    virus_type,     host,     location,     isolation,     year, string::join(" ",     annotations),     reassortant,     serum_id},
        {rhs.virus_type, rhs.host, rhs.location, rhs.isolation, rhs.year, string::join(" ", rhs.annotations), rhs.reassortant, rhs.serum_id}) == 0; }

    inline bool operator<(const Serum& rhs) const { return string::compare(
        {    virus_type,     host,     location,     isolation,     year, string::join(" ",     annotations),     reassortant,     serum_id},
        {rhs.virus_type, rhs.host, rhs.location, rhs.isolation, rhs.year, string::join(" ", rhs.annotations), rhs.reassortant, rhs.serum_id}) < 0; }

}; // class Serum

// ----------------------------------------------------------------------

class Sera : public std::vector<std::unique_ptr<Serum>>
{
 public:
    inline void sort() { std::sort(begin(), end(), [](const auto& a, const auto& b) -> bool { return *a < *b; }); }

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
