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

// ----------------------------------------------------------------------

std::string hidb::json::read(std::string aData)
{
    using ptr_t = char*;

    const auto val = rjson::parse_string(aData);
    Estimations estimations{val};

    std::string result(estimations.size, 0);

    auto* header_bin = reinterpret_cast<hidb::bin::Header*>(const_cast<char*>(result.data()));
    auto* antigen_index = reinterpret_cast<hidb::bin::ASTIndex*>(reinterpret_cast<ptr_t>(header_bin) + sizeof(*header_bin));
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

    return result;

} // hidb::json::read

// ----------------------------------------------------------------------

size_t make_antigen(const rjson::object& aSource, hidb::bin::Antigen* aTarget)
{
    if (std::string year = aSource.get_or_default("y", ""); year.size() == 4)
        std::memmove(aTarget->year, year.data(), 4);
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
    if (auto isolation = aSource.get_or_default("i", ""); !isolation.empty()) {
        std::memmove(target, isolation.data(), isolation.size());
        set_offset(aTarget->isolation_offset, target);
        target += isolation.size();
    }
    else {
        std::cerr << "WARNING: empty isolation in " << aSource << '\n';
    }
    if (auto passage = aSource.get_or_default("P", ""); !passage.empty()) {
        std::memmove(target, passage.data(), passage.size());
        set_offset(aTarget->passage_offset, target);
        target += passage.size();
    }
    if (auto reassortant = aSource.get_or_default("P", ""); !reassortant.empty()) {
        std::memmove(target, reassortant.data(), reassortant.size());
        set_offset(aTarget->reassortant_offset, target);
        target += reassortant.size();
    }
    if (const auto& annotations = aSource.get_or_empty_array("a"); annotations.size() < sizeof(hidb::bin::Antigen::annotation_offset)) {
        for (size_t ann_no = 0; ann_no < annotations.size(); ++ann_no) {
            const std::string ann = annotations[ann_no];
            std::memmove(target, ann.data(), ann.size());
            set_offset(aTarget->annotation_offset[ann_no], target);
            target += ann.size();
        }
    }
    else
        throw std::runtime_error("Too many annotations in " + aSource.to_json());
    if (const auto& lab_ids = aSource.get_or_empty_array("l"); lab_ids.size() < sizeof(hidb::bin::Antigen::lab_id_offset)) {
        for (size_t lab_id_no = 0; lab_id_no < lab_ids.size(); ++lab_id_no) {
            const std::string lab_id = lab_ids[lab_id_no];
            std::memmove(target, lab_id.data(), lab_id.size());
            set_offset(aTarget->lab_id_offset[lab_id_no], target);
            target += lab_id.size();
        }
    }
    else
        throw std::runtime_error("Too many lab ids in " + aSource.to_json());
    if (const auto& dates = aSource.get_or_empty_array("D"); !dates.empty()) {
        set_offset(aTarget->date_offset, target);
        for (size_t date_no = 0; date_no < dates.size(); ++date_no) {
            const auto date = convert_date(dates[date_no]);
            std::memmove(target, &date, sizeof(date));
            target += sizeof(date);
        }
    }

    if (const auto& tables = aSource.get_or_empty_array("T"); !tables.empty()) {
        set_offset(aTarget->table_index_offset, target);
        const auto num_indexes = static_cast<hidb::bin::number_of_table_indexes_t>(tables.size());
        std::memmove(target, &num_indexes, sizeof(num_indexes));
        target += sizeof(num_indexes);
        for (size_t no = 0; no < tables.size(); ++no) {
            const auto index = static_cast<hidb::bin::table_index_t>(static_cast<size_t>(tables[no]));
            std::memmove(target, &index, sizeof(index));
            target += sizeof(index);
        }
    }
    else
        throw std::runtime_error("No table indexes in " + aSource.to_json());

    return sizeof(*aTarget) + static_cast<size_t>(target - target_base);

} // make_antigen

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

    serum_size = 0;

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
