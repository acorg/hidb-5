#include <map>
#include <cstring>

#include "acmacs-base/string.hh"
#include "acmacs-base/stream.hh"
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

std::string hidb::json::read(std::string aData)
{
    using ptr_t = char*;

    const auto val = rjson::parse_string(aData);
    Estimations estimations{val};

    std::string result;
    result.reserve(estimations.size);

    auto* header_bin = reinterpret_cast<hidb::bin::Header*>(const_cast<char*>(result.data()));
    auto* antigen_index = reinterpret_cast<hidb::bin::ASTIndex*>(reinterpret_cast<ptr_t>(header_bin) + sizeof(*header_bin));
    auto* antigen_0 = reinterpret_cast<hidb::bin::ASTIndex*>(reinterpret_cast<ptr_t>(antigen_index) + sizeof(hidb::bin::ast_offset_t) * estimations.number_of_antigens + sizeof(hidb::bin::ast_number_t));

    header_bin->virus_type_size = static_cast<decltype(header_bin->virus_type_size)>(estimations.virus_type.size());
    std::memset(header_bin->virus_type, 0, static_cast<size_t>(header_bin->virus_type_size));
    std::memmove(header_bin->virus_type, estimations.virus_type.data(), static_cast<size_t>(header_bin->virus_type_size));

    return result;

} // hidb::json::read

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
