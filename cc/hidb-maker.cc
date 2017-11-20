#include "acmacs-base/enumerate.hh"
#include "acmacs-base/virus-name.hh"
#include "acmacs-chart-2/chart.hh"
#include "hidb-maker.hh"

// ----------------------------------------------------------------------

void HidbMaker::add(const acmacs::chart::Chart& aChart)
{
    auto* table = mTables.add(aChart);
    table->set_titers(*aChart.titers());
    auto source_antigens = aChart.antigens();
    for (auto source_antigen: *source_antigens) {
        auto target_antigen = mAntigens.add(*source_antigen);
        target_antigen->add_table(table);
        table->add_antigen(target_antigen);
    }

} // HidbMaker::add

// ----------------------------------------------------------------------

void HidbMaker::save(std::string aFilename)
{

} // HidbMaker::save

// ----------------------------------------------------------------------

Table::Table(const acmacs::chart::Info& aInfo)
    : virus(aInfo.virus()), virus_type(aInfo.virus_type()), subset(aInfo.subset()),
      assay(aInfo.assay()), date(aInfo.date()), lab(aInfo.lab()), rbc_species(aInfo.rbc_species())
{

} // Table::Table

// ----------------------------------------------------------------------

void Table::set_titers(const acmacs::chart::Titers& aTiters)
{
    titers.resize(aTiters.number_of_antigens());
    for (auto [ag_no, row]: acmacs::enumerate(titers)) {
        row.resize(aTiters.number_of_sera());
        for (auto [sr_no, target]: acmacs::enumerate(row)) {
            target = aTiters.titer(ag_no, sr_no);
        }
    }

} // Table::set_titers

// ----------------------------------------------------------------------

Table* Tables::add(const acmacs::chart::Chart& aChart)
{
    auto table{std::make_unique<Table>(*aChart.info())};
    const auto insert_at = lower_bound(table);
    if (insert_at != end() && **insert_at == *table)
        throw std::runtime_error("Table " + acmacs::to_string(*table) + " is already in hidb");
    std::cerr << "DEBUG: adding " << acmacs::to_string(*table) << '\n';
    const auto inserted = insert(insert_at, std::move(table));
    return inserted->get();

} // Tables::add

// ----------------------------------------------------------------------

Antigen* Antigens::add(const acmacs::chart::Antigen& aAntigen)
{
    auto antigen{std::make_unique<Antigen>(aAntigen)};
    auto insert_at = lower_bound(antigen);
    if (insert_at == end() || **insert_at != *antigen) {
        insert_at = insert(insert_at, std::move(antigen));
        std::cerr << "DEBUG: AG " << (*insert_at)->to_string() << '\n';
    }
    (*insert_at)->add_date(aAntigen.date());
    const auto lab_ids{aAntigen.lab_ids()};
    (*insert_at)->add_lab_id(lab_ids.begin(), lab_ids.end());
    return insert_at->get();

} // Antigens::add

// ----------------------------------------------------------------------

AntigenSerum::~AntigenSerum()
{
} // AntigenSerum::~AntigenSerum

// ----------------------------------------------------------------------

void AntigenSerum::add_table(Table *aTable)
{
    if (auto found = table_ptrs.find(aTable); found != table_ptrs.end())
        throw std::runtime_error(type_name() + " " + to_string() + " already in the table " + acmacs::to_string(*aTable) + ", duplicate?");
    table_ptrs.insert(aTable);

} // AntigenSerum::add_table

// ----------------------------------------------------------------------

Antigen::Antigen(const acmacs::chart::Antigen& aAntigen)
    : reassortant{aAntigen.reassortant()}, passage{aAntigen.passage()}, annotations{aAntigen.annotations()},
      lineage{aAntigen.lineage()}
{
    const std::string name{aAntigen.name()};
    try {
        std::string temp_passage;
        virus_name::split(name, virus_type, host, location, isolation, year, temp_passage);
    }
    catch (virus_name::Unrecognized&) {
        if (name.size() > 3 && (name[2] == ' ' || name[2] == '-')) {
              // cdc name
            location = name.substr(0, 2);
            isolation = name.substr(3);
        }
        else
            throw std::runtime_error("Unrecognized antigen name: " + name);
    }

} // Antigen::Antigen

// ----------------------------------------------------------------------

Serum::Serum(const acmacs::chart::Serum& aSerum)
{

} // Serum::Serum

// ----------------------------------------------------------------------

std::string acmacs::to_string(const Table& aTable)
{
    return string::join(":", {aTable.virus, aTable.virus_type, aTable.subset, aTable.assay, aTable.lab, aTable.rbc_species, aTable.date});

} // acmacs::to_string

// ----------------------------------------------------------------------

std::string Antigen::to_string() const
{
    return string::join(" ", {string::join("/", {virus_type, host, location, isolation, year}), string::join(" ", annotations), reassortant, passage});

} // Antigen::to_string

// ----------------------------------------------------------------------

std::string Serum::to_string() const
{
    return string::join(" ", {string::join("/", {virus_type, host, location, isolation, year}), string::join(" ", annotations), reassortant, serum_id});

} // Serum::to_string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
