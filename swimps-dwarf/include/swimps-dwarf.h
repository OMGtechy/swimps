#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include <libdwarf/libdwarf.h>

#include "swimps-io-file.h"

namespace swimps::dwarf {
    //!
    //! \brief  Encapculates DWARF debug information access.
    //!
    class DwarfInfo final {
    public:
        //!
        //! \brief  Creates an instance.
        //!
        //! \param[in]  executableFile  The executable to extract debug information of.
        //!
        DwarfInfo(swimps::io::File&& executableFile);

        class DwarfLineInfo final {
        public:
            DwarfLineInfo(Dwarf_Line&);

            using line_number_t = int64_t;
            using address_t = uint64_t;
            using offset_t = int64_t;

            std::optional<line_number_t> getLineNumber() const;
            std::optional<address_t> getAddress() const;
            std::optional<offset_t> getOffset() const;
            std::optional<std::filesystem::path> getSourceFilePath() const;

        private:
            std::optional<line_number_t> m_lineNumber;
            std::optional<address_t> m_address;
            std::optional<offset_t> m_offset;
            std::optional<std::filesystem::path> m_sourceFilePath;
        };

        const std::vector<DwarfLineInfo>& getLineInfos() const;

    private:
        std::vector<DwarfLineInfo> m_lineInfos;
    };
}

