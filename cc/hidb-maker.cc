#include "acmacs-base/stream.hh"
#include "acmacs-virus/virus-name-v1.hh"
#include "acmacs-base/rjson-v2.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-chart-2/chart.hh"
#include "hidb-maker.hh"

// ----------------------------------------------------------------------

void HidbMaker::add(const acmacs::chart::Chart& aChart)
{
    auto* table = mTables.add(aChart);
    table->set_titers(*aChart.titers());

    auto source_antigens = aChart.antigens();
    std::vector<Antigen*> antigens_of_table(source_antigens->size(), nullptr);
    for (auto [ag_no, source_antigen]: acmacs::enumerate(*source_antigens)) {
        if (!source_antigen->annotations().distinct()) {
            auto target_antigen = mAntigens.add(*source_antigen);
            target_antigen->add_table(table);
            table->add_antigen(target_antigen);
            antigens_of_table[ag_no] = target_antigen;
        }
    }

    auto source_sera = aChart.sera();
    for (auto source_serum: *source_sera) {
        if (!source_serum->annotations().distinct()) {
            auto target_serum = mSera.add(*source_serum);
            target_serum->add_table(table);
            table->add_serum(target_serum);
            for (size_t ag_no: source_serum->homologous_antigens()) {
                if (const Antigen* ag = antigens_of_table[ag_no]; ag != nullptr) {
                    target_serum->homologous_ptrs.insert(ag);
                }
            }
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

void HidbMaker::export_antigens(rjson::value& target) const
{
    for (auto& antigen: mAntigens) {
        rjson::value ag{rjson::object{}};
        if (!antigen->virus_type.empty())
            ag["V"] = antigen->virus_type;
        else
            ag["V"] = *mTables[antigen->tables.front()]->virus_type;
        rjson::set_field_if_not_empty(ag, "H", antigen->host);
        rjson::set_field_if_not_empty(ag, "O", antigen->location);
        rjson::set_field_if_not_empty(ag, "i", antigen->isolation);
        rjson::set_field_if_not_empty(ag, "y", antigen->year);
        rjson::set_field_if_not_empty(ag, "L", antigen->lineage);
        rjson::set_field_if_not_empty(ag, "P", antigen->passage);
        rjson::set_field_if_not_empty(ag, "R", antigen->reassortant);
        rjson::set_array_field_if_not_empty(ag, "a", antigen->annotations.begin(), antigen->annotations.end());
        rjson::set_array_field_if_not_empty(ag, "D", antigen->dates.begin(), antigen->dates.end());
        rjson::set_array_field_if_not_empty(ag, "l", antigen->lab_ids.begin(), antigen->lab_ids.end());
        rjson::set_array_field_if_not_empty(ag, "T", antigen->tables.begin(), antigen->tables.end());
        target.append(std::move(ag));
    }

} // HidbMaker::export_antigens

// ----------------------------------------------------------------------

void HidbMaker::export_sera(rjson::value& target) const
{
    for (auto& serum: mSera) {
        rjson::value sr{rjson::object{}};
        rjson::set_field_if_not_empty(sr, "V", serum->virus_type);
        rjson::set_field_if_not_empty(sr, "H", serum->host);
        rjson::set_field_if_not_empty(sr, "O", serum->location);
        rjson::set_field_if_not_empty(sr, "i", serum->isolation);
        rjson::set_field_if_not_empty(sr, "y", serum->year);
        rjson::set_field_if_not_empty(sr, "L", serum->lineage);
        rjson::set_field_if_not_empty(sr, "P", serum->passage);
        rjson::set_field_if_not_empty(sr, "R", serum->reassortant);
        rjson::set_field_if_not_empty(sr, "I", serum->serum_id);
        rjson::set_field_if_not_empty(sr, "s", serum->serum_species);
        rjson::set_array_field_if_not_empty(sr, "a", serum->annotations.begin(), serum->annotations.end());
        rjson::set_array_field_if_not_empty(sr, "T", serum->tables.begin(), serum->tables.end());
        rjson::set_array_field_if_not_empty(sr, "h", serum->homologous.begin(), serum->homologous.end());
        target.append(std::move(sr));
    }

} // HidbMaker::export_sera

// ----------------------------------------------------------------------

void HidbMaker::export_tables(rjson::value& target) const
{
    for (auto& table: mTables) {
        rjson::value tb{rjson::object{}};
        rjson::set_field_if_not_empty(tb, "v", *table->virus == "influenza" ? std::string{} : *table->virus);
        rjson::set_field_if_not_empty(tb, "V", table->virus_type);
        rjson::set_field_if_not_empty(tb, "A", table->assay);
        rjson::set_field_if_not_empty(tb, "D", table->date);
        rjson::set_field_if_not_empty(tb, "l", table->lab);
        rjson::set_field_if_not_empty(tb, "r", table->rbc_species);
        rjson::set_field_if_not_empty(tb, "s", table->subset);
        rjson::set_field_if_not_empty(tb, "L", table->lineage);
        rjson::set_array_field_if_not_empty(tb, "a", table->antigens.begin(), table->antigens.end());
        rjson::set_array_field_if_not_empty(tb, "s", table->sera.begin(), table->sera.end());
        auto& titers = tb["t"] = rjson::array{};
        for (const auto& row: table->titers)
            titers.append(rjson::array(row.begin(), row.end()));
        target.append(std::move(tb));
    }

} // HidbMaker::export_tables

// ----------------------------------------------------------------------

void HidbMaker::save(std::string_view aFilename)
{
    make_index();

    rjson::value data{rjson::object{{"  version", "hidb-v5"}, {"a", rjson::array{}}, {"s", rjson::array{}}, {"t", rjson::array{}}}};
    export_antigens(data["a"]);
    export_sera(data["s"]);
    export_tables(data["t"]);

    acmacs::file::write(aFilename, rjson::pretty(data, rjson::emacs_indent::yes, rjson::PrettyHandler(1)));

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
    std::sort(antigens.begin(), antigens.end());
    sera.resize(serum_ptrs.size());
    std::transform(serum_ptrs.begin(), serum_ptrs.end(), sera.begin(), [](const auto& ptr) -> size_t { return ptr->index; });
    std::sort(sera.begin(), sera.end());

} // Table::make_indexes

// ----------------------------------------------------------------------

Table* Tables::add(const acmacs::chart::Chart& aChart)
{
    auto table{std::make_unique<Table>(*aChart.info())};
    const auto insert_at = lower_bound(table);
    if (insert_at != end() && **insert_at == *table)
        throw std::runtime_error("Table " + acmacs::to_string(*table) + " is already in hidb");
    if (const auto lineage = aChart.lineage(); !lineage.empty())
        table->lineage = acmacs::virus::lineage_t{lineage->substr(0, 1)};
    fmt::print(stderr, "DEBUG: adding  {}\n", acmacs::to_string(*table));
    return insert(insert_at, std::move(table))->get();

} // Tables::add

// ----------------------------------------------------------------------

Antigen* Antigens::add(const acmacs::chart::Antigen& aAntigen)
{
    auto antigen{std::make_unique<Antigen>(aAntigen)};
    auto insert_at = lower_bound(antigen);
    if (insert_at == end() || **insert_at != *antigen) {
        insert_at = insert(insert_at, std::move(antigen));
        // std::cerr << "DEBUG: AG " << (*insert_at)->to_string() << '\n';
    }
    // else {
    //     std::cerr << "DEBUG: AG " << antigen->to_string() << " already here as " + (*insert_at)->to_string() << '\n';
    // }
    (*insert_at)->add_date(*aAntigen.date());
    const auto lab_ids{aAntigen.lab_ids()};
    (*insert_at)->add_lab_id(lab_ids.begin(), lab_ids.end());
    (*insert_at)->update_lineage(aAntigen.lineage());
    return insert_at->get();

} // Antigens::add

// ----------------------------------------------------------------------

Serum* Sera::add(const acmacs::chart::Serum& aSerum)
{
    auto serum{std::make_unique<Serum>(aSerum)};
    auto insert_at = lower_bound(serum);
    if (insert_at == end() || **insert_at != *serum) {
        insert_at = insert(insert_at, std::move(serum));
        // std::cerr << "DEBUG: SR " << (*insert_at)->to_string() << '\n';
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
    std::sort(tables.begin(), tables.end());

} // AntigenSerum::make_indexes

// ----------------------------------------------------------------------

Antigen::Antigen(const acmacs::chart::Antigen& aAntigen)
    : AntigenSerum(aAntigen.reassortant(), aAntigen.passage(), *aAntigen.annotations(), aAntigen.lineage().to_string())
{
    const std::string name{aAntigen.name()};
    try {
        std::string temp_passage;
        virus_name::split(name, virus_type, host, location, isolation, year, temp_passage);
        if (!temp_passage.empty()) {
            throw virus_name::Unrecognized{name};
        }
    }
    catch (virus_name::Unrecognized&) {
        virus_type.clear();
        host.clear();
        year.clear();
        if (name.size() > 3 && (name[2] == ' ' || name[2] == '-')) {
              // cdc name with location
            location = name.substr(0, 2);
            isolation = name.substr(3);
        }
        else {
              // cdc name without location (H3 FRA tables sometimes miss location data)
            std::cerr << "WARNING: cdc name without location: " << name << '\n';
            location = "cdc-name-without-location";
            isolation = name;
        }
    }

} // Antigen::Antigen

// ----------------------------------------------------------------------

Serum::Serum(const acmacs::chart::Serum& aSerum)
    : AntigenSerum(aSerum.reassortant(), aSerum.passage(), *aSerum.annotations(), aSerum.lineage().to_string()),
      serum_id{aSerum.serum_id()}, serum_species{aSerum.serum_species()}
{
    const std::string name{aSerum.name()};
    try {
        std::string temp_passage;
        virus_name::split(name, virus_type, host, location, isolation, year, temp_passage);
        if (!temp_passage.empty()) {
              // std::cerr << "WARNING: strange serum name: " << name << '\n';
            throw virus_name::Unrecognized{name};
        }
    }
    catch (virus_name::Unrecognized&) {
        std::cerr << "WARNING: unrecognized serum name: " << name << '\n';
        virus_type.clear();
        host.clear();
        location.clear();
        year.clear();
        isolation = name;
    }

} // Serum::Serum

// ----------------------------------------------------------------------

void Serum::make_indexes()
{
    AntigenSerum::make_indexes();

    homologous.resize(homologous_ptrs.size());
    std::transform(homologous_ptrs.begin(), homologous_ptrs.end(), homologous.begin(), [](const auto& ptr) -> size_t { return ptr->index; });
    std::sort(homologous.begin(), homologous.end());

} // Serum::make_indexes

// ----------------------------------------------------------------------

std::string acmacs::to_string(const Table& aTable)
{
    return acmacs::string::join(acmacs::string::join_colon, *aTable.virus == "influenza" ? std::string{} : *aTable.virus, aTable.virus_type, aTable.subset, aTable.lineage, aTable.assay, aTable.lab, aTable.rbc_species, aTable.date);

} // acmacs::to_string

// ----------------------------------------------------------------------

std::string Antigen::to_string() const
{
    return acmacs::string::join(acmacs::string::join_space, acmacs::string::join(acmacs::string::join_slash, virus_type, host, location, isolation, year), acmacs::string::join(acmacs::string::join_space, annotations), reassortant, passage);

} // Antigen::to_string

// ----------------------------------------------------------------------

std::string Serum::to_string() const
{
    return acmacs::string::join(acmacs::string::join_space, acmacs::string::join(acmacs::string::join_slash, virus_type, host, location, isolation, year), acmacs::string::join(acmacs::string::join_space, annotations), reassortant, serum_id);

} // Serum::to_string

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
