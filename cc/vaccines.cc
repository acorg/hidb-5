#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>

#include "acmacs-chart-2/chart-modify.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/vaccines.hh"

// ----------------------------------------------------------------------

hidb::VaccinesOfChart hidb::vaccines(const acmacs::chart::Chart& aChart)
{
    const auto virus_type = aChart.info()->virus_type(acmacs::chart::Info::Compute::Yes);
    const auto& hidb = hidb::get(virus_type);
    auto hidb_antigens = hidb.antigens();
    auto hidb_sera = hidb.sera();
    auto chart_antigens = aChart.antigens();
    auto chart_sera = aChart.sera();

    VaccinesOfChart result;
    for (const auto& name_type : acmacs::whocc::vaccine_names(virus_type, aChart.lineage())) {
        Vaccines& vaccines = result.emplace_back(name_type);
        for (size_t ag_no : chart_antigens->find_by_name(fmt::format("{}/{}", virus_type, name_type.name))) {
            auto chart_antigen = aChart.antigen(ag_no);
            if (const auto hidb_antigen_index = hidb_antigens->find(*chart_antigen, passage_strictness::ignore_if_empty); hidb_antigen_index.has_value()) {
                std::vector<hidb::Vaccines::HomologousSerum> homologous_sera;
                for (auto sd : hidb_sera->find_homologous(*hidb_antigen_index->second, *hidb_antigen_index->first)) {
                    if (const auto sr_no = chart_sera->find_by_full_name(fmt::format("{}/{}", virus_type, sd->full_name()))) {
                        homologous_sera.emplace_back(*sr_no, (*chart_sera)[*sr_no], sd, hidb.tables()->most_recent(sd->tables()));
                    }
                }
                vaccines.add(ag_no, chart_antigen, hidb_antigen_index->first, hidb.tables()->most_recent(hidb_antigen_index->first->tables()), std::move(homologous_sera));
            }
        }
        vaccines.sort();
    }

    return result;

} // hidb::vaccines

// ----------------------------------------------------------------------

void hidb::update_vaccines(acmacs::chart::ChartModify& /*aChart*/, const VaccinesOfChart& vaccines)
{
    AD_WARNING("hidb::update_vaccines not implemented (need to implemented sematic attributes first)");
    // fmt::print("{}\n", vaccines.report());


    for (const auto& vacc : vaccines) {
        // fmt::print("Vaccine {} ({})\n", vacc.name(), vacc.type());
        if (!vacc.empty()) {
            Vaccines::for_each_passage_type([&vacc](Vaccines::PassageType pt) {
                // fmt::print("  {} ({}):\n", Vaccines::passage_type_name(pt), vacc.size_for_passage_type(pt));
                for (size_t no = 0; no < vacc.size_for_passage_type(pt); ++no) {
                    if (const auto* entry = vacc.for_passage_type(pt, no); entry) {
                        // fmt::print("    {:2d}: AG {:4d} {}\n", no, entry->chart_antigen_index, entry->chart_antigen->full_name());
                    }
                }
            });
        }
    }

} // hidb::update_vaccines

// ----------------------------------------------------------------------

void hidb::update_vaccines(acmacs::chart::ChartModify& aChart)
{
    update_vaccines(aChart, vaccines(aChart));

} // hidb::update_vaccines

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
        fmt::format_to(out, "{:{}c}Vaccine {} [{}]\n", ' ', config.indent_, type(), mNameType.name);
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

    const auto& entry = mEntries[static_cast<size_t>(aPassageType)];
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
