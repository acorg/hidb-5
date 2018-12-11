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
    Estimations(const rjson::value& aSource, bool verbose);

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
    void antigen(const rjson::value& antigens, bool verbose);
    void serum(const rjson::value& sera, bool verbose);
    void table(const rjson::value& tables, bool verbose);

}; // class Estimations

// ----------------------------------------------------------------------

static size_t make_antigen(const rjson::value& aSource, hidb::bin::Antigen* aTarget);
static size_t make_serum(const rjson::value& aSource, hidb::bin::Serum* aTarget);
static size_t make_table(const rjson::value& aSource, hidb::bin::Table* aTarget);

// ----------------------------------------------------------------------

std::string hidb::json::read(std::string aData, bool verbose)
{
    using ptr_t = char*;

    const auto val = rjson::parse_string(aData);
    Estimations estimations(val, verbose);

    std::string result(estimations.size, 0);

    auto* const data_start = const_cast<char*>(result.data());
    auto* const header_bin = reinterpret_cast<hidb::bin::Header*>(data_start);
    std::string sig = hidb::bin::signature();
    std::memmove(header_bin->signature, sig.data(), sig.size());
    header_bin->antigen_offset = sizeof(hidb::bin::Header);

    auto* antigen_index = reinterpret_cast<hidb::bin::ASTIndex*>(data_start + sizeof(*header_bin));
    auto* antigen_data = reinterpret_cast<ptr_t>(reinterpret_cast<ptr_t>(antigen_index) + sizeof(hidb::bin::ast_offset_t) * (estimations.number_of_antigens + 1) + sizeof(hidb::bin::ast_number_t));

    header_bin->virus_type_size = static_cast<decltype(header_bin->virus_type_size)>(estimations.virus_type.size());
    std::memset(header_bin->virus_type_, 0, static_cast<size_t>(header_bin->virus_type_size));
    std::memmove(header_bin->virus_type_, estimations.virus_type.data(), static_cast<size_t>(header_bin->virus_type_size));

    Timeit ti_antigens("converting " + acmacs::to_string(estimations.number_of_antigens) + " antigens: ", do_report_time(verbose));
    antigen_index->number_of = static_cast<hidb::bin::ast_number_t>(estimations.number_of_antigens);
    hidb::bin::ast_offset_t* antigen_offset = &antigen_index->offset;
    *antigen_offset = 0;
    ++antigen_offset;
    hidb::bin::ast_offset_t previous_antigen_offset = 0;
    rjson::for_each(val["a"], [&antigen_data, &antigen_offset, &previous_antigen_offset](const rjson::value& antigen) {
        const auto ag_size = make_antigen(antigen, reinterpret_cast<hidb::bin::Antigen*>(antigen_data));
        *antigen_offset = static_cast<hidb::bin::ast_offset_t>(ag_size) + previous_antigen_offset;
        previous_antigen_offset = *antigen_offset;
        ++antigen_offset;
        antigen_data += ag_size;
    });
    ti_antigens.report();

    header_bin->serum_offset = static_cast<decltype(header_bin->serum_offset)>(antigen_data - data_start);
    auto* serum_index = reinterpret_cast<hidb::bin::ASTIndex*>(data_start + header_bin->serum_offset);
    auto* serum_data = reinterpret_cast<ptr_t>(reinterpret_cast<ptr_t>(serum_index) + sizeof(hidb::bin::ast_offset_t) * (estimations.number_of_sera + 1) + sizeof(hidb::bin::ast_number_t));

    Timeit ti_sera("converting " + acmacs::to_string(estimations.number_of_sera) + " sera: ", do_report_time(verbose));
    serum_index->number_of = static_cast<hidb::bin::ast_number_t>(estimations.number_of_sera);
    hidb::bin::ast_offset_t* serum_offset = &serum_index->offset;
    *serum_offset = 0;
    ++serum_offset;
    hidb::bin::ast_offset_t previous_serum_offset = 0;
    rjson::for_each(val["s"], [&serum_data, &serum_offset, &previous_serum_offset](const rjson::value& serum) {
        const auto sr_size = make_serum(serum, reinterpret_cast<hidb::bin::Serum*>(serum_data));
        *serum_offset = static_cast<hidb::bin::ast_offset_t>(sr_size) + previous_serum_offset;
        previous_serum_offset = *serum_offset;
        ++serum_offset;
        serum_data += sr_size;
    });
    ti_sera.report();

    header_bin->table_offset = static_cast<decltype(header_bin->table_offset)>(serum_data - data_start);
    auto* table_index = reinterpret_cast<hidb::bin::ASTIndex*>(data_start + header_bin->table_offset);
    auto* table_data = reinterpret_cast<ptr_t>(reinterpret_cast<ptr_t>(table_index) + sizeof(hidb::bin::ast_offset_t) * (estimations.number_of_tables + 1) + sizeof(hidb::bin::ast_number_t));

    Timeit ti_tables("converting " + acmacs::to_string(estimations.number_of_tables) + " tables: ", do_report_time(verbose));
    table_index->number_of = static_cast<hidb::bin::ast_number_t>(estimations.number_of_tables);
    hidb::bin::ast_offset_t* table_offset = &table_index->offset;
    *table_offset = 0;
    ++table_offset;
    hidb::bin::ast_offset_t previous_table_offset = 0;
    rjson::for_each(val["t"], [&table_data, &table_offset, &previous_table_offset](const rjson::value& table) {
        const auto table_size = make_table(table, reinterpret_cast<hidb::bin::Table*>(table_data));
        *table_offset = static_cast<hidb::bin::ast_offset_t>(table_size) + previous_table_offset;
        previous_table_offset = *table_offset;
        ++table_offset;
        table_data += table_size;
    });
    ti_tables.report();

    result.resize(static_cast<size_t>(table_data - data_start));
    if (verbose)
        std::cerr << "INFO: hidb bin size: " << result.size() << '\n';
    if (result.size() > estimations.size)
        std::cerr << "WARNING: data overflow: " << (result.size() - estimations.size) << " bytes " << std::fixed << std::setprecision(1) << (100.0 * (result.size() - estimations.size) / result.size())
                  << "%\n";
    else if (verbose)
        std::cerr << "size estimation extra: " << (estimations.size - result.size()) << " bytes " << std::fixed << std::setprecision(1) << (100.0 * (estimations.size - result.size()) / result.size())
                  << "%\n";
    return result;

} // hidb::json::read

// ----------------------------------------------------------------------

size_t make_antigen(const rjson::value& aSource, hidb::bin::Antigen* aTarget)
{
    if (const auto& year = aSource["y"]; year.size() == 4)
        std::memmove(aTarget->year_data, static_cast<std::string_view>(year).data(), 4);
    else if (!year.empty())
        throw std::runtime_error("Invalid year in " + rjson::to_string(aSource));
    if (const auto& lineage = aSource["L"]; lineage.size() == 1)
        aTarget->lineage = static_cast<std::string_view>(lineage)[0];
    else if (!lineage.empty())
        throw std::runtime_error("Invalid lineage in " + rjson::to_string(aSource));

    auto* const target_base = reinterpret_cast<char*>(aTarget) + sizeof(hidb::bin::Antigen);
    auto set_offset = [target_base, &aSource](uint8_t& offset, char* target) -> void {
        const auto off = static_cast<size_t>(target - target_base);
        if (off > std::numeric_limits<std::decay_t<decltype(offset)>>::max())
            throw std::runtime_error("Overflow when setting offset for a field (antigen): " + acmacs::to_string(off) + " when processing " + rjson::to_string(aSource));
        offset = static_cast<std::decay_t<decltype(offset)>>(off);
    };

    auto* target = target_base;
    if (const auto& host = aSource["H"]; !host.empty()) {
        std::memmove(target, static_cast<std::string_view>(host).data(), host.size());
        target += host.size();
    }
    if (const auto& location = aSource["O"]; !location.empty()) {
        std::memmove(target, static_cast<std::string_view>(location).data(), location.size());
        set_offset(aTarget->location_offset, target);
        target += location.size();
    }
    else {
        std::cerr << "WARNING: empty location in " << aSource << '\n';
    }

    set_offset(aTarget->isolation_offset, target);
    if (const auto& isolation = aSource["i"]; !isolation.empty()) {
        std::memmove(target, static_cast<std::string_view>(isolation).data(), isolation.size());
        target += isolation.size();
    }
    else {
        std::cerr << "WARNING: empty isolation in " << aSource << '\n';
    }

    set_offset(aTarget->passage_offset, target);
    if (const auto& passage = aSource["P"]; !passage.empty()) {
        std::memmove(target, static_cast<std::string_view>(passage).data(), passage.size());
        target += passage.size();
    }

    set_offset(aTarget->reassortant_offset, target);
    if (const auto& reassortant = aSource["R"]; !reassortant.empty()) {
        std::memmove(target, static_cast<std::string_view>(reassortant).data(), reassortant.size());
        target += reassortant.size();
    }

    if (const auto& annotations = aSource["a"]; annotations.size() <= sizeof(hidb::bin::Antigen::annotation_offset)) {
        for (size_t ann_no = 0; ann_no < sizeof(hidb::bin::Antigen::annotation_offset); ++ann_no) {
            set_offset(aTarget->annotation_offset[ann_no], target);
            if (ann_no < annotations.size()) {
                const std::string_view ann = annotations[ann_no];
                std::memmove(target, ann.data(), ann.size());
                target += ann.size();
            }
        }
    }
    else
        throw std::runtime_error("Too many annotations in " + rjson::to_string(aSource));

    if (const auto& lab_ids = aSource["l"]; lab_ids.size() <= sizeof(hidb::bin::Antigen::lab_id_offset)) {
        for (size_t lab_id_no = 0; lab_id_no < sizeof(hidb::bin::Antigen::lab_id_offset); ++lab_id_no) {
            set_offset(aTarget->lab_id_offset[lab_id_no], target);
            if (lab_id_no < lab_ids.size()) {
                const std::string_view lab_id = lab_ids[lab_id_no];
                std::memmove(target, lab_id.data(), lab_id.size());
                target += lab_id.size();
            }
        }
    }
    else
        throw std::runtime_error("Too many lab ids in " + rjson::to_string(aSource));

    // padding
    if (size_t size = static_cast<size_t>(target - target_base); size % 4)
        target += 4 - size % 4;

    set_offset(aTarget->date_offset, target);
    if (const auto& dates = aSource["D"]; !dates.empty()) {
        for (size_t date_no = 0; date_no < dates.size(); ++date_no) {
            try {
                const auto date = hidb::bin::Antigen::make_date(dates[date_no]);
                std::memmove(target, &date, sizeof(date));
                target += sizeof(date);
            }
            catch (hidb::bin::invalid_date&) {
                throw std::runtime_error("Invalid date in " + rjson::to_string(aSource));
            }
        }
    }

    set_offset(aTarget->table_index_offset, target);
    const auto& tables = aSource["T"];
    const auto num_indexes = static_cast<hidb::bin::number_of_table_indexes_t>(tables.size());
    std::memmove(target, &num_indexes, sizeof(num_indexes));
    target += sizeof(num_indexes);
    if (tables.empty())
        throw std::runtime_error("No table indexes in " + rjson::to_string(aSource));
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

size_t make_serum(const rjson::value& aSource, hidb::bin::Serum* aTarget)
{
    if (const auto& year = aSource["y"]; year.size() == 4)
        std::memmove(aTarget->year_data, static_cast<std::string_view>(year).data(), 4);
    else if (!year.empty())
        throw std::runtime_error("Invalid year in " + rjson::to_string(aSource));
    if (const auto& lineage = aSource["L"]; lineage.size() == 1)
        aTarget->lineage = static_cast<std::string_view>(lineage)[0];
    else if (!lineage.empty())
        throw std::runtime_error("Invalid lineage in " + rjson::to_string(aSource));

    auto* const target_base = reinterpret_cast<char*>(aTarget) + sizeof(hidb::bin::Serum);
    auto set_offset = [target_base,&aSource](uint8_t& offset, char* target) -> void {
        const auto off = static_cast<size_t>(target - target_base);
        if (off > std::numeric_limits<std::decay_t<decltype(offset)>>::max())
            throw std::runtime_error("Overflow when setting offset for a field (serum): " + acmacs::to_string(off) + " when processing " + rjson::to_string(aSource));
        offset = static_cast<std::decay_t<decltype(offset)>>(off);
    };

    auto* target = target_base;

    if (const auto& host = aSource["H"]; !host.empty()) {
        std::memmove(target, static_cast<std::string_view>(host).data(), host.size());
        target += host.size();
    }

    set_offset(aTarget->location_offset, target);
    if (const auto& location = aSource["O"]; !location.empty()) {
        std::memmove(target, static_cast<std::string_view>(location).data(), location.size());
        target += location.size();
    }
      // location is empty if name was not recognized
    // else {
    //     std::cerr << "WARNING: empty location in " << aSource << '\n';
    // }

    set_offset(aTarget->isolation_offset, target);
    if (const auto& isolation = aSource["i"]; !isolation.empty()) {
        std::memmove(target, static_cast<std::string_view>(isolation).data(), isolation.size());
        target += isolation.size();
    }
    else {
        std::cerr << "WARNING: empty isolation in " << aSource << '\n';
    }

    set_offset(aTarget->passage_offset, target);
    if (const auto& passage = aSource["P"]; !passage.empty()) {
        std::memmove(target, static_cast<std::string_view>(passage).data(), passage.size());
        target += passage.size();
    }

    set_offset(aTarget->reassortant_offset, target);
    if (const auto& reassortant = aSource["R"]; !reassortant.empty()) {
        std::memmove(target, static_cast<std::string_view>(reassortant).data(), reassortant.size());
        target += reassortant.size();
    }

    if (const auto& annotations = aSource["a"]; annotations.size() <= sizeof(hidb::bin::Serum::annotation_offset)) {
        for (size_t ann_no = 0; ann_no < sizeof(hidb::bin::Serum::annotation_offset); ++ann_no) {
            set_offset(aTarget->annotation_offset[ann_no], target);
            if (ann_no < annotations.size()) {
                const std::string_view ann = annotations[ann_no];
                std::memmove(target, ann.data(), ann.size());
                target += ann.size();
            }
        }
    }
    else
        throw std::runtime_error("Too many annotations (" + std::to_string(annotations.size()) + ", avail: " + std::to_string(sizeof(hidb::bin::Serum::annotation_offset)) + ") in " + rjson::to_string(aSource));

    set_offset(aTarget->serum_id_offset, target);
    if (const auto& serum_id = aSource["I"]; !serum_id.empty()) {
        std::memmove(target, static_cast<std::string_view>(serum_id).data(), serum_id.size());
        target += serum_id.size();
    }

    set_offset(aTarget->serum_species_offset, target);
    if (const auto& serum_species = aSource["s"]; !serum_species.empty()) {
        std::memmove(target, static_cast<std::string_view>(serum_species).data(), serum_species.size());
        target += serum_species.size();
    }

      // padding
    if (size_t size = static_cast<size_t>(target - target_base); size % 4)
        target += 4 - size % 4;

    set_offset(aTarget->homologous_antigen_index_offset, target);
    const auto& homologous = aSource["h"];
    for (size_t no = 0; no < homologous.size(); ++no) {
        const auto index = static_cast<hidb::bin::homologous_t>(static_cast<size_t>(homologous[no]));
        std::memmove(target, &index, sizeof(index));
        target += sizeof(index);
    }

    set_offset(aTarget->table_index_offset, target);
    const auto& tables = aSource["T"];
    const auto num_indexes = static_cast<hidb::bin::number_of_table_indexes_t>(tables.size());
    std::memmove(target, &num_indexes, sizeof(num_indexes));
    target += sizeof(num_indexes);
    if (tables.empty())
        throw std::runtime_error("No table indexes in " + rjson::to_string(aSource));
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

size_t make_table(const rjson::value& aSource, hidb::bin::Table* aTarget)
{
    if (const auto& lineage = aSource["L"]; lineage.size() == 1)
        aTarget->lineage = static_cast<std::string_view>(lineage)[0];
    else if (!lineage.empty())
        throw std::runtime_error("Invalid lineage in " + rjson::to_string(aSource));

    auto* const target_base = reinterpret_cast<char*>(aTarget) + sizeof(hidb::bin::Table);
    auto set_offset = [target_base,&aSource](uint8_t& offset, char* target) -> void {
        const auto off = static_cast<size_t>(target - target_base);
        if (off > std::numeric_limits<std::decay_t<decltype(offset)>>::max())
            throw std::runtime_error("Overflow when setting offset for a field (table): " + acmacs::to_string(off) + " when processing " + rjson::to_string(aSource));
        offset = static_cast<std::decay_t<decltype(offset)>>(off);
    };

    auto* target = target_base;

    if (const auto& assay = aSource["A"]; !assay.empty()) {
        std::memmove(target, static_cast<std::string_view>(assay).data(), assay.size());
        target += assay.size();
    }

    set_offset(aTarget->date_offset, target);
    if (const auto& date = aSource["D"]; !date.empty()) {
        std::memmove(target, static_cast<std::string_view>(date).data(), date.size());
        target += date.size();
    }
    else {
        std::cerr << "WARNING: table has no date: " << rjson::to_string(aSource) << '\n';
    }

    set_offset(aTarget->lab_offset, target);
    if (const auto& lab = aSource["l"]; !lab.empty()) {
        std::memmove(target, static_cast<std::string_view>(lab).data(), lab.size());
        target += lab.size();
    }
    else {
        std::cerr << "WARNING: table has no lab: " << rjson::to_string(aSource) << '\n';
    }

    set_offset(aTarget->rbc_offset, target);
    if (const auto& rbc = aSource["r"]; !rbc.empty()) {
        std::memmove(target, static_cast<std::string_view>(rbc).data(), rbc.size());
        target += rbc.size();
    }

      // padding
    if (size_t size = static_cast<size_t>(target - target_base); size % 4)
        target += 4 - size % 4;

    aTarget->antigen_index_offset = static_cast<decltype(aTarget->antigen_index_offset)>(target - target_base);
    const auto& antigens = aSource["a"];
    if (antigens.empty())
        throw std::runtime_error("No antigen indexes in " + rjson::to_string(aSource));
    for (size_t no = 0; no < antigens.size(); ++no) {
        const auto index = static_cast<hidb::bin::antigen_index_t>(static_cast<size_t>(antigens[no]));
        std::memmove(target, &index, sizeof(index));
        target += sizeof(index);
    }

    aTarget->serum_index_offset = static_cast<decltype(aTarget->serum_index_offset)>(target - target_base);
    const auto& sera = aSource["s"];
    if (sera.empty())
        throw std::runtime_error("No serum indexes in " + rjson::to_string(aSource));
    for (size_t no = 0; no < sera.size(); ++no) {
        const auto index = static_cast<hidb::bin::serum_index_t>(static_cast<size_t>(sera[no]));
        std::memmove(target, &index, sizeof(index));
        target += sizeof(index);
    }

    aTarget->titer_offset = static_cast<decltype(aTarget->titer_offset)>(target - target_base);
    const auto& titers = aSource["t"];
    size_t max_titer_size = 0;
    for (size_t ag_no = 0; ag_no < antigens.size(); ++ag_no) {
        for (size_t sr_no = 0; sr_no < sera.size(); ++sr_no) {
            max_titer_size = std::max(max_titer_size, static_cast<std::string_view>(titers[ag_no][sr_no]).size());
        }
    }
    *target = static_cast<char>(max_titer_size);
    ++target;
    for (size_t ag_no = 0; ag_no < antigens.size(); ++ag_no) {
        for (size_t sr_no = 0; sr_no < sera.size(); ++sr_no) {
            std::string titer(titers[ag_no][sr_no]);
            titer.resize(max_titer_size, 0);
            std::memmove(target, titer.data(), titer.size());
            target += titer.size();
        }
    }

    size_t size = sizeof(*aTarget) + static_cast<size_t>(target - target_base);
    if (size % 4)
        size += 4 - size % 4;
    return size;

} // make_table

// ----------------------------------------------------------------------

Estimations::Estimations(const rjson::value& aSource, bool verbose)
{
    antigen(aSource["a"], verbose);
    serum(aSource["s"], verbose);
    table(aSource["t"], verbose);

    size = sizeof(hidb::bin::Header)
            + (number_of_antigens + number_of_sera + number_of_tables) * sizeof(hidb::bin::ast_offset_t) + sizeof(hidb::bin::ast_number_t) * 3
            + number_of_antigens * antigen_size + number_of_sera * serum_size + number_of_tables * table_size
              // + 1024
            ;

    if (verbose)
        std::cerr << "Estimated bin size: " << size << '\n';

    if (verbose)
        std::cerr << "virus_types: " << virus_types << '\n';
    const auto most_often = std::max_element(virus_types.begin(), virus_types.end(), [](const auto& a, const auto& b) -> bool { return a.second < b.second; });
    virus_type = most_often->first;
    if (verbose)
        std::cerr << "virus_type: " << virus_type << '\n';

} // Estimations::Estimations

// ----------------------------------------------------------------------

void Estimations::antigen(const rjson::value& antigens, bool verbose)
{
    number_of_antigens = antigens.size();

    size_t ag_max_host = 0, ag_max_location = 0, ag_max_isolation = 0, ag_max_passage = 0,
            ag_max_reassortant = 0, ag_max_annotations = 0, ag_max_lab_id = 0, ag_max_num_dates = 0,
            ag_max_all = 0;
    size_t ag_max_num_annotations = 0, ag_max_num_lab_id = 0, ag_max_num_table_indexes = 0;
    rjson::for_each(antigens, [&](const rjson::value& antigen) {
        const auto host = antigen["H"].size();
        ag_max_host = std::max(ag_max_host, host);
        const auto location = antigen["O"].size();
        ag_max_location = std::max(ag_max_location, location);
        const auto isolation = antigen["i"].size();
        ag_max_isolation = std::max(ag_max_isolation, isolation);
        const auto passage = antigen["P"].size();
        ag_max_passage = std::max(ag_max_passage, passage);
        const auto reassortant = antigen["R"].size();
        ag_max_reassortant = std::max(ag_max_reassortant, reassortant);
        ag_max_num_annotations = std::max(ag_max_num_annotations, antigen["a"].size());
        size_t annotations = 0;
        rjson::for_each(antigen["a"], [&annotations](const rjson::value& anno) { annotations += anno.size(); });
        ag_max_annotations = std::max(ag_max_annotations, annotations);
        ag_max_num_lab_id = std::max(ag_max_num_lab_id, antigen["l"].size());
        size_t lab_ids = 0;
        rjson::for_each(antigen["l"], [&lab_ids](const rjson::value& li) { lab_ids += li.size(); });
        ag_max_lab_id = std::max(ag_max_lab_id, lab_ids);
        ag_max_num_dates = std::max(ag_max_num_dates, antigen["D"].size());
        ag_max_num_table_indexes = std::max(ag_max_num_table_indexes, antigen["T"].size());
        ag_max_all = std::max(ag_max_all, host + location + isolation + passage + reassortant + annotations + lab_ids);

        if (const auto& vt = antigen["V"]; !vt.empty())
            ++virus_types.emplace(vt, 0).first->second;
    });

    antigen_size = ag_max_all + sizeof(hidb::bin::Antigen) + ag_max_num_dates * sizeof(hidb::bin::date_t)
            + sizeof(hidb::bin::number_of_table_indexes_t) + ag_max_num_table_indexes * sizeof(hidb::bin::table_index_t);

    if (verbose) {
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
    }

} // Estimations::antigen

// ----------------------------------------------------------------------

void Estimations::serum(const rjson::value& sera, bool verbose)
{
    number_of_sera = sera.size();

    size_t sr_max_host = 0, sr_max_location = 0, sr_max_isolation = 0, sr_max_passage = 0,
            sr_max_reassortant = 0, sr_max_annotations = 0, sr_max_serum_id = 0, sr_max_serum_species = 0,
            sr_max_all = 0;
    size_t sr_max_num_annotations = 0, sr_max_num_table_indexes = 0, sr_max_num_homologous = 0;
    rjson::for_each(sera, [&](const rjson::value& serum) {
        const auto host = serum["H"].size();
        sr_max_host = std::max(sr_max_host, host);
        const auto location = serum["O"].size();
        sr_max_location = std::max(sr_max_location, location);
        const auto isolation = serum["i"].size();
        sr_max_isolation = std::max(sr_max_isolation, isolation);
        const auto passage = serum["P"].size();
        sr_max_passage = std::max(sr_max_passage, passage);
        const auto reassortant = serum["R"].size();
        sr_max_reassortant = std::max(sr_max_reassortant, reassortant);
        const auto serum_id = serum["I"].size();
        sr_max_serum_id = std::max(sr_max_serum_id, serum_id);
        const auto serum_species = serum["s"].size();
        sr_max_serum_species = std::max(sr_max_serum_species, serum_species);
        sr_max_num_annotations = std::max(sr_max_num_annotations, serum["a"].size());
        size_t annotations = 0;
        rjson::for_each(serum["a"], [&annotations](const rjson::value& anno) { annotations += anno.size(); });
        sr_max_annotations = std::max(sr_max_annotations, annotations);
        sr_max_num_table_indexes = std::max(sr_max_num_table_indexes, serum["T"].size());
        size_t num_homologous = serum["h"].size();
        sr_max_num_homologous = std::max(sr_max_num_homologous, num_homologous);
        sr_max_all = std::max(sr_max_all, host + location + isolation + passage + reassortant + annotations + serum_id + serum_species + num_homologous * sizeof(hidb::bin::homologous_t));
        if (const auto vt = serum["V"]; !vt.empty())
            ++virus_types.emplace(vt, 0).first->second;
    });

    serum_size = sr_max_all + sizeof(hidb::bin::Serum)
            + sr_max_num_homologous * sizeof(hidb::bin::homologous_t)
            + sizeof(hidb::bin::number_of_table_indexes_t) + sr_max_num_table_indexes * sizeof(hidb::bin::table_index_t);

    if (verbose) {
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
    }

} // Estimations::serum

// ----------------------------------------------------------------------

void Estimations::table(const rjson::value& tables, bool verbose)
{
    constexpr const size_t average_titer_length = 5;

    size_t fields_size = 0, antigens = 0, sera = 0;;

    number_of_tables = tables.size();
    rjson::for_each(tables, [&](const rjson::value& table) {
        fields_size += table["A"].size() // assay
                + table["D"].size()      // date
                + table["l"].size()      // lab
                + table["r"].size()      // rbc
                ;
        antigens += table["a"].size();
        sera += table["s"].size();
    });
    const auto antigens_per_table = antigens / number_of_tables + 1;
    const auto sera_per_table = sera / number_of_tables + 1;

    table_size = sizeof(hidb::bin::Table)
            + fields_size / number_of_tables
            + 1                 // padding
            + sizeof(uint32_t) * antigens_per_table
            + sizeof(uint32_t) * sera_per_table
            + 1                    // max titer length
            + average_titer_length * antigens_per_table * sera_per_table
            ;

    if (verbose) {
        std::cerr << "Tables:               " << number_of_tables << '\n'
                  << "table_size:         " << table_size << '\n'
                  << '\n';
    }

} // Estimations::table

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
