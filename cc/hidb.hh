#pragma once

#include <string>

#include "acmacs-base/read-file.hh"
#include "acmacs-chart-2/chart.hh"
#include "hidb-5/hidb-set.hh"

// ----------------------------------------------------------------------

class LocDb;

namespace hidb
{
    namespace bin { struct Table; }

    using indexes_t = std::vector<size_t>;
    class not_found : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    enum class find_fuzzy { no, yes };
    enum class fix_location { no, yes };
    enum class passage_strictness { yes, ignore_if_empty };

    class HiDb;

    class Antigen : public acmacs::chart::Antigen
    {
     public:
        Antigen(const char* aAntigen, const HiDb& aHiDb) : mAntigen(aAntigen), mHiDb(aHiDb) {}

        acmacs::chart::Name name() const override;
        acmacs::chart::Date date() const override;
        acmacs::virus::Passage passage() const override;
        acmacs::chart::BLineage lineage() const override;
        acmacs::virus::Reassortant reassortant() const override;
        acmacs::chart::LabIds lab_ids() const override;
        acmacs::chart::Clades clades() const override { return {}; }
        acmacs::chart::Annotations annotations() const override;
        bool reference() const override { return false; }

        indexes_t tables() const;
        size_t number_of_tables() const;

        std::string_view location() const;
        std::string_view isolation() const;
        std::string year() const;
        std::string date_compact() const;

        std::string country(const LocDb& locdb) const;

     private:
        const char* mAntigen;
        const HiDb& mHiDb;

    }; // class Antigen

    using AntigenP = std::shared_ptr<Antigen>;
    using AntigenPList = std::vector<AntigenP>;
    using AntigenPIndex = std::pair<AntigenP, size_t>;
    using AntigenPIndexList = std::vector<AntigenPIndex>;

    class Antigens : public acmacs::chart::Antigens
    {
     public:
        Antigens(size_t aNumberOfAntigens, const char* aIndex, const char* aAntigen0, const HiDb& aHiDb)
            : mNumberOfAntigens(aNumberOfAntigens), mIndex(aIndex), mAntigen0(aAntigen0), mHiDb(aHiDb) {}

        size_t size() const override { return mNumberOfAntigens; }
        std::shared_ptr<acmacs::chart::Antigen> operator[](size_t aIndex) const override { return at(aIndex); }
        AntigenP at(size_t aIndex) const;
        AntigenPIndexList find(std::string aName, fix_location aFixLocation, find_fuzzy fuzzy = find_fuzzy::no) const;
        AntigenPList find_labid(std::string labid) const;
        AntigenPIndex find(const acmacs::chart::Antigen& aAntigen, passage_strictness aPassageStrictness = passage_strictness::yes) const; // hidb::vaccines_for_name
        AntigenPList find(const acmacs::chart::Antigens& aAntigens) const;
        AntigenPList date_range(std::string_view first, std::string_view after_last) const;

     private:
        size_t mNumberOfAntigens;
        const char* mIndex;
        const char* mAntigen0;
        const HiDb& mHiDb;

    }; // class Antigens

      // ----------------------------------------------------------------------

    class Tables;

    class Serum : public acmacs::chart::Serum
    {
     public:
        Serum(const char* aSerum, const HiDb& aHiDb) : mSerum(aSerum), mHiDb(aHiDb) {}

        acmacs::chart::Name name() const override;
        acmacs::virus::Passage passage() const override;
        acmacs::chart::BLineage lineage() const override;
        acmacs::virus::Reassortant reassortant() const override;
        acmacs::chart::Annotations annotations() const override;
        acmacs::chart::SerumId serum_id() const override;
        acmacs::chart::SerumSpecies serum_species() const override;
        acmacs::chart::PointIndexList homologous_antigens() const override;

        indexes_t tables() const;
        size_t number_of_tables() const;

        std::vector<std::string> labs(const Tables& tables) const;
        std::string_view location() const;
        std::string_view isolation() const;
        std::string year() const;

     private:
        const char* mSerum;
        const HiDb& mHiDb;

    }; // class Serum

    using SerumP = std::shared_ptr<Serum>;
    using SerumPList = std::vector<SerumP>;
    using SerumPIndex = std::pair<SerumP, size_t>;
    using SerumPIndexList = std::vector<SerumPIndex>;

    class Sera : public acmacs::chart::Sera
    {
     public:
        Sera(size_t aNumberOfSera, const char* aIndex, const char* aSerum0, const HiDb& aHiDb)
            : mNumberOfSera(aNumberOfSera), mIndex(aIndex), mSerum0(aSerum0), mHiDb(aHiDb) {}

        size_t size() const override { return mNumberOfSera; }
        std::shared_ptr<acmacs::chart::Serum> operator[](size_t aIndex) const override { return at(aIndex); }
        SerumP at(size_t aIndex) const;
        SerumPIndexList find(std::string aName, fix_location aFixLocation, find_fuzzy fuzzy = find_fuzzy::no) const;
        SerumPIndex find(const acmacs::chart::Serum& aSerum) const; // find_serum_of_chart
        SerumPList find(const acmacs::chart::Sera& aSera) const;
        SerumPList find_homologous(size_t aAntigenIndex, const Antigen& aAntigen) const; // for vaccines

     private:
        size_t mNumberOfSera;
        const char* mIndex;
        const char* mSerum0;
        const HiDb& mHiDb;

    }; // class Sera

      // ----------------------------------------------------------------------

    class Table // : public acmacs::chart::Table
    {
     public:
        Table(const char* aTable) : mTable(reinterpret_cast<const bin::Table*>(aTable)) {}

        std::string name() const;
        std::string_view assay() const;
        std::string_view lab() const;
        std::string_view date() const;
        std::string_view rbc() const;
        size_t number_of_antigens() const;
        size_t number_of_sera() const;
        indexes_t antigens() const;
        indexes_t sera() const;
        indexes_t reference_antigens(const HiDb& aHidb) const;

     private:
        const bin::Table* mTable;

    }; // class Table

    struct TableStat
    {
        TableStat(std::string_view a_assay, std::string_view a_lab, std::string_view a_rbc, std::shared_ptr<Table> table)
            : assay{a_assay}, lab{a_lab}, rbc{a_rbc}, number{1}, most_recent{table}, oldest{table} {}
        std::string title() const noexcept;
        const std::string_view assay;
        const std::string_view lab;
        const std::string_view rbc;
        size_t number;
        std::shared_ptr<Table> most_recent;
        std::shared_ptr<Table> oldest;
    };

    class Tables // : public acmacs::chart::Tables
    {
     public:
        Tables(size_t aNumberOfTables, const char* aIndex, const char* aTable0)
            : mNumberOfTables(aNumberOfTables), mIndex(aIndex), mTable0(aTable0) {}

        size_t size() const { return mNumberOfTables; }
        std::shared_ptr<Table> operator[](size_t aIndex) const;
        std::shared_ptr<Table> most_recent(const indexes_t& aTables) const;
        std::shared_ptr<Table> oldest(const indexes_t& aTables) const;
        std::vector<TableStat> stat(const indexes_t& aTables) const;

        using iterator = acmacs::chart::detail::iterator<Tables, std::shared_ptr<Table>>;
        iterator begin() const { return {*this, 0}; }
        iterator end() const { return {*this, size()}; }

     private:
        size_t mNumberOfTables;
        const char* mIndex;
        const char* mTable0;

    }; // class Tables

      // ----------------------------------------------------------------------

    class HiDb
    {
     public:
        HiDb(std::string aFilename, bool verbose);

        std::shared_ptr<Antigens> antigens() const;
        std::shared_ptr<Sera> sera() const;
        std::shared_ptr<Tables> tables() const;
        std::string_view virus_type() const;

        std::string_view lab(const Antigen& aAntigen, const Tables& aTables) const { return aTables[aAntigen.tables()[0]]->lab(); }
        std::string_view lab(const Serum& aSerum, const Tables& aTables) const { return aTables[aSerum.tables()[0]]->lab(); }

        // void find_homologous_antigens_for_sera_of_chart(Chart& aChart) const; // sets homologous_antigen attribute in chart
        // std::string serum_date(const SerumData& aSerum) const; // for stat

        // void stat_antigens(HiDbStat& aStat, std::string aStart, std::string aEnd) const;
        // void stat_sera(HiDbStat& aStat, HiDbStat* aStatUnique, std::string aStart, std::string aEnd) const;

        void save(std::string aFilename) const;

     private:
        const char* mData = nullptr;
        std::string mDataStorage;
        acmacs::file::read_access mAccess;

    }; // class HiDb

// ----------------------------------------------------------------------

} // namespace hidb


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
