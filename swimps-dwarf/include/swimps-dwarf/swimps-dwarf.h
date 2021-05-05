#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include <libdwarf/libdwarf.h>

#include "swimps-io/swimps-io-file.h"

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

        using address_t = uint64_t;

        class DwarfLineInfo final {
        public:
            DwarfLineInfo(Dwarf_Line&);

            using line_number_t = int64_t;
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

        class DwarfFunctionInfo final {
        public:
            DwarfFunctionInfo(Dwarf_Addr lowPC, Dwarf_Addr highPC, std::string name);

            address_t getLowPC() const;
            address_t getHighPC() const;
            std::string getName() const;

        private:
            address_t m_lowPC;
            address_t m_highPC;
            std::string m_name;
        };

        const std::vector<DwarfLineInfo>& getLineInfos() const;
        const std::vector<DwarfFunctionInfo>& getFunctionInfos() const;

    private:
        std::vector<DwarfLineInfo> m_lineInfos;
        std::vector<DwarfFunctionInfo> m_functionInfos;
    };
}

