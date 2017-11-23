#include <map>
#include <cstring>
#include <limits>

#include "acmacs-base/string.hh"
#include "acmacs-base/stream.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-base/rjson.hh"
#include "hidb-5/hidb-json.hh"
#include "hidb-5/hidb-bin.hh"

// ----------------------------------------------------------------------

class Estimations
{
 public:
    Estimations(const rjson::value& aSource);

    size_t number_of_antigens;
    size_t number_of_sera;
    size_t number_of_tables;

    size_t antigen_size;
    size_t serum_size;
    size_t table_size;

    size_t size;

    std::map<std::string, size_t> virus_types;
    std::string virus_type;

 private:
    void antigen(const rjson::array& antigens);
    void serum(const rjson::array& sera);
    void table(const rjson::array& tables);

}; // class Estimations

// ----------------------------------------------------------------------

static size_t make_antigen(const rjson::object& aSource, hidb::bin::Antigen* aTarget);
static size_t make_serum(const rjson::object& aSource, hidb::bin::Serum* aTarget);
static size_t make_table(const rjson::object& aSource, hidb::bin::Table* aTarget);

// ----------------------------------------------------------------------

std::string hidb::json::read(std::string aData)
{
    using ptr_t = char*;

    const auto val = rjson::parse_string(aData);
    Estimations estimations{val};

    std::string result(estimations.size, 0);

    auto* const data_start = const_cast<char*>(result.data());
    auto* const header_bin = reinterpret_cast<hidb::bin::Header*>(data_start);
    std::memmove(header_bin->signature, "HIDB0500", 8);
    header_bin->antigen_offset = sizeof(hidb::bin::Header);

    auto* antigen_index = reinterpret_cast<hidb::bin::ASTIndex*>(data_start + sizeof(*header_bin));
    auto* antigen_data = reinterpret_cast<ptr_t>(reinterpret_cast<ptr_t>(antigen_index) + sizeof(hidb::bin::ast_offset_t) * estimations.number_of_antigens + sizeof(hidb::bin::ast_number_t));

    header_bin->virus_type_size = static_cast<decltype(header_bin->virus_type_size)>(estimations.virus_type.size());
    std::memset(header_bin->virus_type, 0, static_cast<size_t>(header_bin->virus_type_size));
    std::memmove(header_bin->virus_type, estimations.virus_type.data(), static_cast<size_t>(header_bin->virus_type_size));

    Timeit ti_antigens("converting " + acmacs::to_string(estimations.number_of_antigens) + " antigens: ");
    antigen_index->number_of = static_cast<hidb::bin::ast_number_t>(estimations.number_of_antigens);
    hidb::bin::ast_offset_t* antigen_offset = &antigen_index->offset;
    hidb::bin::ast_offset_t previous_antigen_offset = 0;
    for (const rjson::object& antigen: static_cast<const rjson::array&>(val["a"])) {
        const auto ag_size = make_antigen(antigen, reinterpret_cast<hidb::bin::Antigen*>(antigen_data));
        *antigen_offset = static_cast<hidb::bin::ast_offset_t>(ag_size) + previous_antigen_offset;
        previous_antigen_offset = *antigen_offset;
        ++antigen_offset;
        antigen_data += ag_size;
    }
    ti_antigens.report();

    header_bin->serum_offset = static_cast<decltype(header_bin->serum_offset)>(antigen_data - data_start);
    auto* serum_index = reinterpret_cast<hidb::bin::ASTIndex*>(data_start + header_bin->serum_offset);
    auto* serum_data = reinterpret_cast<ptr_t>(reinterpret_cast<ptr_t>(serum_index) + sizeof(hidb::bin::ast_offset_t) * estimations.number_of_sera + sizeof(hidb::bin::ast_number_t));

    Timeit ti_sera("converting " + acmacs::to_string(estimations.number_of_sera) + " sera: ");
    serum_index->number_of = static_cast<hidb::bin::ast_number_t>(estimations.number_of_sera);
    hidb::bin::ast_offset_t* serum_offset = &serum_index->offset;
    hidb::bin::ast_offset_t previous_serum_offset = 0;
    for (const rjson::object& serum: static_cast<const rjson::array&>(val["s"])) {
        const auto sr_size = make_serum(serum, reinterpret_cast<hidb::bin::Serum*>(serum_data));
        *serum_offset = static_cast<hidb::bin::ast_offset_t>(sr_size) + previous_serum_offset;
        previous_serum_offset = *serum_offset;
        ++serum_offset;
        serum_data += sr_size;
    }
    ti_sera.report();

    header_bin->table_offset = static_cast<decltype(header_bin->table_offset)>(serum_data - data_start);
    auto* table_index = reinterpret_cast<hidb::bin::ASTIndex*>(data_start + header_bin->table_offset);
    auto* table_data = reinterpret_cast<ptr_t>(reinterpret_cast<ptr_t>(table_index) + sizeof(hidb::bin::ast_offset_t) * estimations.number_of_sera + sizeof(hidb::bin::ast_number_t));

    Timeit ti_tables("converting " + acmacs::to_string(estimations.number_of_tables) + " tables: ");
    table_index->number_of = static_cast<hidb::bin::ast_number_t>(estimations.number_of_tables);
    hidb::bin::ast_offset_t* table_offset = &table_index->offset;
    hidb::bin::ast_offset_t previous_table_offset = 0;
    for (const rjson::object& table: static_cast<const rjson::array&>(val["s"])) {
        const auto table_size = make_table(table, reinterpret_cast<hidb::bin::Table*>(table_data));
        *table_offset = static_cast<hidb::bin::ast_offset_t>(table_size) + previous_table_offset;
        previous_table_offset = *table_offset;
        ++table_offset;
        table_data += table_size;
    }
    ti_tables.report();

    result.resize(static_cast<size_t>(table_data - data_start));
    std::cerr << "INFO: hidb bin size: " << result.size() << '\n';
    return result;

} // hidb::json::read

// ----------------------------------------------------------------------

size_t make_antigen(const rjson::object& aSource, hidb::bin::Antigen* aTarget)
{
    if (std::string year = aSource.get_or_default("y", ""); year.size() == 4)
        std::memmove(aTarget->year_data, year.data(), 4);
    else if (!year.empty())
        throw std::runtime_error("Invalid year in " + aSource.to_json());
    if (std::string lineage = aSource.get_or_default("L", ""); lineage.size() == 1)
        aTarget->lineage = lineage[0];
    else if (!lineage.empty())
        throw std::runtime_error("Invalid lineage in " + aSource.to_json());

    auto* const target_base = reinterpret_cast<char*>(aTarget) + sizeof(hidb::bin::Antigen);
    auto set_offset = [target_base,&aSource](uint8_t& offset, char* target) -> void {
        const auto off = static_cast<size_t>(target - target_base);
        if (off > std::numeric_limits<std::decay_t<decltype(offset)>>::max())
            throw std::runtime_error("Overflow when setting offset for a field (antigen): " + acmacs::to_string(off) + " when processing " + aSource.to_json());
        offset = static_cast<std::decay_t<decltype(offset)>>(off);
    };
    auto convert_date = [&aSource](std::string date) -> hidb::bin::date_t {
        if (date.size() != 10)
            throw std::runtime_error("Invalid date in " + aSource.to_json());
        const std::string compacted = date.substr(0, 4) + date.substr(5, 2) + date.substr(8, 2);
        return static_cast<hidb::bin::date_t>(stoul(compacted));
    };

    auto* target = target_base;
    if (auto host = aSource.get_or_default("H", ""); !host.empty()) {
        std::memmove(target, host.data(), host.size());
        target += host.size();
    }
    if (auto location = aSource.get_or_default("O", ""); !location.empty()) {
        std::memmove(target, location.data(), location.size());
        set_offset(aTarget->location_offset, target);
        target += location.size();
    }
    else {
        std::cerr << "WARNING: empty location in " << aSource << '\n';
    }

    set_offset(aTarget->isolation_offset, target);
    if (auto isolation = aSource.get_or_default("i", ""); !isolation.empty()) {
        std::memmove(target, isolation.data(), isolation.size());
        target += isolation.size();
    }
    else {
        std::cerr << "WARNING: empty isolation in " << aSource << '\n';
    }

    set_offset(aTarget->passage_offset, target);
    if (auto passage = aSource.get_or_default("P", ""); !passage.empty()) {
        std::memmove(target, passage.data(), passage.size());
        target += passage.size();
    }

    set_offset(aTarget->reassortant_offset, target);
    if (auto reassortant = aSource.get_or_default("R", ""); !reassortant.empty()) {
        std::memmove(target, reassortant.data(), reassortant.size());
        target += reassortant.size();
    }

    if (const auto& annotations = aSource.get_or_empty_array("a"); annotations.size() <= sizeof(hidb::bin::Antigen::annotation_offset)) {
        for (size_t ann_no = 0; ann_no < sizeof(hidb::bin::Antigen::annotation_offset); ++ann_no) {
            set_offset(aTarget->annotation_offset[ann_no], target);
            if (ann_no < annotations.size()) {
                const std::string ann = annotations[ann_no];
                std::memmove(target, ann.data(), ann.size());
                target += ann.size();
            }
        }
    }
    else
        throw std::runtime_error("Too many annotations in " + aSource.to_json());

    if (const auto& lab_ids = aSource.get_or_empty_array("l"); lab_ids.size() <= sizeof(hidb::bin::Antigen::lab_id_offset)) {
        for (size_t lab_id_no = 0; lab_id_no < sizeof(hidb::bin::Antigen::lab_id_offset); ++lab_id_no) {
            set_offset(aTarget->lab_id_offset[lab_id_no], target);
            if (lab_id_no < lab_ids.size()) {
                const std::string lab_id = lab_ids[lab_id_no];
                std::memmove(target, lab_id.data(), lab_id.size());
                target += lab_id.size();
            }
        }
    }
    else
        throw std::runtime_error("Too many lab ids in " + aSource.to_json());

    set_offset(aTarget->date_offset, target);
    if (const auto& dates = aSource.get_or_empty_array("D"); !dates.empty()) {
        for (size_t date_no = 0; date_no < dates.size(); ++date_no) {
            const auto date = convert_date(dates[date_no]);
            std::memmove(target, &date, sizeof(date));
            target += sizeof(date);
        }
    }

    set_offset(aTarget->table_index_offset, target);
    const auto& tables = aSource.get_or_empty_array("T");
    const auto num_indexes = static_cast<hidb::bin::number_of_table_indexes_t>(tables.size());
    std::memmove(target, &num_indexes, sizeof(num_indexes));
    target += sizeof(num_indexes);
    if (tables.empty())
        throw std::runtime_error("No table indexes in " + aSource.to_json());
    for (size_t no = 0; no < tables.size(); ++no) {
        const auto index = static_cast<hidb::bin::table_index_t>(static_cast<size_t>(tables[no]));
        std::memmove(target, &index, sizeof(index));
        target += sizeof(index);
    }

    size_t size = sizeof(*aTarget) + static_cast<size_t>(target - target_base);
    if (size % 4)
        size += 4 - size % 4;
    return size;

} // make_antigen

// ----------------------------------------------------------------------

size_t make_serum(const rjson::object& aSource, hidb::bin::Serum* aTarget)
{
    if (std::string year = aSource.get_or_default("y", ""); year.size() == 4)
        std::memmove(aTarget->year_data, year.data(), 4);
    else if (!year.empty())
        throw std::runtime_error("Invalid year in " + aSource.to_json());
    if (std::string lineage = aSource.get_or_default("L", ""); lineage.size() == 1)
        aTarget->lineage = lineage[0];
    else if (!lineage.empty())
        throw std::runtime_error("Invalid lineage in " + aSource.to_json());

    auto* const target_base = reinterpret_cast<char*>(aTarget) + sizeof(hidb::bin::Serum);
    auto set_offset = [target_base,&aSource](uint8_t& offset, char* target) -> void {
        const auto off = static_cast<size_t>(target - target_base);
        if (off > std::numeric_limits<std::decay_t<decltype(offset)>>::max())
            throw std::runtime_error("Overflow when setting offset for a field (serum): " + acmacs::to_string(off) + " when processing " + aSource.to_json());
        offset = static_cast<std::decay_t<decltype(offset)>>(off);
    };

    auto* target = target_base;

    if (auto host = aSource.get_or_default("H", ""); !host.empty()) {
        std::memmove(target, host.data(), host.size());
        target += host.size();
    }

    set_offset(aTarget->location_offset, target);
    if (auto location = aSource.get_or_default("O", ""); !location.empty()) {
        std::memmove(target, location.data(), location.size());
        target += location.size();
    }
      // location is empty if name was not recognized
    // else {
    //     std::cerr << "WARNING: empty location in " << aSource << '\n';
    // }

    set_offset(aTarget->isolation_offset, target);
    if (auto isolation = aSource.get_or_default("i", ""); !isolation.empty()) {
        std::memmove(target, isolation.data(), isolation.size());
        target += isolation.size();
    }
    else {
        std::cerr << "WARNING: empty isolation in " << aSource << '\n';
    }

    set_offset(aTarget->passage_offset, target);
    if (auto passage = aSource.get_or_default("P", ""); !passage.empty()) {
        std::memmove(target, passage.data(), passage.size());
        target += passage.size();
    }

    set_offset(aTarget->reassortant_offset, target);
    if (auto reassortant = aSource.get_or_default("R", ""); !reassortant.empty()) {
        std::memmove(target, reassortant.data(), reassortant.size());
        target += reassortant.size();
    }

    if (const auto& annotations = aSource.get_or_empty_array("a"); annotations.size() <= sizeof(hidb::bin::Serum::annotation_offset)) {
        for (size_t ann_no = 0; ann_no < sizeof(hidb::bin::Serum::annotation_offset); ++ann_no) {
            set_offset(aTarget->annotation_offset[ann_no], target);
            if (ann_no < annotations.size()) {
                const std::string ann = annotations[ann_no];
                std::memmove(target, ann.data(), ann.size());
                target += ann.size();
            }
        }
    }
    else
        throw std::runtime_error("Too many annotations (" + std::to_string(annotations.size()) + ", avail: " + std::to_string(sizeof(hidb::bin::Serum::annotation_offset)) + ") in " + aSource.to_json());

    set_offset(aTarget->serum_id_offset, target);
    if (auto serum_id = aSource.get_or_default("I", ""); !serum_id.empty()) {
        std::memmove(target, serum_id.data(), serum_id.size());
        target += serum_id.size();
    }

    set_offset(aTarget->serum_species_offset, target);
    if (auto serum_species = aSource.get_or_default("s", ""); !serum_species.empty()) {
        std::memmove(target, serum_species.data(), serum_species.size());
        target += serum_species.size();
    }

    set_offset(aTarget->homologous_antigen_index_offset, target);
    const auto& homologous = aSource.get_or_empty_array("h");
    for (size_t no = 0; no < homologous.size(); ++no) {
        const auto index = static_cast<hidb::bin::homologous_t>(static_cast<size_t>(homologous[no]));
        std::memmove(target, &index, sizeof(index));
        target += sizeof(index);
    }

    set_offset(aTarget->table_index_offset, target);
    const auto& tables = aSource.get_or_empty_array("T");
    const auto num_indexes = static_cast<hidb::bin::number_of_table_indexes_t>(tables.size());
    std::memmove(target, &num_indexes, sizeof(num_indexes));
    target += sizeof(num_indexes);
    if (tables.empty())
        throw std::runtime_error("No table indexes in " + aSource.to_json());
    for (size_t no = 0; no < tables.size(); ++no) {
        const auto index = static_cast<hidb::bin::table_index_t>(static_cast<size_t>(tables[no]));
        std::memmove(target, &index, sizeof(index));
        target += sizeof(index);
    }

    size_t size = sizeof(*aTarget) + static_cast<size_t>(target - target_base);
    if (size % 4)
        size += 4 - size % 4;
    return size;

} // make_serum

// ----------------------------------------------------------------------

size_t make_table(const rjson::object& aSource, hidb::bin::Table* aTarget)
{
    if (std::string lineage = aSource.get_or_default("L", ""); lineage.size() == 1)
        aTarget->lineage = lineage[0];
    else if (!lineage.empty())
        throw std::runtime_error("Invalid lineage in " + aSource.to_json());

    auto* const target_base = reinterpret_cast<char*>(aTarget) + sizeof(hidb::bin::Table);
    auto set_offset = [target_base,&aSource](uint8_t& offset, char* target) -> void {
        const auto off = static_cast<size_t>(target - target_base);
        if (off > std::numeric_limits<std::decay_t<decltype(offset)>>::max())
            throw std::runtime_error("Overflow when setting offset for a field (table): " + acmacs::to_string(off) + " when processing " + aSource.to_json());
        offset = static_cast<std::decay_t<decltype(offset)>>(off);
    };

    auto* target = target_base;

    if (auto assay = aSource.get_or_default("A", ""); !assay.empty()) {
        std::memmove(target, assay.data(), assay.size());
        target += assay.size();
    }

    set_offset(aTarget->date_offset, target);
    if (auto date = aSource.get_or_default("D", ""); !date.empty()) {
        std::memmove(target, date.data(), date.size());
        target += date.size();
    }
    else {
        std::cerr << "WARNING: table has no date: " << aSource.to_json() << '\n';
    }

    set_offset(aTarget->lab_offset, target);
    if (auto lab = aSource.get_or_default("l", ""); !lab.empty()) {
        std::memmove(target, lab.data(), lab.size());
        target += lab.size();
    }
    else {
        std::cerr << "WARNING: table has no lab: " << aSource.to_json() << '\n';
    }

    set_offset(aTarget->rbc_offset, target);
    if (auto rbc = aSource.get_or_default("l", ""); !rbc.empty()) {
        std::memmove(target, rbc.data(), rbc.size());
        target += rbc.size();
    }

      // padding
    if (size_t size = static_cast<size_t>(target - target_base); size % 4)
        target += 4 - size % 4;

    aTarget->antigen_index_offset = static_cast<decltype(aTarget->antigen_index_offset)>(target - target_base);
    const auto& antigens = aSource.get_or_empty_array("a");
    if (antigens.empty())
        throw std::runtime_error("No antigen indexes in " + aSource.to_json());
    for (size_t no = 0; no < antigens.size(); ++no) {
        const auto index = static_cast<hidb::bin::antigen_index_t>(static_cast<size_t>(antigens[no]));
        std::memmove(target, &index, sizeof(index));
        target += sizeof(index);
    }

    aTarget->serum_index_offset = static_cast<decltype(aTarget->serum_index_offset)>(target - target_base);
    const auto& sera = aSource.get_or_empty_array("a");
    if (sera.empty())
        throw std::runtime_error("No serum indexes in " + aSource.to_json());
    for (size_t no = 0; no < sera.size(); ++no) {
        const auto index = static_cast<hidb::bin::serum_index_t>(static_cast<size_t>(sera[no]));
        std::memmove(target, &index, sizeof(index));
        target += sizeof(index);
    }

    aTarget->titer_offset = static_cast<decltype(aTarget->titer_offset)>(target - target_base);

    size_t size = sizeof(*aTarget) + static_cast<size_t>(target - target_base);
    if (size % 4)
        size += 4 - size % 4;
    return size;

// 1            <lineage>      V, Y, uint8_t(0)
// 4                           antigen indexes offset from assay beginning
// 4                           serum indexes offset from assay beginning
// 4                           titers offset from assay beginning

//              padding        indexes must start at 4
// 4*num-antigens              antigen indexes
// 4*num-sera                  serum indexes
// 1            max titer length
// max-titer-length*num-antigens*num-sera   <titers>  titers for the antigen 0, then antigen 1, etc.

} // make_table

// ----------------------------------------------------------------------

Estimations::Estimations(const rjson::value& aSource)
{
    antigen(aSource["a"]);
    serum(aSource["s"]);
    table(aSource["t"]);

    size = sizeof(hidb::bin::Header)
            + (number_of_antigens + number_of_sera + number_of_tables) * sizeof(hidb::bin::ast_offset_t) + sizeof(hidb::bin::ast_number_t) * 3
            + number_of_antigens * antigen_size + number_of_sera * serum_size + number_of_tables * table_size;

    std::cerr << "Estimated bin size: " << size << '\n';

    std::cerr << "virus_types: " << virus_types << '\n';
    const auto most_often = std::max_element(virus_types.begin(), virus_types.end(), [](const auto& a, const auto& b) -> bool { return a.second < b.second; });
    virus_type = most_often->first;
    std::cerr << "virus_type: " << virus_type << '\n';

} // Estimations::Estimations

// ----------------------------------------------------------------------

void Estimations::antigen(const rjson::array& antigens)
{
    number_of_antigens = antigens.size();

    size_t ag_max_host = 0, ag_max_location = 0, ag_max_isolation = 0, ag_max_passage = 0,
            ag_max_reassortant = 0, ag_max_annotations = 0, ag_max_lab_id = 0, ag_max_num_dates = 0,
            ag_max_all = 0;
    size_t ag_max_num_annotations = 0, ag_max_num_lab_id = 0, ag_max_num_table_indexes = 0;
    for (const auto& antigen: antigens) {
        const auto host = antigen.get_or_default("H", "").size();
        ag_max_host = std::max(ag_max_host, host);
        const auto location = antigen.get_or_default("O", "").size();
        ag_max_location = std::max(ag_max_location, location);
        const auto isolation = antigen.get_or_default("i", "").size();
        ag_max_isolation = std::max(ag_max_isolation, isolation);
        const auto passage = antigen.get_or_default("P", "").size();
        ag_max_passage = std::max(ag_max_passage, passage);
        const auto reassortant = antigen.get_or_default("R", "").size();
        ag_max_reassortant = std::max(ag_max_reassortant, reassortant);
        ag_max_num_annotations = std::max(ag_max_num_annotations, antigen.get_or_empty_array("a").size());
        size_t annotations = 0;
        for (const rjson::string& anno: antigen.get_or_empty_array("a"))
            annotations += anno.size();
        ag_max_annotations = std::max(ag_max_annotations, annotations);
        ag_max_num_lab_id = std::max(ag_max_num_lab_id, antigen.get_or_empty_array("l").size());
        size_t lab_ids = 0;
        for (const rjson::string& li: antigen.get_or_empty_array("l"))
            lab_ids += li.size();
        ag_max_lab_id = std::max(ag_max_lab_id, lab_ids);
        ag_max_num_dates = std::max(ag_max_num_dates, antigen.get_or_empty_array("D").size());
        ag_max_num_table_indexes = std::max(ag_max_num_table_indexes, antigen.get_or_empty_array("T").size());
        ag_max_all = std::max(ag_max_all, host + location + isolation + passage + reassortant + annotations + lab_ids);

        if (const auto vt = antigen.get_or_default("V", ""); !vt.empty())
            ++virus_types.emplace(vt, 0).first->second;
    }

    antigen_size = ag_max_all + sizeof(hidb::bin::Antigen) + ag_max_num_dates * sizeof(hidb::bin::date_t)
            + sizeof(hidb::bin::number_of_table_indexes_t) + ag_max_num_table_indexes * sizeof(hidb::bin::table_index_t);

    std::cerr << "Antigens:           " << number_of_antigens << '\n'
              << "ag_max_host:        " << ag_max_host << '\n'
              << "ag_max_location:    " << ag_max_location << '\n'
              << "ag_max_isolation:   " << ag_max_isolation << '\n'
              << "ag_max_passage:     " << ag_max_passage << '\n'
              << "ag_max_reassortant: " << ag_max_reassortant << '\n'
              << "ag_max_annotations: " << ag_max_annotations << '\n'
              << "ag_max_num_annotat: " << ag_max_num_annotations << '\n'
              << "ag_max_lab_id:      " << ag_max_lab_id << '\n'
              << "ag_max_num_lab_id:  " << ag_max_num_lab_id << '\n'
              << "ag_max_num_dates:   " << ag_max_num_dates << '\n'
              << "ag_max_num_table_i: " << ag_max_num_table_indexes << '\n'
              << "ag_max_all:         " << ag_max_all << '\n'
              << "antigen_size:       " << antigen_size << '\n'
              << "ag_super_max:       " << (ag_max_host + ag_max_location + ag_max_isolation + ag_max_passage + ag_max_reassortant + ag_max_annotations + ag_max_lab_id) << '\n'
              << '\n';

} // Estimations::antigen

// ----------------------------------------------------------------------

void Estimations::serum(const rjson::array& sera)
{
    number_of_sera = sera.size();

    size_t sr_max_host = 0, sr_max_location = 0, sr_max_isolation = 0, sr_max_passage = 0,
            sr_max_reassortant = 0, sr_max_annotations = 0, sr_max_serum_id = 0, sr_max_serum_species = 0,
            sr_max_all = 0;
    size_t sr_max_num_annotations = 0, sr_max_num_table_indexes = 0, sr_max_num_homologous = 0;
    for (const auto& serum: sera) {
        const auto host = serum.get_or_default("H", "").size();
        sr_max_host = std::max(sr_max_host, host);
        const auto location = serum.get_or_default("O", "").size();
        sr_max_location = std::max(sr_max_location, location);
        const auto isolation = serum.get_or_default("i", "").size();
        sr_max_isolation = std::max(sr_max_isolation, isolation);
        const auto passage = serum.get_or_default("P", "").size();
        sr_max_passage = std::max(sr_max_passage, passage);
        const auto reassortant = serum.get_or_default("R", "").size();
        sr_max_reassortant = std::max(sr_max_reassortant, reassortant);
        const auto serum_id = serum.get_or_default("I", "").size();
        sr_max_serum_id = std::max(sr_max_serum_id, serum_id);
        const auto serum_species = serum.get_or_default("s", "").size();
        sr_max_serum_species = std::max(sr_max_serum_species, serum_species);
        sr_max_num_annotations = std::max(sr_max_num_annotations, serum.get_or_empty_array("a").size());
        size_t annotations = 0;
        for (const rjson::string& anno: serum.get_or_empty_array("a"))
            annotations += anno.size();
        sr_max_annotations = std::max(sr_max_annotations, annotations);
        sr_max_num_table_indexes = std::max(sr_max_num_table_indexes, serum.get_or_empty_array("T").size());
        size_t num_homologous = serum.get_or_empty_array("h").size();
        sr_max_num_homologous = std::max(sr_max_num_homologous, num_homologous);
        sr_max_all = std::max(sr_max_all, host + location + isolation + passage + reassortant + annotations + serum_id + serum_species + num_homologous * sizeof(hidb::bin::homologous_t));
        if (const auto vt = serum.get_or_default("V", ""); !vt.empty())
            ++virus_types.emplace(vt, 0).first->second;
    }

    serum_size = sr_max_all + sizeof(hidb::bin::Serum)
            + sr_max_num_homologous * sizeof(hidb::bin::homologous_t)
            + sizeof(hidb::bin::number_of_table_indexes_t) + sr_max_num_table_indexes * sizeof(hidb::bin::table_index_t);

    std::cerr << "Sera:               " << number_of_sera << '\n'
              << "sr_max_host:        " << sr_max_host << '\n'
              << "sr_max_location:    " << sr_max_location << '\n'
              << "sr_max_isolation:   " << sr_max_isolation << '\n'
              << "sr_max_passage:     " << sr_max_passage << '\n'
              << "sr_max_reassortant: " << sr_max_reassortant << '\n'
              << "sr_max_annotations: " << sr_max_annotations << '\n'
              << "sr_max_num_annotat: " << sr_max_num_annotations << '\n'
              << "sr_max_serum_id:    " << sr_max_serum_id << '\n'
              << "sr_max_serum_speci: " << sr_max_serum_species << '\n'
              << "sr_max_num_table_i: " << sr_max_num_table_indexes << '\n'
              << "sr_max_num_homolog: " << sr_max_num_homologous << '\n'
              << "sr_max_all:         " << sr_max_all << '\n'
              << "serum_size:         " << serum_size << '\n'
              << "sr_super_max:       " << (sr_max_host + sr_max_location + sr_max_isolation + sr_max_passage + sr_max_reassortant + sr_max_annotations + sr_max_serum_id + sr_max_serum_species + sr_max_num_homologous * sizeof(hidb::bin::homologous_t)) << '\n'
              << '\n';

} // Estimations::serum

// ----------------------------------------------------------------------

void Estimations::table(const rjson::array& tables)
{
    number_of_tables = tables.size();

    table_size = 0;

} // Estimations::table

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
