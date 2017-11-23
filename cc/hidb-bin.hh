#pragma once

#include <string>
#include <cinttypes>

// ----------------------------------------------------------------------

namespace hidb::bin
{
    using ast_number_t = uint32_t;
    using ast_offset_t = uint32_t;
    using date_t = uint32_t;
    using number_of_table_indexes_t = uint32_t;
    using table_index_t = uint32_t;
      // using number_of_homologous_t = uint32_t;
    using homologous_t = uint32_t;
    using antigen_index_t = uint32_t;
    using serum_index_t = uint32_t;

    struct Header
    {
        char signature[8];
        uint32_t antigen_offset;
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

      // ----------------------------------------------------------------------

    struct Antigen
    {
        uint8_t location_offset; // from the beginning of host (end of year)
        uint8_t isolation_offset;
        uint8_t passage_offset;
        uint8_t reassortant_offset;

        uint8_t annotation_offset[3];
        uint8_t lab_id_offset[5];

        uint8_t date_offset;
        uint8_t table_index_offset;
        char lineage;
        uint8_t _padding1;

        char year_data[4];

        std::string name() const;
        std::string date() const;
        inline std::string passage() const { return std::string(_start() + passage_offset, reassortant_offset - passage_offset); }
        inline std::string reassortant() const { return std::string(_start() + reassortant_offset, annotation_offset[0] - reassortant_offset); }
        std::vector<std::string> lab_ids() const;
        std::vector<std::string> annotations() const;

        inline std::pair<number_of_table_indexes_t, const table_index_t*> tables() const
            {
                return {*reinterpret_cast<const number_of_table_indexes_t*>(_start() + table_index_offset),
                        reinterpret_cast<const table_index_t*>(_start() + table_index_offset + sizeof(number_of_table_indexes_t))};
            }

        inline const char* _start() const { return reinterpret_cast<const char*>(this) + sizeof(*this); }
        inline std::string host() const { return std::string(_start(), location_offset); }
        inline std::string location() const { return std::string(_start() + location_offset, isolation_offset - location_offset); }
        inline std::string isolation() const { return std::string(_start() + isolation_offset, passage_offset - isolation_offset); }
        inline std::string year() const { if (*year_data) return std::string(year_data, sizeof(year_data)); else return std::string{}; }

    }; // struct Antigen

      // ----------------------------------------------------------------------

    struct Serum
    {
        uint8_t location_offset; // from the beginning of host (end of year)
        uint8_t isolation_offset;
        uint8_t passage_offset;
        uint8_t reassortant_offset;

        uint8_t annotation_offset[3];
        uint8_t serum_id_offset;

        uint8_t serum_species_offset;
        uint8_t homologous_antigen_index_offset;
        uint8_t table_index_offset;
        char lineage;

        char year_data[4];

        std::string name() const;
        inline std::string passage() const { return std::string(_start() + passage_offset, reassortant_offset - passage_offset); }
        inline std::string reassortant() const { return std::string(_start() + reassortant_offset, annotation_offset[0] - reassortant_offset); }
        std::vector<std::string> annotations() const;
        inline std::string serum_id() const { return std::string(_start() + serum_id_offset, serum_species_offset - serum_id_offset); }

        inline std::string serum_species() const
            {
                  // ignore padding after serum species
                const auto* start = _start() + serum_species_offset;
                auto* end = _start() + homologous_antigen_index_offset;
                while (end > start && !end[-1])
                    --end;
                return std::string(start, static_cast<size_t>(end - start));
            }

        inline std::pair<size_t, const homologous_t*> homologous_antigens() const
            {
                return {static_cast<size_t>(table_index_offset - homologous_antigen_index_offset) / sizeof(homologous_t),
                        reinterpret_cast<const table_index_t*>(_start() + homologous_antigen_index_offset)};
            }

        inline std::pair<number_of_table_indexes_t, const table_index_t*> tables() const
            {
                return {*reinterpret_cast<const number_of_table_indexes_t*>(_start() + table_index_offset),
                        reinterpret_cast<const table_index_t*>(_start() + table_index_offset + sizeof(number_of_table_indexes_t))};
            }

        inline const char* _start() const { return reinterpret_cast<const char*>(this) + sizeof(*this); }
        inline std::string host() const { return std::string(_start(), location_offset); }
        inline std::string location() const { return std::string(_start() + location_offset, isolation_offset - location_offset); }
        inline std::string isolation() const { return std::string(_start() + isolation_offset, passage_offset - isolation_offset); }
        inline std::string year() const { if (*year_data) return std::string(year_data, sizeof(year_data)); else return std::string{}; }

    }; // struct Serum

      // ----------------------------------------------------------------------

    struct Table
    {
        uint8_t date_offset;
        uint8_t lab_offset;
        uint8_t rbc_offset;
        char lineage;
        uint32_t antigen_index_offset;
        uint32_t serum_index_offset;
        uint32_t titer_offset;

        inline std::string assay() const { return std::string(_start(), date_offset); }
        inline std::string lab() const { return std::string(_start() + lab_offset, rbc_offset - lab_offset); }
        inline std::string date() const { return std::string(_start() + date_offset, lab_offset - date_offset); }

        inline std::string rbc() const
            {
                  // ignore padding after rbc species
                const auto* start = _start() + rbc_offset;
                auto* end = _start() + antigen_index_offset;
                while (end > start && !end[-1])
                    --end;
                return std::string(start, static_cast<size_t>(end - start));
            }

        inline size_t number_of_antigens() const { return static_cast<size_t>(serum_index_offset - antigen_index_offset) / sizeof(antigen_index_t); }
        inline size_t number_of_sera() const { return static_cast<size_t>(titer_offset - serum_index_offset) / sizeof(serum_index_t); }

        inline const char* _start() const { return reinterpret_cast<const char*>(this) + sizeof(*this); }

    }; // struct Table

} // namespace hidb::bin

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
