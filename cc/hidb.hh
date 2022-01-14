#pragma once

#include "acmacs-base/read-file.hh"
#include "acmacs-base/named-type.hh"
#include "locationdb/locdb.hh"
#include "acmacs-chart-2/chart.hh"
#include "hidb-5/hidb-set.hh"

// ----------------------------------------------------------------------

namespace hidb
{
    namespace bin { struct Table; struct Antigen; }

    using TableIndex = acmacs::named_size_t<struct TableIndex_tag>;
    using TableIndexList = std::vector<TableIndex>;

    class error : public std::runtime_error { public: using std::runtime_error::runtime_error; };
    class not_found : public error { public: using error::error; };

    enum class find_fuzzy { no, yes };
    enum class fix_location { no, yes };
    enum class passage_strictness { yes, ignore_if_empty, ignore };

    class HiDb;

    class Antigen : public acmacs::chart::Antigen
    {
     public:
        Antigen(const char* aAntigen, const HiDb& aHiDb) : mAntigen(aAntigen), mHiDb(aHiDb) {}

        acmacs::virus::name_t name() const override;
        acmacs::chart::Date date() const override;
        acmacs::virus::Passage passage() const override;
        acmacs::chart::BLineage lineage() const override;
        acmacs::virus::Reassortant reassortant() const override;
        acmacs::chart::LabIds lab_ids() const override;
        acmacs::chart::Clades clades() const override { return {}; }
        acmacs::chart::Annotations annotations() const override;
        bool reference() const override { return false; }

        TableIndexList tables() const;
        size_t number_of_tables() const;

        std::string_view location() const;
        std::string_view isolation() const;
        std::string year() const;
        std::string date_compact() const;

        std::string_view country(const LocDb& locdb) const noexcept;

        std::string full_name() const;

     private:
        const char* mAntigen;
        const HiDb& mHiDb;

    }; // class Antigen

    using AntigenIndex = acmacs::named_size_t<struct AntigenIndex_tag>;
    using AntigenIndexList = std::vector<AntigenIndex>;

    using AntigenP = std::shared_ptr<Antigen>;
    using AntigenPList = std::vector<AntigenP>;
    using AntigenPIndex = std::pair<AntigenP, AntigenIndex>;
    using AntigenPIndexList = std::vector<AntigenPIndex>;

    class Antigens : public acmacs::chart::Antigens
    {
     public:
        Antigens(size_t aNumberOfAntigens, const char* aIndex, const char* aAntigen0, const HiDb& aHiDb)
            : mNumberOfAntigens(aNumberOfAntigens), mIndex(aIndex), mAntigen0(aAntigen0), mHiDb(aHiDb) {}

        size_t size() const override { return mNumberOfAntigens; }
        std::shared_ptr<acmacs::chart::Antigen> operator[](size_t aIndex) const override { return at(AntigenIndex{aIndex}); }
        AntigenP at(AntigenIndex aIndex) const;
        // AntigenPIndexList find(std::string_view aName, fix_location aFixLocation, find_fuzzy fuzzy = find_fuzzy::no) const;
        AntigenIndexList find(std::string_view aName, fix_location aFixLocation, find_fuzzy fuzzy = find_fuzzy::no) const;
        AntigenPList find_labid(std::string_view labid) const;
        std::optional<AntigenPIndex> find(const acmacs::chart::Antigen& aAntigen, passage_strictness aPassageStrictness = passage_strictness::yes) const;
        AntigenPList find(const acmacs::chart::Antigens& aAntigens) const; // entry* per each antigen
        AntigenPList find(const acmacs::chart::Antigens& aAntigens, const acmacs::chart::Indexes& indexes) const; // entry* per each index
        AntigenPList date_range(std::string_view first, std::string_view after_last) const;

        std::vector<std::pair<std::string_view, const hidb::bin::Antigen*>> sorted_by_labid() const; // for seqdb-3, to speed up looking by lab_id
        AntigenP make(const hidb::bin::Antigen* antigen_bin) const { return std::make_shared<Antigen>(reinterpret_cast<const char*>(antigen_bin), mHiDb); }
        AntigenIndex index(const hidb::bin::Antigen* antigen_bin) const;
        AntigenPList list(const AntigenIndexList& indexes) const;

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

        acmacs::virus::name_t name() const override;
        acmacs::virus::Passage passage() const override;
        acmacs::chart::BLineage lineage() const override;
        acmacs::virus::Reassortant reassortant() const override;
        acmacs::chart::Annotations annotations() const override;
        acmacs::chart::Clades clades() const override { return {}; }
        acmacs::chart::SerumId serum_id() const override;
        acmacs::chart::SerumSpecies serum_species() const override;
        acmacs::chart::PointIndexList homologous_antigens() const override;

        TableIndexList tables() const;
        size_t number_of_tables() const;

        std::vector<std::string> labs(const Tables& tables) const;
        std::string_view location() const;
        std::string_view isolation() const;
        std::string year() const;

        std::string full_name() const;

     private:
        const char* mSerum;
        const HiDb& mHiDb;

    }; // class Serum

    using SerumIndex = acmacs::named_size_t<struct SerumIndex_tag>;
    using SerumIndexList = std::vector<SerumIndex>;

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
        std::shared_ptr<acmacs::chart::Serum> operator[](size_t aIndex) const override { return at(SerumIndex{aIndex}); }
        SerumP at(SerumIndex aIndex) const;
        SerumIndexList find(std::string_view aName, fix_location aFixLocation, find_fuzzy fuzzy = find_fuzzy::no) const;
        std::optional<SerumPIndex> find(const acmacs::chart::Serum& aSerum) const; // find_serum_of_chart
        SerumPList find(const acmacs::chart::Sera& aSera) const; // entry* per each serum
        SerumPList find(const acmacs::chart::Sera& aSera, const acmacs::chart::Indexes& indexes) const; // entry* per each index
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
        AntigenIndexList antigens() const;
        SerumIndexList sera() const;
        AntigenIndexList reference_antigens(const HiDb& aHidb) const;

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

    struct lab_assay_rbc_table_t
    {
        std::string_view lab;
        acmacs::chart::Assay assay;
        acmacs::chart::RbcSpecies rbc;
        std::vector<std::shared_ptr<Table>> tables;

        lab_assay_rbc_table_t(std::string_view a_lab, const acmacs::chart::Assay& a_assay, const acmacs::chart::RbcSpecies& a_rbc, std::shared_ptr<Table> a_table)
            : lab{a_lab}, assay{a_assay}, rbc{a_rbc}, tables{a_table}
        {
        }

        enum sort_by_date_order { oldest_first, recent_first };

        void sort_by_date(sort_by_date_order ord)
        {
            switch (ord) {
                case oldest_first:
                    std::sort(std::begin(tables), std::end(tables), [](const auto& t1, const auto& t2) { return t1->date() < t2->date(); });
                    break;
                case recent_first:
                    std::sort(std::begin(tables), std::end(tables), [](const auto& t1, const auto& t2) { return t1->date() > t2->date(); });
                    break;
            }
        }
    };

    class Tables // : public acmacs::chart::Tables
    {
     public:
        Tables(TableIndex aNumberOfTables, const char* aIndex, const char* aTable0)
            : mNumberOfTables{aNumberOfTables}, mIndex{aIndex}, mTable0{aTable0} {}

        TableIndex size() const { return mNumberOfTables; }
        std::shared_ptr<Table> at(TableIndex aIndex) const;
        std::shared_ptr<Table> operator[](TableIndex aIndex) const { return at(aIndex); }
        std::shared_ptr<Table> most_recent(const TableIndexList& aTables) const;
        std::shared_ptr<Table> oldest(const TableIndexList& aTables) const;
        std::vector<TableStat> stat(const TableIndexList& aTables) const;

        using iterator = acmacs::iterator<Tables, std::shared_ptr<Table>, TableIndex>;
        iterator begin() const { return {*this, TableIndex{0}}; }
        iterator end() const { return {*this, size()}; }

        std::vector<lab_assay_rbc_table_t> sorted(const TableIndexList& indexes, lab_assay_rbc_table_t::sort_by_date_order order) const;

     private:
        TableIndex mNumberOfTables;
        const char* mIndex;
        const char* mTable0;

    }; // class Tables

      // ----------------------------------------------------------------------

    class HiDb
    {
     public:
        HiDb(std::string_view aFilename, bool verbose=false);

        std::shared_ptr<Antigens> antigens() const;
        std::shared_ptr<Sera> sera() const;
        std::shared_ptr<Tables> tables() const;
        std::string_view virus_type() const;

        std::string_view lab(const Antigen& aAntigen) const { return tables()->at(aAntigen.tables()[0])->lab(); }
        std::string_view lab(const Serum& aSerum) const { return tables()->at(aSerum.tables()[0])->lab(); }

        std::vector<lab_assay_rbc_table_t> tables(const Antigen& aAntigen, lab_assay_rbc_table_t::sort_by_date_order order) const { return tables()->sorted(aAntigen.tables(), order); }
        std::vector<lab_assay_rbc_table_t> tables(const Serum& aSerum, lab_assay_rbc_table_t::sort_by_date_order order) const { return tables()->sorted(aSerum.tables(), order); }

        // void find_homologous_antigens_for_sera_of_chart(Chart& aChart) const; // sets homologous_antigen attribute in chart
        // std::string serum_date(const SerumData& aSerum) const; // for stat

        // void stat_antigens(HiDbStat& aStat, std::string aStart, std::string aEnd) const;
        // void stat_sera(HiDbStat& aStat, HiDbStat* aStatUnique, std::string aStart, std::string aEnd) const;

        void save(std::string_view aFilename) const;

     private:
        const char* mData = nullptr;
        std::string mDataStorage;
        acmacs::file::read_access mAccess;
        mutable std::shared_ptr<Tables> tables_;

    }; // class HiDb

// ----------------------------------------------------------------------

} // namespace hidb


// ----------------------------------------------------------------------
