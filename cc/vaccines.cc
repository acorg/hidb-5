#include <algorithm>
#include <functional>
#include <iomanip>
// #include <cmath>

#include "acmacs-chart-2/chart-modify.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/vaccines.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

// https://www.fludb.org/brc/vaccineRecommend.spg?decorator=influenza

static std::map<std::string, std::vector<hidb::Vaccine>> sVaccines = {
    {"A(H1N1)", {
              // Seasonal
            // {"USSR/90/1977",             hidb::Vaccine::Previous}, // 1978
            // {"BRAZIL/11/1978",           hidb::Vaccine::Previous}, // 1980
            // {"CHILE/1/1983",             hidb::Vaccine::Previous}, // 1984
            // {"SINGAPORE/6/1986",         hidb::Vaccine::Previous}, // 1987
            // {"BAYERN/7/1995",            hidb::Vaccine::Previous}, // 1997
            // {"BEIJING/262/1995",         hidb::Vaccine::Previous}, // 1998
            // {"NEW CALEDONIA/20/1999",    hidb::Vaccine::Previous}, // 2000
            // {"SOLOMON ISLANDS/3/2006",   hidb::Vaccine::Previous}, // 2008
            // {"BRISBANE/59/2007",         hidb::Vaccine::Previous}, // 2008, appears on H1pdm CDC maps as an outlier

              // PDM
            {"CALIFORNIA/7/2009",        hidb::Vaccine::Previous},
            {"MICHIGAN/45/2015",         hidb::Vaccine::Current},
        }},
    {"A(H3N2)", {
            {"PORT CHALMERS/1/1973",     hidb::Vaccine::Previous}, // 1974?
            {"VICTORIA/3/1975",          hidb::Vaccine::Previous}, // 1976
            {"TEXAS/1/1977",             hidb::Vaccine::Previous}, // 1978
            {"BANGKOK/1/1979",           hidb::Vaccine::Previous}, // 1980
            {"PHILIPPINES/2/1982",       hidb::Vaccine::Previous}, // 1983
            {"MISSISSIPPI/1/1985",       hidb::Vaccine::Previous}, // 1986
            {"CHRISTCHURCH/4/1985",      hidb::Vaccine::Previous}, // 1986
            {"LENINGRAD/360/1986",       hidb::Vaccine::Previous}, // 1987
            {"SICHUAN/2/1987",           hidb::Vaccine::Previous}, // 1988
            {"SHANGHAI/11/1987",         hidb::Vaccine::Previous}, // 1989
            {"GUIZHOU/54/1989",          hidb::Vaccine::Previous}, // 1990
            {"BEIJING/353/1989",         hidb::Vaccine::Previous}, // 1991
            {"BEIJING/32/1992",          hidb::Vaccine::Previous}, // 1993
            {"SHANGDONG/9/1993",         hidb::Vaccine::Previous}, // 1994
            {"JOHANNESBURG/33/1994",     hidb::Vaccine::Previous}, // 1995
            {"WUHAN/359/1995",           hidb::Vaccine::Previous}, // 1996
            {"SYDNEY/5/1997",            hidb::Vaccine::Previous}, // 1998-2000
            {"MOSCOW/10/1999",           hidb::Vaccine::Previous}, // 2000-2004
            {"FUJIAN/411/2002",          hidb::Vaccine::Previous}, // 2004-2005
            {"WELLINGTON/1/2004",        hidb::Vaccine::Previous}, // 2005
            {"CALIFORNIA/7/2004",        hidb::Vaccine::Previous}, // 2005-2006
            {"WISCONSIN/67/2005",        hidb::Vaccine::Previous}, // 2006-2008
            {"BRISBANE/10/2007",         hidb::Vaccine::Previous}, // 2008-2010
            {"PERTH/16/2009",            hidb::Vaccine::Previous},  // 2010
            {"VICTORIA/361/2011",        hidb::Vaccine::Previous},  // 2012
            {"TEXAS/50/2012",            hidb::Vaccine::Previous},  // 2014
            {"SWITZERLAND/9715293/2013", hidb::Vaccine::Previous},  // 2015
            {"HONG KONG/4801/2014",      hidb::Vaccine::Previous},  // 2016
            {"SINGAPORE/INFIMH-16-0019/2016", hidb::Vaccine::Previous},
            {"SWITZERLAND/8060/2017",    hidb::Vaccine::Current}, // 2018-09
            {"KANSAS/14/2017",           hidb::Vaccine::Current}, // ? 2019-02
            {"SAITAMA/103/2014",         hidb::Vaccine::Surrogate},
            {"HONG KONG/7295/2014",      hidb::Vaccine::Surrogate},
        }},
    {"BVICTORIA", {
            {"MALAYSIA/2506/2004",       hidb::Vaccine::Previous}, // 2006
            {"BRISBANE/60/2008",         hidb::Vaccine::Previous},  // 2009
              // {"MARYLAND/15/2016",       hidb::Vaccine::Current},
            {"COLORADO/6/2017",        hidb::Vaccine::Current}, // 2018-03-01 http://www.who.int/influenza/vaccines/virus/recommendations/2018_19_north/en/
              // {"PARIS/1762/2009",          hidb::Vaccine::Current}, // not used by Crick anymore, B/Ireland/3154/2016 is used instead (2017-08-21)
            {"IOWA/6/2017",              hidb::Vaccine::Surrogate}, // QMC2 2018-11-14 requested to mark by David Wentworth (CDC, NIH)
            {"SOUTH AUSTRALIA/81/2012",  hidb::Vaccine::Surrogate},
            {"IRELAND/3154/2016",        hidb::Vaccine::Surrogate},
        }},
    {"BYAMAGATA", {
            {"FLORIDA/4/2006",           hidb::Vaccine::Previous}, // 2008
            {"WISCONSIN/1/2010",         hidb::Vaccine::Previous}, // 2012
            {"MASSACHUSETTS/2/2012",     hidb::Vaccine::Previous}, // 2013
            {"PHUKET/3073/2013",         hidb::Vaccine::Current}, // 2015
        }},
    {"B", {
            {"HONG KONG/5/1972",         hidb::Vaccine::Previous}, // 1974?
            {"SINGAPORE/222/1979",       hidb::Vaccine::Previous}, // 1980
            {"USSR/100/83",              hidb::Vaccine::Previous}, // 1984
            {"ANN ARBOR/1/1986",         hidb::Vaccine::Previous}, // 1986
            {"BEIJING/1/1987",           hidb::Vaccine::Previous}, // 1988
            {"YAMAGATA/16/1988",         hidb::Vaccine::Previous}, // 1989
            {"PANAMA/45/1990",           hidb::Vaccine::Previous}, // 1993
            {"BEIJING/184/1993",         hidb::Vaccine::Previous}, // 1995
            {"SICHUAN/379/1999",         hidb::Vaccine::Previous}, // 2001
            {"HONG KONG/330/2001",       hidb::Vaccine::Previous}, // 2002
            {"SHANGHAI/361/2002",        hidb::Vaccine::Previous}, // 2004
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
    const auto& hidb = hidb::get(virus_type, do_report_time(aVerbose));
    auto hidb_antigens = hidb.antigens();
    auto hidb_sera = hidb.sera();
    auto chart_antigens = aChart.antigens();
    auto chart_sera = aChart.sera();
    for (size_t ag_no: chart_antigens->find_by_name(virus_type + "/" + aName)) {
        try {
            auto chart_antigen = aChart.antigen(ag_no);
            auto [hidb_antigen, hidb_antigen_index] = hidb_antigens->find(*chart_antigen, passage_strictness::ignore_if_empty);
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

void hidb::update_vaccines(acmacs::chart::ChartModify& /*chart*/, const VaccinesOfChart& vaccines, bool /*verbose*/)
{
    for (const auto& vacc : vaccines) {
        if (!vacc.empty()) {
            Vaccines::for_each_passage_type([&vacc](Vaccines::PassageType pt) {
                for (size_t no = 0; no < vacc.size_for_passage_type(pt); ++no) {
                      // const auto* entry = vacc.for_passage_type(pt, no);
                }
            });
        }
    }

} // hidb::update_vaccines

// ----------------------------------------------------------------------

void hidb::update_vaccines(acmacs::chart::ChartModify& aChart, bool aVerbose)
{
    update_vaccines(aChart, vaccines(aChart, aVerbose), aVerbose);

} // hidb::update_vaccines

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
