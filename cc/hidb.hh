#pragma once

#include <string>

#include "acmacs-base/read-file.hh"
#include "acmacs-chart-2/chart.hh"
#include "hidb-5/hidb-set.hh"

// ----------------------------------------------------------------------

namespace hidb
{
    namespace bin { struct Table; }

    using indexes_t = std::vector<size_t>;
    class not_found : public std::runtime_error { public: using std::runtime_error::runtime_error; };

    class Antigen : public acmacs::chart::Antigen
    {
     public:
        inline Antigen(const char* aAntigen) : mAntigen(aAntigen) {}

        acmacs::chart::Name name() const override;
        acmacs::chart::Date date() const override;
        acmacs::chart::Passage passage() const override;
        acmacs::chart::BLineage lineage() const override;
        acmacs::chart::Reassortant reassortant() const override;
        acmacs::chart::LabIds lab_ids() const override;
        inline acmacs::chart::Clades clades() const override { return {}; }
        acmacs::chart::Annotations annotations() const override;
        inline bool reference() const override { return false; }

        indexes_t tables() const;
        size_t number_of_tables() const;

        std::string_view location() const;
        std::string_view isolation() const;
        std::string year() const;
        std::string date_compact() const;

     private:
        const char* mAntigen;

    }; // class Antigen

    using AntigenP = std::shared_ptr<Antigen>;
    using AntigenPList = std::vector<AntigenP>;
    using AntigenPIndex = std::pair<AntigenP, size_t>;
    using AntigenPIndexList = std::vector<AntigenPIndex>;

    class Antigens : public acmacs::chart::Antigens
    {
     public:
        inline Antigens(size_t aNumberOfAntigens, const char* aIndex, const char* aAntigen0)
            : mNumberOfAntigens(aNumberOfAntigens), mIndex(aIndex), mAntigen0(aAntigen0) {}

        inline size_t size() const override { return mNumberOfAntigens; }
        inline std::shared_ptr<acmacs::chart::Antigen> operator[](size_t aIndex) const override { return at(aIndex); }
        AntigenP at(size_t aIndex) const;
        AntigenPIndexList find(std::string aName, bool aFixLocation) const;
        AntigenPList find_labid(std::string labid) const;
        AntigenPIndex find(const acmacs::chart::Antigen& aAntigen) const; // find_antigen_of_chart
        AntigenPList find(const acmacs::chart::Antigens& aAntigens) const;
        AntigenPList date_range(std::string first, std::string after_last) const;

     private:
        size_t mNumberOfAntigens;
        const char* mIndex;
        const char* mAntigen0;

    }; // class Antigens

      // ----------------------------------------------------------------------

    class Serum : public acmacs::chart::Serum
    {
     public:
        inline Serum(const char* aSerum) : mSerum(aSerum) {}

        acmacs::chart::Name name() const override;
        acmacs::chart::Passage passage() const override;
        acmacs::chart::BLineage lineage() const override;
        acmacs::chart::Reassortant reassortant() const override;
        acmacs::chart::Annotations annotations() const override;
        acmacs::chart::SerumId serum_id() const override;
        acmacs::chart::SerumSpecies serum_species() const override;
        acmacs::chart::PointIndexList homologous_antigens() const override;

        indexes_t tables() const;
        size_t number_of_tables() const;

        std::string_view location() const;
        std::string_view isolation() const;
        std::string year() const;

     private:
        const char* mSerum;

    }; // class Serum

    using SerumP = std::shared_ptr<Serum>;
    using SerumPList = std::vector<SerumP>;
    using SerumPIndex = std::pair<SerumP, size_t>;
    using SerumPIndexList = std::vector<SerumPIndex>;

    class Sera : public acmacs::chart::Sera
    {
     public:
        inline Sera(size_t aNumberOfSera, const char* aIndex, const char* aSerum0)
            : mNumberOfSera(aNumberOfSera), mIndex(aIndex), mSerum0(aSerum0) {}

        inline size_t size() const override { return mNumberOfSera; }
        inline std::shared_ptr<acmacs::chart::Serum> operator[](size_t aIndex) const override { return at(aIndex); }
        SerumP at(size_t aIndex) const;
        SerumPIndexList find(std::string aName, bool aFixLocation) const;
        SerumPIndex find(const acmacs::chart::Serum& aSerum) const; // find_serum_of_chart
        SerumPList find(const acmacs::chart::Sera& aSera) const;
        SerumPList find_homologous(size_t aAntigenIndex, const Antigen& aAntigen) const; // for vaccines

     private:
        size_t mNumberOfSera;
        const char* mIndex;
        const char* mSerum0;

    }; // class Sera

      // ----------------------------------------------------------------------

    class Table // : public acmacs::chart::Table
    {
     public:
        inline Table(const char* aTable) : mTable(reinterpret_cast<const bin::Table*>(aTable)) {}

        std::string name() const;
        std::string_view assay() const;
        std::string_view lab() const;
        std::string_view date() const;
        std::string_view rbc() const;
        size_t number_of_antigens() const;
        size_t number_of_sera() const;

     private:
        const bin::Table* mTable;

    }; // class Table

    class Tables // : public acmacs::chart::Tables
    {
     public:
        inline Tables(size_t aNumberOfTables, const char* aIndex, const char* aTable0)
            : mNumberOfTables(aNumberOfTables), mIndex(aIndex), mTable0(aTable0) {}

        inline size_t size() const { return mNumberOfTables; }
        std::shared_ptr<Table> operator[](size_t aIndex) const;
        std::shared_ptr<Table> most_recent(const indexes_t& aTables) const;

        using iterator = acmacs::chart::internal::iterator<Tables, std::shared_ptr<Table>>;
        inline iterator begin() const { return {*this, 0}; }
        inline iterator end() const { return {*this, size()}; }

     private:
        size_t mNumberOfTables;
        const char* mIndex;
        const char* mTable0;

    }; // class Tables

      // ----------------------------------------------------------------------

    class HiDb
    {
     public:
        HiDb(std::string aFilename, report_time timer);

        std::shared_ptr<Antigens> antigens() const;
        std::shared_ptr<Sera> sera() const;
        std::shared_ptr<Tables> tables() const;

        inline std::string_view lab(const Antigen& aAntigen, const Tables& aTables) const { return aTables[aAntigen.tables()[0]]->lab(); }
        inline std::string_view lab(const Serum& aSerum, const Tables& aTables) const { return aTables[aSerum.tables()[0]]->lab(); }

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
