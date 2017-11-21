#pragma once

#include <cinttypes>

// ----------------------------------------------------------------------

namespace hidb::bin
{
    using ast_number_t = uint32_t;
    using ast_offset_t = uint32_t;
    using date_t = uint32_t;
    using number_of_table_indexes_t = uint32_t;
    using table_index_t = uint32_t;

    struct Header
    {
        char signature[8]{'H', 'I', 'D', 'B', '0', '5', '0', '0'};
        uint32_t antigen_offset = sizeof(Header);
        uint32_t serum_offset;
        uint32_t table_offset;
        uint8_t  virus_type_size;
        char virus_type[7];

    }; // struct Header

    struct ASTIndex
    {
        ast_number_t number_of;
        ast_offset_t offset;
    };

    struct Antigen
    {
        uint8_t location_offset; // from the beginning of host (end of year)
        uint8_t isolation_offset;
        uint8_t passage_offset;
        uint8_t reassortant_offset;

        uint8_t annotation_1_offset;
        uint8_t annotation_2_offset;
        uint8_t annotation_3_offset;
        uint8_t lab_id_1_offset;

        uint8_t lab_id_2_offset;
        uint8_t lab_id_3_offset;
        uint8_t lab_id_4_offset;
        uint8_t lab_id_5_offset;

        uint8_t date_offset;
        uint8_t table_index_offset;
        char lineage;
        uint8_t _padding1;

        char year[4];

//             <host>
//             <location>
//             <isolation>
//             <passage>
//             <reassortant>
//             <annotation-1>
//             <annotation-2>
//             <annotation-3>
//             <lab_id-1>
//             <lab_id-2>
//             <lab_id-3>
//             <lab_id-4>
//             <lab_id-5>
//                             padding, dates must start at 4
// 4*num-dates <dates>         date is uint32, e.g. 20170101
// 4           <num-indexes>   number of table indexes
// 4*num-indexes               index of table
    }; // struct Antigen

} // namespace hidb::bin

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
