#pragma once

#include <string>
#include <vector>
#include <algorithm>

// #include "acmacs-chart-2/chart.hh"
#include "acmacs-whocc-data/vaccines.hh"
#include "hidb-5/hidb.hh"

// ----------------------------------------------------------------------

namespace acmacs::chart { class Chart; }

namespace hidb
{
    // class Vaccine
    // {
    //  public:
    //     enum Type { Previous, Current, Surrogate };

    //     Vaccine(std::string_view aName, Type aType) : name{aName}, type{aType} {}

    //     std::string name;
    //     Type type;

    //     std::string type_as_string() const;

    //     static std::string type_as_string(Type aType);
    //     static Type type_from_string(std::string aType);

    // }; // class Vaccine

// ----------------------------------------------------------------------

    class VaccinesOfChart;

    class Vaccines
    {
     public:
        class HomologousSerum
        {
         public:
            HomologousSerum(size_t aSerumIndex, std::shared_ptr<acmacs::chart::Serum> aChartSerum, hidb::SerumP aHidbSerum, std::shared_ptr<hidb::Table> aMostRecentTable)
                : chart_serum(aChartSerum), chart_serum_index(aSerumIndex), hidb_serum(aHidbSerum), most_recent_table(aMostRecentTable) {}
            bool operator < (const HomologousSerum& a) const;
            size_t number_of_tables() const;

            std::shared_ptr<acmacs::chart::Serum> chart_serum;
            size_t chart_serum_index;
            hidb::SerumP hidb_serum;
            std::shared_ptr<hidb::Table> most_recent_table;
        };

        class Entry
        {
         public:
            Entry(size_t aAntigenIndex, std::shared_ptr<acmacs::chart::Antigen> aChartAntigen, AntigenP aHidbAntigen, std::shared_ptr<hidb::Table> aMostRecentTable, std::vector<HomologousSerum>&& aSera)
                : chart_antigen(aChartAntigen), chart_antigen_index(aAntigenIndex), hidb_antigen(aHidbAntigen),
                  most_recent_table(aMostRecentTable), homologous_sera(std::move(aSera))
                {
                    std::sort(homologous_sera.begin(), homologous_sera.end());
                }
            bool operator < (const Entry& a) const;

            std::shared_ptr<acmacs::chart::Antigen> chart_antigen;
            size_t chart_antigen_index;
            AntigenP hidb_antigen;
            std::shared_ptr<hidb::Table> most_recent_table;
            std::vector<HomologousSerum> homologous_sera; // sorted by number of tables and the most recent table
        };

        struct ReportConfig
        {
            size_t indent_ = 0;
            std::string vaccine_sep_ = "";
            std::string passage_type_sep_ = "";
            bool show_no_ = true;

            constexpr ReportConfig& indent(size_t new_indent) { indent_ = new_indent; return *this; }
            ReportConfig& passage_type_sep(std::string sep) { passage_type_sep_ = sep; return *this; }
            ReportConfig& vaccine_sep(std::string sep) { vaccine_sep_ = sep; return *this; }
            constexpr ReportConfig& show_no(bool show) { show_no_ = show; return *this; }

        };

        enum PassageType : int { Cell, Egg, Reassortant, PassageTypeSize };
        template <typename UnaryFunction> static void for_each_passage_type(UnaryFunction f) { f(Cell); f(Egg); f(Reassortant); }

        Vaccines(const acmacs::whocc::Vaccine& aNameType) : mNameType(aNameType) {}

        size_t number_of(PassageType pt) const { return mEntries[pt].size(); }
        size_t number_of_eggs() const { return number_of(Egg); }
        size_t number_of_cells() const { return number_of(Cell); }
        size_t number_of_reassortants() const { return number_of(Reassortant); }
        bool empty(PassageType pt) const { return mEntries[pt].empty(); }
        bool empty() const { return std::all_of(std::begin(mEntries), std::end(mEntries), [](const auto& e) { return e.empty(); }); }

        const Entry* for_passage_type(PassageType pt, size_t aNo = 0) const { return mEntries[pt].size() > aNo ? &mEntries[pt][aNo] : nullptr; }
        size_t size_for_passage_type(PassageType pt) const { return mEntries[pt].size(); }
        const Entry* egg(size_t aNo = 0) const { return for_passage_type(Egg, aNo); }
        const Entry* cell(size_t aNo = 0) const { return for_passage_type(Cell, aNo); }
        const Entry* reassortant(size_t aNo = 0) const { return for_passage_type(Reassortant, aNo); }

        std::string type_as_string() const { return mNameType.type_as_string(); }
        std::string report(const Vaccines::ReportConfig& config) const;
        std::string report(PassageType aPassageType, const Vaccines::ReportConfig& config, size_t aMark = static_cast<size_t>(-1)) const;

        bool match(std::string aName, std::string aType) const
            {
                return (aName.empty() || mNameType.name.find(aName) != std::string::npos) && (aType.empty() || mNameType.type == acmacs::whocc::Vaccine::type_from_string(aType));
            }

        std::string type() const { return mNameType.type_as_string(); }
        std::string name() const { return mNameType.name; }

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
                    throw error("Unrecognized passage type: " + pt);
            }

     private:
        acmacs::whocc::Vaccine mNameType;
        std::vector<Entry> mEntries[PassageTypeSize];

        friend VaccinesOfChart vaccines(const acmacs::chart::Chart&);

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

        void add(size_t aAntigenIndex, std::shared_ptr<acmacs::chart::Antigen> aChartAntigen, AntigenP aHidbAntigen, std::shared_ptr<hidb::Table> aMostRecentTable, std::vector<HomologousSerum>&& aSera)
            {
                mEntries[passage_type(*aChartAntigen)].emplace_back(aAntigenIndex, aChartAntigen, aHidbAntigen, aMostRecentTable, std::move(aSera));
            }

        void sort()
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

        std::string report(const Vaccines::ReportConfig& config) const;

    }; // class VaccinesOfChart

// ----------------------------------------------------------------------

    // const std::vector<Vaccine>& vaccine_names(const acmacs::virus::type_subtype_t& aSubtype, std::string aLineage);

    VaccinesOfChart vaccines(const acmacs::chart::Chart& aChart);

    Vaccines* find_vaccines_in_chart(std::string aName, const acmacs::chart::Chart& aChart);
    void update_vaccines(const VaccinesOfChart& vaccines);
    inline void update_vaccines(const acmacs::chart::Chart& aChart) { update_vaccines(vaccines(aChart)); }

} // namespace hidb

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
