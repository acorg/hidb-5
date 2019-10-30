#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>

#include "acmacs-base/acmacsd.hh"
#include "acmacs-base/settings.hh"
#include "acmacs-chart-2/chart-modify.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/vaccines.hh"

// see ~/AD/share/conf/vaccines.json

// ----------------------------------------------------------------------

class VaccineData : public acmacs::settings::Settings
{
  public:
    VaccineData()
    {
        using namespace std::string_literals;
        using namespace std::string_view_literals;

        if (const auto filename = fmt::format("{}/share/conf/vaccines.json", acmacs::acmacsd_root()); fs::exists(filename))
            acmacs::settings::Settings::load(filename);
        else
            throw hidb::error{fmt::format("WARNING: cannot load \"{}\": file not found\n", filename)};

        using pp = std::pair<std::string, std::string_view>;
        for (const auto& [virus_type, tag] : {pp{"A(H1N1)"s, "vaccines-A(H1N1)PDM09"sv}, pp{"A(H3N2)"s, "vaccines-A(H3N2)"sv}, pp{"BVICTORIA"s, "vaccines-BVICTORIA"sv}, pp{"BYAMAGATA"s, "vaccines-BYAMAGATA"sv}}) {
            current_virus_type_ = virus_type;
            apply(tag, acmacs::verbose::no);
        }
    }

    bool apply_built_in(std::string_view name) override // returns true if built-in command with that name found and applied
    {
        if (name == "vaccine")
            data_[current_virus_type_].emplace_back(getenv("name", ""), vaccine_type(getenv("vaccine_type", "")));
        else
            return acmacs::settings::Settings::apply_built_in(name);
        return true;
    }

    const auto& find(std::string_view virus_type) const
    {
        if (const auto found = data_.find(virus_type); found != data_.end())
            return found->second;
        throw hidb::error(fmt::format("No vaccines defined for \"{}\"", virus_type));
    }

  private:
    std::string current_virus_type_;
    std::map<std::string, std::vector<hidb::Vaccine>, std::less<>> data_;

    hidb::Vaccine::Type vaccine_type(std::string_view type_s) const
    {
        if (type_s == "previous")
            return hidb::Vaccine::Previous;
        else if (type_s == "current")
            return hidb::Vaccine::Current;
        else if (type_s == "surrogate")
            return hidb::Vaccine::Surrogate;
        else
            throw hidb::error(fmt::format("Unrecognized vaccine type: \"{}\" (loaded from vaccines.json)", type_s));
    }
};

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif
static std::mutex sVaccineMutex;
static std::unique_ptr<VaccineData> sVaccines;
#pragma GCC diagnostic pop

const std::vector<hidb::Vaccine>& hidb::vaccine_names(const acmacs::chart::VirusType& aSubtype, std::string aLineage)
{
    {
        std::lock_guard<std::mutex> vaccine_guard{sVaccineMutex};
        if (!sVaccines)
            sVaccines = std::make_unique<VaccineData>();
    }

    return sVaccines->find(fmt::format("{}{}", *aSubtype, aLineage));

} // hidb::vaccines

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

void hidb::vaccines_for_name(Vaccines& aVaccines, std::string_view aName, const acmacs::chart::Chart& aChart, bool aVerbose)
{
    const auto virus_type = aChart.info()->virus_type(acmacs::chart::Info::Compute::Yes);
    const auto& hidb = hidb::get(virus_type, do_report_time(aVerbose));
    auto hidb_antigens = hidb.antigens();
    auto hidb_sera = hidb.sera();
    auto chart_antigens = aChart.antigens();
    auto chart_sera = aChart.sera();
    for (size_t ag_no: chart_antigens->find_by_name(fmt::format("{}/{}", virus_type, aName))) {
        try {
            auto chart_antigen = aChart.antigen(ag_no);
            auto [hidb_antigen, hidb_antigen_index] = hidb_antigens->find(*chart_antigen, passage_strictness::ignore_if_empty);
            std::vector<hidb::Vaccines::HomologousSerum> homologous_sera;
            for (auto sd: hidb_sera->find_homologous(hidb_antigen_index, *hidb_antigen)) {
                if (const auto sr_no = chart_sera->find_by_full_name(fmt::format("{}/{}", virus_type, sd->full_name()))) {
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
    if (chart_serum->serum_species() == acmacs::chart::SerumSpecies{"SHEEP"}) { // avoid using sheep serum as homologous (NIMR)
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
    fmt::memory_buffer out;
    if (!empty()) {
        fmt::format_to(out, "{:{}c}Vaccine {} [{}]\n", ' ', config.indent_, type_as_string(), mNameType.name);
        for_each_passage_type([this, &config, &out](PassageType pt) { fmt::format_to(out, "{}", this->report(pt, config)); });
        fmt::format_to(out, "{}", config.vaccine_sep_);
    }
    return fmt::to_string(out);

} // hidb::Vaccines::report

// ----------------------------------------------------------------------

std::string hidb::Vaccines::report(PassageType aPassageType, const Vaccines::ReportConfig& config, size_t aMark) const
{
    fmt::memory_buffer out;
    auto entry_report = [&](size_t aNo, const auto& entry, bool aMarkIt) {
        fmt::format_to(out, "{:{}c}{}", ' ', config.indent_ + 2, aMarkIt ? ">>" : "  ");
        if (config.show_no_)
            fmt::format_to(out, "{:2d} ", aNo);
        fmt::format_to(out, "{:4d} \"{}\" tables:{} recent:{}\n", entry.chart_antigen_index, entry.chart_antigen->full_name(), entry.hidb_antigen->number_of_tables(), entry.most_recent_table->name());
        for (const auto& hs: entry.homologous_sera)
            fmt::format_to(out, "{:{}c}      {} {} tables:{} recent:{}\n", ' ', config.indent_ + 2, hs.chart_serum->serum_id(), hs.chart_serum->annotations().join(), hs.hidb_serum->number_of_tables(), hs.most_recent_table->name());
    };

    const auto& entry = mEntries[aPassageType];
    if (!entry.empty()) {
        fmt::format_to(out, "{:{}c}{} ({})\n", ' ', config.indent_ + 2, passage_type_name(aPassageType), entry.size());
        // const auto me = std::max_element(entry.begin(), entry.end(), [](const auto& e1, const auto& e2) { return e1.chart_antigen_index < e2.chart_antigen_index; });
          // const auto num_digits = static_cast<int>(std::log10(me->chart_antigen_index)) + 1;
        for (size_t no = 0; no < entry.size(); ++no)
            entry_report(no, entry[no], aMark == no);
        fmt::format_to(out, "{}", config.passage_type_sep_);
    }
    return fmt::to_string(out);

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
