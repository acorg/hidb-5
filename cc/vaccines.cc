#include <algorithm>
#include <functional>
#include <iomanip>
// #include <cmath>

#include "hidb-5/hidb.hh"
#include "hidb-5/vaccines.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

static std::map<std::string, std::vector<hidb::Vaccine>> sVaccines = {
    {"A(H1N1)", {
            {"CALIFORNIA/7/2009",        hidb::Vaccine::Previous},
            {"MICHIGAN/45/2015",         hidb::Vaccine::Current},
        }},
    {"A(H3N2)", {
            {"BRISBANE/10/2007",         hidb::Vaccine::Previous},
            {"PERTH/16/2009",            hidb::Vaccine::Previous},
            {"VICTORIA/361/2011",        hidb::Vaccine::Previous},
            {"TEXAS/50/2012",            hidb::Vaccine::Previous},
            {"SWITZERLAND/9715293/2013", hidb::Vaccine::Previous},
            {"HONG KONG/4801/2014",      hidb::Vaccine::Previous},
            {"SINGAPORE/INFIMH-16-0019/2016", hidb::Vaccine::Current},
            {"SAITAMA/103/2014",         hidb::Vaccine::Surrogate},
            {"HONG KONG/7295/2014",      hidb::Vaccine::Surrogate},
        }},
    {"BVICTORIA", {
            {"MALAYSIA/2506/2004",       hidb::Vaccine::Previous},
            {"BRISBANE/60/2008",         hidb::Vaccine::Current},
              // {"PARIS/1762/2009",          hidb::Vaccine::Current}, // not used by Crick anymore, B/Ireland/3154/2016 is used instead (2017-08-21)
            {"SOUTH AUSTRALIA/81/2012",  hidb::Vaccine::Surrogate},
            {"IRELAND/3154/2016",        hidb::Vaccine::Surrogate},
        }},
    {"BYAMAGATA", {
            {"FLORIDA/4/2006",           hidb::Vaccine::Previous},
            {"WISCONSIN/1/2010",         hidb::Vaccine::Previous},
            {"MASSACHUSETTS/2/2012",     hidb::Vaccine::Previous},
            {"PHUKET/3073/2013",         hidb::Vaccine::Current},
        }},
};

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

hidb::VaccinesOfChart hidb::vaccines(const acmacs::chart::Chart& aChart, bool aVerbose)
{
    VaccinesOfChart result;
    for (const auto& name_type: vaccine_names(aChart)) {
        vaccines_for_name(result.emplace_back(name_type), name_type.name, aChart, aVerbose);
    }
    return result;

} // hidb::vaccines

// ----------------------------------------------------------------------

void hidb::vaccines_for_name(Vaccines& aVaccines, std::string aName, const acmacs::chart::Chart& aChart, bool aVerbose)
{
    const auto virus_type = aChart.info()->virus_type(acmacs::chart::Info::Compute::Yes);
    const auto& hidb = hidb::get(virus_type, aVerbose ? report_time::Yes : report_time::No);
    auto hidb_antigens = hidb.antigens();
    auto hidb_sera = hidb.sera();
    auto chart_antigens = aChart.antigens();
    auto chart_sera = aChart.sera();
    for (size_t ag_no: chart_antigens->find_by_name(virus_type + "/" + aName)) {
        try {
            auto chart_antigen = aChart.antigen(ag_no);
            auto [hidb_antigen, hidb_antigen_index] = hidb_antigens->find(*chart_antigen);
            std::vector<hidb::Vaccines::HomologousSerum> homologous_sera;
            for (auto sd: hidb_sera->find_homologous(hidb_antigen_index, *hidb_antigen)) {
                if (const auto sr_no = chart_sera->find_by_full_name(virus_type + "/" + sd->full_name())) {
                    homologous_sera.emplace_back(*sr_no, (*chart_sera)[*sr_no], sd, hidb.tables()->most_recent(sd->tables()));
                }
            }
            aVaccines.add(ag_no, chart_antigen, hidb_antigen, hidb.tables()->most_recent(hidb_antigen->tables()), std::move(homologous_sera));
        }
        catch (hidb::not_found&) {
        }
    }
    aVaccines.sort();

} // hidb::vaccines_for_name

// ----------------------------------------------------------------------

const std::vector<hidb::Vaccine>& hidb::vaccine_names(std::string aSubtype, std::string aLineage)
{
    return sVaccines.at(aSubtype + aLineage);

} // hidb::vaccines

// ----------------------------------------------------------------------

std::string hidb::Vaccine::type_as_string(hidb::Vaccine::Type aType)
{
    switch (aType) {
      case Previous:
          return "previous";
      case Current:
          return "current";
      case Surrogate:
          return "surrogate";
    }
    return {};

} // hidb::Vaccine::type_as_string

// ----------------------------------------------------------------------

hidb::Vaccine::Type hidb::Vaccine::type_from_string(std::string aType)
{
    if (aType == "previous")
        return Previous;
    else if (aType == "current")
        return Current;
    else if (aType == "surrogate")
        return Surrogate;
    return Previous;

} // hidb::Vaccine::type_from_string

// ----------------------------------------------------------------------

std::string hidb::Vaccine::type_as_string() const
{
    return type_as_string(type);

} // hidb::Vaccine::type_as_string

// ----------------------------------------------------------------------

inline bool hidb::Vaccines::Entry::operator < (const hidb::Vaccines::Entry& a) const
{
    const auto a_nt = a.hidb_antigen->number_of_tables(), t_nt = hidb_antigen->number_of_tables();
    return t_nt == a_nt ? (most_recent_table->date() > a.most_recent_table->date()) : (t_nt > a_nt);
}

// ----------------------------------------------------------------------

bool hidb::Vaccines::HomologousSerum::operator < (const hidb::Vaccines::HomologousSerum& a) const
{
    bool result = true;
    if (chart_serum->serum_species() == "SHEEP") { // avoid using sheep serum as homologous (NIMR)
        result = false;
    }
    else {
        const auto s_nt = a.hidb_serum->number_of_tables(), t_nt = hidb_serum->number_of_tables();
        result = t_nt == s_nt ? most_recent_table->date() > a.most_recent_table->date() : t_nt > s_nt;
    }
    return result;

} // hidb::Vaccines::HomologousSerum::operator <

// ----------------------------------------------------------------------

size_t hidb::Vaccines::HomologousSerum::number_of_tables() const
{
    return hidb_serum->number_of_tables();

} // hidb::Vaccines::HomologousSerum::number_of_tables

// ----------------------------------------------------------------------

std::string hidb::Vaccines::report(const Vaccines::ReportConfig& config) const
{
    std::ostringstream out;
    if (!empty()) {
        out << std::string(config.indent_, ' ') << "Vaccine " << type_as_string() << " [" << mNameType.name << "]\n";
        for_each_passage_type([this,&config,&out](PassageType pt) { out << this->report(pt, config); });
        out << config.vaccine_sep_;
    }
    return out.str();

} // hidb::Vaccines::report

// ----------------------------------------------------------------------

std::string hidb::Vaccines::report(PassageType aPassageType, const Vaccines::ReportConfig& config, size_t aMark) const
{
    std::ostringstream out;
    const std::string indent(config.indent_ + 2, ' ');
    auto entry_report = [&](size_t aNo, const auto& entry, bool aMarkIt) {
        out << indent << (aMarkIt ? ">>" : "  ");
        if (config.show_no_)
            out << std::setw(2) << aNo << ' ';
        out << std::setw(4) << entry.chart_antigen_index << " \"" << entry.chart_antigen->full_name()
            << "\" tables:" << entry.hidb_antigen->number_of_tables()
            << " recent:" << entry.most_recent_table->name() << '\n';
        for (const auto& hs: entry.homologous_sera)
            out << indent << "      " << hs.chart_serum->serum_id() << ' ' << hs.chart_serum->annotations().join() << " tables:" << hs.hidb_serum->number_of_tables() << " recent:" << hs.most_recent_table->name() << std::endl;
    };

    const auto& entry = mEntries[aPassageType];
    if (!entry.empty()) {
        out << indent << passage_type_name(aPassageType) << " (" << entry.size() << ')' << std::endl;
        // const auto me = std::max_element(entry.begin(), entry.end(), [](const auto& e1, const auto& e2) { return e1.chart_antigen_index < e2.chart_antigen_index; });
          // const auto num_digits = static_cast<int>(std::log10(me->chart_antigen_index)) + 1;
        for (size_t no = 0; no < entry.size(); ++no)
            entry_report(no, entry[no], aMark == no);
        out << config.passage_type_sep_;
    }
    return out.str();

} // hidb::Vaccines::report

// ----------------------------------------------------------------------

// hidb::Vaccines* hidb::find_vaccines_in_chart(std::string aName, const Chart& aChart)
// {
//     Vaccines* result = new Vaccines(Vaccine(aName, hidb::Vaccine::Previous));
//     vaccines_for_name(*result, aName, aChart);
//     return result;

// } // find_vaccines_in_chart

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

std::string hidb::VaccinesOfChart::report(const Vaccines::ReportConfig& config) const
{
    std::string result;
    for (const auto& v: *this)
        result += v.report(config);
    return result;

} // hidb::VaccinesOfChart::report

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
