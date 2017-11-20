#include "acmacs-base/virus-name.hh"
#include "acmacs-base/rjson.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-chart-2/chart.hh"
#include "hidb-maker.hh"

// ----------------------------------------------------------------------

void HidbMaker::add(const acmacs::chart::Chart& aChart)
{
    auto* table = mTables.add(aChart);
    table->set_titers(*aChart.titers());

    auto source_antigens = aChart.antigens();
    for (auto source_antigen: *source_antigens) {
        if (!source_antigen->annotations().distinct()) {
            auto target_antigen = mAntigens.add(*source_antigen);
            target_antigen->add_table(table);
            table->add_antigen(target_antigen);
        }
    }

    auto source_sera = aChart.sera();
    for (auto source_serum: *source_sera) {
        if (!source_serum->annotations().distinct()) {
            auto target_serum = mSera.add(*source_serum);
            target_serum->add_table(table);
            table->add_serum(target_serum);
        }
    }

} // HidbMaker::add

// ----------------------------------------------------------------------

void HidbMaker::make_index()
{
    mTables.make_index();
    mAntigens.make_index();
    mSera.make_index();

    mTables.make_indexes();
    mAntigens.make_indexes();
    mSera.make_indexes();

} // HidbMaker::make_index

// ----------------------------------------------------------------------

namespace rjson
{
    template <> struct content_type<Lineage> { using type = rjson::string; };
    template <> struct content_type<Virus> { using type = rjson::string; };
    // template <> struct content_type<std::vector<std::string>> { using type = rjson::array; };

} // namespace rjson

// ----------------------------------------------------------------------

void HidbMaker::export_antigens(rjson::array& target) const
{
    for (auto& antigen: mAntigens) {
        rjson::object ag;
        ag.set_field_if_not_empty("V", antigen->virus_type);
        ag.set_field_if_not_empty("H", antigen->host);
        ag.set_field_if_not_empty("O", antigen->location);
        ag.set_field_if_not_empty("i", antigen->isolation);
        ag.set_field_if_not_empty("y", antigen->year);
        ag.set_field_if_not_empty("L", antigen->lineage);
        ag.set_field_if_not_empty("P", antigen->passage);
        ag.set_field_if_not_empty("R", antigen->reassortant);
        ag.set_array_field_if_not_empty("a", antigen->annotations.begin(), antigen->annotations.end());
        ag.set_array_field_if_not_empty("D", antigen->dates.begin(), antigen->dates.end());
        ag.set_array_field_if_not_empty("l", antigen->lab_ids.begin(), antigen->lab_ids.end());
        ag.set_array_field_if_not_empty("T", antigen->tables.begin(), antigen->tables.end());
        target.insert(std::move(ag));
    }

} // HidbMaker::export_antigens

// ----------------------------------------------------------------------

void HidbMaker::export_sera(rjson::array& target) const
{
    std::cerr << "WARNING: serum homologous not implemented" << '\n';
    for (auto& serum: mSera) {
        rjson::object sr;
        sr.set_field_if_not_empty("V", serum->virus_type);
        sr.set_field_if_not_empty("H", serum->host);
        sr.set_field_if_not_empty("O", serum->location);
        sr.set_field_if_not_empty("i", serum->isolation);
        sr.set_field_if_not_empty("y", serum->year);
        sr.set_field_if_not_empty("L", serum->lineage);
        sr.set_field_if_not_empty("P", serum->passage);
        sr.set_field_if_not_empty("R", serum->reassortant);
        sr.set_field_if_not_empty("I", serum->serum_id);
        sr.set_field_if_not_empty("s", serum->serum_species);
        sr.set_array_field_if_not_empty("a", serum->annotations.begin(), serum->annotations.end());
        sr.set_array_field_if_not_empty("T", serum->tables.begin(), serum->tables.end());
        sr.set_array_field_if_not_empty("h", serum->homologous.begin(), serum->homologous.end());
        target.insert(std::move(sr));
    }

} // HidbMaker::export_sera

// ----------------------------------------------------------------------

void HidbMaker::export_tables(rjson::array& target) const
{
    for (auto& table: mTables) {
        rjson::object tb;
        tb.set_field_if_not_empty("v", table->virus);
        tb.set_field_if_not_empty("V", table->virus_type);
        tb.set_field_if_not_empty("A", table->assay);
        tb.set_field_if_not_empty("D", table->date);
        tb.set_field_if_not_empty("l", table->lab);
        tb.set_field_if_not_empty("r", table->rbc_species);
        tb.set_field_if_not_empty("s", table->subset);
        tb.set_field_if_not_empty("L", table->lineage);
        tb.set_array_field_if_not_empty("a", table->antigens.begin(), table->antigens.end());
        tb.set_array_field_if_not_empty("s", table->sera.begin(), table->sera.end());
        rjson::array titers;
        for (const auto& row: table->titers) {
            titers.insert(rjson::array{rjson::array::use_iterator, row.begin(), row.end()});
        }
        tb.set_field("t", std::move(titers));
        target.insert(std::move(tb));
    }

} // HidbMaker::export_tables

// ----------------------------------------------------------------------

void HidbMaker::save(std::string aFilename)
{
    make_index();

    rjson::object data{{{"  version", rjson::string{"hidb-v5"}}, {"a", rjson::array{}}, {"s", rjson::array{}}, {"t", rjson::array{}}}};
    export_antigens(data["a"]);
    export_sera(data["s"]);
    export_tables(data["t"]);

    acmacs::file::write(aFilename, data.to_json_pp(1, rjson::json_pp_emacs_indent::yes));

    std::cerr << "INFO: antigens: " << mAntigens.size() << '\n';
    std::cerr << "INFO: sera:     " << mSera.size() << '\n';
    std::cerr << "INFO: tables:   " << mTables.size() << '\n';

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

void Table::make_indexes()
{
    antigens.resize(antigen_ptrs.size());
    std::transform(antigen_ptrs.begin(), antigen_ptrs.end(), antigens.begin(), [](const auto& ptr) -> size_t { return ptr->index; });
    sera.resize(serum_ptrs.size());
    std::transform(serum_ptrs.begin(), serum_ptrs.end(), sera.begin(), [](const auto& ptr) -> size_t { return ptr->index; });

} // Table::make_indexes

// ----------------------------------------------------------------------

Table* Tables::add(const acmacs::chart::Chart& aChart)
{
    auto table{std::make_unique<Table>(*aChart.info())};
    const auto insert_at = lower_bound(table);
    if (insert_at != end() && **insert_at == *table)
        throw std::runtime_error("Table " + acmacs::to_string(*table) + " is already in hidb");
    std::cerr << "DEBUG: adding " << acmacs::to_string(*table) << '\n';
    table->lineage = aChart.lineage();
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
    // else {
    //     std::cerr << "DEBUG: AG " << antigen->to_string() << " already here as " + (*insert_at)->to_string() << '\n';
    // }
    (*insert_at)->add_date(aAntigen.date());
    const auto lab_ids{aAntigen.lab_ids()};
    (*insert_at)->add_lab_id(lab_ids.begin(), lab_ids.end());
    return insert_at->get();

} // Antigens::add

// ----------------------------------------------------------------------

Serum* Sera::add(const acmacs::chart::Serum& aSerum)
{
    auto serum{std::make_unique<Serum>(aSerum)};
    auto insert_at = lower_bound(serum);
    if (insert_at == end() || **insert_at != *serum) {
        insert_at = insert(insert_at, std::move(serum));
        std::cerr << "DEBUG: SR " << (*insert_at)->to_string() << '\n';
    }
    // else {
    //     std::cerr << "DEBUG: SR " << serum->to_string() << " already here as " + (*insert_at)->to_string() << '\n';
    // }
    return insert_at->get();

} // Sera::add

// ----------------------------------------------------------------------

AntigenSerum::~AntigenSerum()
{
} // AntigenSerum::~AntigenSerum

// ----------------------------------------------------------------------

void AntigenSerum::add_table(Table *aTable)
{
    if (auto found = table_ptrs.find(aTable); found != table_ptrs.end())
        throw std::runtime_error(type_name() + " " + to_string() + " already in the table " + acmacs::to_string(*aTable) + ",  duplicate?");
    table_ptrs.insert(aTable);

} // AntigenSerum::add_table

// ----------------------------------------------------------------------

void AntigenSerum::make_indexes()
{
    tables.resize(table_ptrs.size());
    std::transform(table_ptrs.begin(), table_ptrs.end(), tables.begin(), [](const auto& ptr) -> size_t { return ptr->index; });

} // AntigenSerum::make_indexes

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
    : reassortant{aSerum.reassortant()}, passage{aSerum.passage()}, annotations{aSerum.annotations()},
      serum_id{aSerum.serum_id()}, serum_species{aSerum.serum_species()}, lineage{aSerum.lineage()}
{
    const std::string name{aSerum.name()};
    try {
        std::string temp_passage;
        virus_name::split(name, virus_type, host, location, isolation, year, temp_passage);
    }
    catch (virus_name::Unrecognized&) {
        throw std::runtime_error("Unrecognized serum name: " + name);
    }

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
