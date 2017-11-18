#pragma once

#include <string>
#include <vector>
#include <memory>

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

}; // class Table

// ----------------------------------------------------------------------

class Tables : public std::vector<std::unique_ptr<Table>>
{
}; // class Tables

// ----------------------------------------------------------------------

class Annotations : public std::vector<std::string>
{
}; // class Annotations

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
    std::string date;
    LabIds lab_ids;
    std::string lineage;
    Indexes tables;

    TablePtrs table_ptrs;

}; // class Antigen

// ----------------------------------------------------------------------

class Antigens : public std::vector<std::unique_ptr<Antigen>>
{
}; // class Antigens

// ----------------------------------------------------------------------

class Serum
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
    std::string serum_id;
    std::string serum_sepcies;
    std::string lineage;
    Indexes tables;

    TablePtrs table_ptrs;

}; // class Serum

// ----------------------------------------------------------------------

class Sera : public std::vector<std::unique_ptr<Serum>>
{
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
