#include "acmacs-base/string.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/rjson.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/hidb-bin.hh"

// ----------------------------------------------------------------------

static void raw_stat(const rjson::value& val);

// ----------------------------------------------------------------------

hidb::HiDb::HiDb(std::string aFilename, report_time timer)
{
    Timeit ti("reading hidb from " + aFilename + ": ", timer);
    const std::string data = acmacs::file::read(aFilename);
    if (data.find("\"  version\": \"hidb-v5\"") != std::string::npos)
        read_json(data);
    else
        throw std::runtime_error("[hidb] unrecognized file: " + aFilename);

} // hidb::HiDb::HiDb

// ----------------------------------------------------------------------

void hidb::HiDb::read_json(std::string aData)
{
    const auto val = rjson::parse_string(aData);
    raw_stat(val);

} // hidb::HiDb::read_json

// ----------------------------------------------------------------------

void raw_stat(const rjson::value& val)
{
    const rjson::array& antigens = val["a"];
    size_t ag_max_host = 0, ag_max_location = 0, ag_max_isolation = 0, ag_max_passage = 0, ag_max_reassortant = 0, ag_max_annotations = 0, ag_max_lab_id = 0, ag_max_all = 0;
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
        ag_max_num_table_indexes = std::max(ag_max_num_table_indexes, antigen.get_or_empty_array("T").size());
        ag_max_all = std::max(ag_max_all, host + location + isolation + passage + reassortant + annotations + lab_ids);
    }
    std::cerr << "Antigens:           " << antigens.size() << '\n'
              << "ag_max_host:        " << ag_max_host << '\n'
              << "ag_max_location:    " << ag_max_location << '\n'
              << "ag_max_isolation:   " << ag_max_isolation << '\n'
              << "ag_max_passage:     " << ag_max_passage << '\n'
              << "ag_max_reassortant: " << ag_max_reassortant << '\n'
              << "ag_max_annotations: " << ag_max_annotations << '\n'
              << "ag_max_num_annotat: " << ag_max_num_annotations << '\n'
              << "ag_max_lab_id:      " << ag_max_lab_id << '\n'
              << "ag_max_num_lab_id:  " << ag_max_num_lab_id << '\n'
              << "ag_max_num_table_i: " << ag_max_num_table_indexes << '\n'
              << "ag_max_all:         " << ag_max_all << '\n'
              << "ag_super_max:       " << (ag_max_host + ag_max_location + ag_max_isolation + ag_max_passage + ag_max_reassortant + ag_max_annotations + ag_max_lab_id) << '\n';

} // hidb::HiDb::raw_stat

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
