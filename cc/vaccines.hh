#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "acmacs-chart-2/chart.hh"

// ----------------------------------------------------------------------

namespace hidb
{
    class Vaccine
    {
     public:
        enum Type { Previous, Current, Surrogate };

        inline Vaccine(std::string aName, Type aType) : name(aName), type(aType) {}

        std::string name;
        Type type;

        std::string type_as_string() const;

        static std::string type_as_string(Type aType);
        static Type type_from_string(std::string aType);

    }; // class Vaccine

// ----------------------------------------------------------------------

    class Vaccines
    {
     public:
        // class HomologousSerum
        // {
        //  public:
        //     inline HomologousSerum(size_t aSerumIndex, const Serum* aSerum, const hidb::Serum* aSerumData, std::string aMostRecentTableDate)
        //         : serum(aSerum), serum_index(aSerumIndex), serum_data(aSerumData), most_recent_table_date(aMostRecentTableDate) {}
        //     bool operator < (const HomologousSerum& a) const;
        //     size_t number_of_tables() const;

        //     const Serum* serum;
        //     size_t serum_index;
        //     const hidb::Serum* serum_data;
        //     std::string most_recent_table_date;
        // };

        class Entry
        {
         public:
            inline Entry(size_t aAntigenIndex, std::shared_ptr<acmacs::chart::Antigen> aChartAntigen, std::shared_ptr<hidb::Antigen> aHidbAntigen, std::shared_ptr<hidb::Table> aMostRecentTable) // , std::vector<HomologousSerum>&& aSera
                : chart_antigen(aChartAntigen), chart_antigen_index(aAntigenIndex), hidb_antigen(aHidbAntigen),
                  most_recent_table(aMostRecentTable) // , homologous_sera(aSera)
                {
                      // std::sort(homologous_sera.begin(), homologous_sera.end());
                }
            bool operator < (const Entry& a) const;

            std::shared_ptr<acmacs::chart::Antigen> chart_antigen;
            size_t chart_antigen_index;
            std::shared_ptr<hidb::Antigen> hidb_antigen;
            std::shared_ptr<hidb::Table> most_recent_table;
            // std::vector<HomologousSerum> homologous_sera; // sorted by number of tables and the most recent table
        };

        enum PassageType : int { Cell, Egg, Reassortant, PassageTypeSize };
        template <typename UnaryFunction> static void for_each_passage_type(UnaryFunction f) { f(Cell); f(Egg); f(Reassortant); }

        inline Vaccines(const Vaccine& aNameType) : mNameType(aNameType) {}

        inline size_t number_of(PassageType pt) const { return mEntries[pt].size(); }
        inline size_t number_of_eggs() const { return number_of(Egg); }
        inline size_t number_of_cells() const { return number_of(Cell); }
        inline size_t number_of_reassortants() const { return number_of(Reassortant); }
        inline bool empty(PassageType pt) const { return mEntries[pt].empty(); }
        inline bool empty() const { return std::all_of(std::begin(mEntries), std::end(mEntries), [](const auto& e) { return e.empty(); }); }

        inline const Entry* for_passage_type(PassageType pt, size_t aNo = 0) const { return mEntries[pt].size() > aNo ? &mEntries[pt][aNo] : nullptr; }
        inline const Entry* egg(size_t aNo = 0) const { return for_passage_type(Egg, aNo); }
        inline const Entry* cell(size_t aNo = 0) const { return for_passage_type(Cell, aNo); }
        inline const Entry* reassortant(size_t aNo = 0) const { return for_passage_type(Reassortant, aNo); }

        inline std::string type_as_string() const { return mNameType.type_as_string(); }
        std::string report(size_t aIndent = 0) const;
        std::string report(PassageType aPassageType, size_t aIndent = 0, size_t aMark = static_cast<size_t>(-1)) const;

        inline bool match(std::string aName, std::string aType) const
            {
                return (aName.empty() || mNameType.name.find(aName) != std::string::npos) && (aType.empty() || mNameType.type == Vaccine::type_from_string(aType));
            }

        inline std::string type() const { return mNameType.type_as_string(); }
        inline std::string name() const { return mNameType.name; }

        static inline PassageType passage_type(std::string pt)
            {
                if (pt == "egg")
                    return Egg;
                else if (pt == "cell")
                    return Cell;
                else if (pt == "reassortant")
                    return Reassortant;
                else if (pt.empty())
                    return PassageTypeSize;
                else
                    throw std::runtime_error("Unrecognized passage type: " + pt);
            }

     private:
        Vaccine mNameType;
        std::vector<Entry> mEntries[PassageTypeSize];

        friend void vaccines_for_name(Vaccines& aVaccines, std::string aName, const acmacs::chart::Chart& aChart, bool aVerbose);

        static inline PassageType passage_type(const acmacs::chart::Antigen& aAntigen)
            {
                if (!aAntigen.reassortant().empty())
                    return Reassortant;
                else if (aAntigen.passage().is_egg())
                    return Egg;
                return Cell;
            }

        static inline const char* passage_type_name(PassageType pt)
            {
                switch (pt) {
                  case Egg: return "Egg";
                  case Cell: return "Cell";
                  case Reassortant: return "Reassortant";
                  case PassageTypeSize: return "?";
                }
                return "?";
            }

        inline void add(size_t aAntigenIndex, std::shared_ptr<acmacs::chart::Antigen> aChartAntigen, std::shared_ptr<hidb::Antigen> aHidbAntigen, std::shared_ptr<hidb::Table> aMostRecentTable) //, std::vector<HomologousSerum>&& aSera
            {
                mEntries[passage_type(*aChartAntigen)].emplace_back(aAntigenIndex, aChartAntigen, aHidbAntigen, aMostRecentTable); //, std::move(aSera)
            }

        inline void sort()
            {
                for (auto& entry: mEntries)
                    std::sort(entry.begin(), entry.end());
            }

    }; // class Vaccines

// ----------------------------------------------------------------------

    class VaccinesOfChart : public std::vector<Vaccines>
    {
     public:
        using std::vector<Vaccines>::vector;

        std::string report(size_t aIndent = 0) const;

    }; // class VaccinesOfChart

// ----------------------------------------------------------------------

    const std::vector<Vaccine>& vaccine_names(std::string aSubtype, std::string aLineage);
    inline const std::vector<Vaccine>& vaccine_names(const acmacs::chart::Chart& aChart) { return vaccine_names(aChart.info()->virus_type(), aChart.lineage()); }
    // Vaccines* find_vaccines_in_chart(std::string aName, const Chart& aChart);
    void vaccines_for_name(Vaccines& aVaccines, std::string aName, const acmacs::chart::Chart& aChart, bool aVerbose = false);
    VaccinesOfChart vaccines(const acmacs::chart::Chart& aChart, bool aVerbose = false);

} // namespace hidb

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
