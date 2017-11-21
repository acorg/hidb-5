#pragma once

#include <string>

#include "hidb-5/hidb-set.hh"

// ----------------------------------------------------------------------

namespace hidb
{
    class HiDb
    {
     public:
        HiDb(std::string aFilename, report_time timer);

        // inline const Antigens& antigens() const { return mAntigens; }
        // inline Antigens& antigens() { return mAntigens; }
        // inline const Sera& sera() const { return mSera; }
        // inline Sera& sera() { return mSera; }
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


     private:


    }; // class HiDb

// ----------------------------------------------------------------------


} // namespace hidb


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
