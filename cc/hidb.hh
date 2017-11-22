#pragma once

#include <string>

#include "acmacs-chart-2/chart.hh"
#include "hidb-5/hidb-set.hh"

// ----------------------------------------------------------------------

namespace hidb
{
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

        std::vector<size_t> tables() const;

     private:
        const char* mAntigen;

    }; // class Antigen

    class Antigens : public acmacs::chart::Antigens
    {
     public:
        inline Antigens(size_t aNumberOfAntigens, const char* aIndex, const char* aAntigen0)
            : mNumberOfAntigens(aNumberOfAntigens), mIndex(aIndex), mAntigen0(aAntigen0) {}

        inline size_t size() const override { return mNumberOfAntigens; }
        std::shared_ptr<acmacs::chart::Antigen> operator[](size_t aIndex) const override;
          // acmacs::chart::Indexes find_by_name(std::string aName) const override;

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

        std::vector<size_t> tables() const;

     private:
        const char* mSerum;

    }; // class Serum

    class Sera : public acmacs::chart::Sera
    {
     public:
        inline Sera(size_t aNumberOfSera, const char* aIndex, const char* aSerum0)
            : mNumberOfSera(aNumberOfSera), mIndex(aIndex), mSerum0(aSerum0) {}

        inline size_t size() const override { return mNumberOfSera; }
        std::shared_ptr<acmacs::chart::Serum> operator[](size_t aIndex) const override;

     private:
        size_t mNumberOfSera;
        const char* mIndex;
        const char* mSerum0;

    }; // class Antigens

      // ----------------------------------------------------------------------

    class HiDb
    {
     public:
        HiDb(std::string aFilename, report_time timer);

        std::shared_ptr<Antigens> antigens() const;
        std::shared_ptr<Sera> sera() const;
        // inline const Tables& charts() const { return mCharts; }
        // inline Tables& charts() { return mCharts; }
        // inline const ChartData& table(std::string table_id) const { return charts()[table_id]; }

        // std::vector<const AntigenData*> find_antigens(std::string name_reassortant_annotations_passage) const;
        // const AntigenData& find_antigen_exactly(std::string name_reassortant_annotations_passage) const; // throws NotFound if antigen with this very set of data not found
        // std::vector<const AntigenData*> find_antigens_fuzzy(std::string name_reassortant_annotations_passage) const;
        // std::vector<const AntigenData*> find_antigens_extra_fuzzy(std::string name_reassortant_annotations_passage) const;
        // inline std::vector<const AntigenData*> find_antigens_by_name(std::string name, std::string* aNotFoundLocation = nullptr) const { return mAntigens.find_by_index(name, aNotFoundLocation); }
        // inline std::vector<const AntigenData*> find_antigens_by_cdcid(std::string cdcid) const  { return mAntigens.find_by_cdcid(cdcid); }
        // const AntigenData& find_antigen_of_chart(const Antigen& aAntigen) const; // throws if not found

        // std::vector<std::pair<const AntigenData*, size_t>> find_antigens_with_score(std::string name) const;
        // std::vector<std::string> list_antigen_names(std::string aLab, std::string aLineage, bool aFullName) const;
        // std::vector<const AntigenData*> list_antigens(std::string aLab, std::string aLineage, std::string aAssay) const;
        // std::vector<const SerumData*> find_sera(std::string name) const;
        // const SerumData& find_serum_exactly(std::string name_reassortant_annotations_serum_id) const; // throws NotFound if serum with this very set of data not found
        // std::vector<std::pair<const SerumData*, size_t>> find_sera_with_score(std::string name) const;
        // std::vector<std::string> list_serum_names(std::string aLab, std::string aLineage, bool aFullName) const;
        // std::vector<const SerumData*> list_sera(std::string aLab, std::string aLineage) const;
        // std::vector<const SerumData*> find_homologous_sera(const AntigenData& aAntigen) const;
        // const SerumData& find_serum_of_chart(const Serum& aSerum, bool report_if_not_found = false) const; // throws if not found
        // void find_homologous_antigens_for_sera_of_chart(Chart& aChart) const;
        // std::string serum_date(const SerumData& aSerum) const;

        //   // name is just (international) name without reassortant/passage

        // inline AntigenRefs all_antigens() const { return mAntigens.all(*this); }

        // std::vector<std::string> all_countries() const;
        // std::vector<std::string> unrecognized_locations() const;
        // void stat_antigens(HiDbStat& aStat, std::string aStart, std::string aEnd) const;
        // void stat_sera(HiDbStat& aStat, HiDbStat* aStatUnique, std::string aStart, std::string aEnd) const;


        void save(std::string aFilename) const;

     private:
        const char* mData = nullptr;
        std::string mDataStorage;

    }; // class HiDb

// ----------------------------------------------------------------------


} // namespace hidb


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: