#include "swimps-dwarf.h"

#include "swimps-dwarf-debug-raii.h"

using swimps::dwarf::DwarfInfo;
using swimps::io::File;

DwarfInfo::DwarfInfo(File&& executableFile) {
    DwarfDebugRAII dwarfDebugRAII(executableFile);

    if (! dwarfDebugRAII.get_initialised()) {
        return;
    }

    Dwarf_Debug& dwarfDebug = dwarfDebugRAII.get_dwarf_debug();
    Dwarf_Error dwarfError{};

    Dwarf_Unsigned cuLength;
    Dwarf_Half cuVersion;
    Dwarf_Off cuAbbrevOffset;
    Dwarf_Half cuPointerSize;
    Dwarf_Half cuOffsetSize;
    Dwarf_Half cuExtensionSize;
    Dwarf_Sig8 typeSignature;
    Dwarf_Unsigned typeOffset;
    Dwarf_Unsigned cuNextOffset;

    int nextCUHeaderReturnValue = DW_DLV_OK;
    do {
        nextCUHeaderReturnValue = dwarf_next_cu_header_c(
            dwarfDebug,
            1 /* 1 == search the .debug_info section, 0 == search the .debug_types section */,
            &cuLength,
            &cuVersion,
            &cuAbbrevOffset,
            &cuPointerSize,
            &cuOffsetSize,
            &cuExtensionSize,
            &typeSignature,
            &typeOffset,
            &cuNextOffset,
            &dwarfError
        );

        Dwarf_Die currentDwarfDie = NULL;
        Dwarf_Die nextDwarfDie = NULL;

        int siblingOfReturnValue = DW_DLV_OK;
        do {
            siblingOfReturnValue =
                dwarf_siblingof(dwarfDebug, currentDwarfDie, &nextDwarfDie, &dwarfError);

            currentDwarfDie = nextDwarfDie;
            Dwarf_Line* dwarfLines;
            Dwarf_Signed dwarfNumberOfLines;

            const auto srcLinesReturnValue = dwarf_srclines(
                currentDwarfDie,
                &dwarfLines,
                &dwarfNumberOfLines,
                &dwarfError
            );

            if (srcLinesReturnValue == DW_DLV_OK) {
                for (Dwarf_Signed i = 0; i < dwarfNumberOfLines; ++i) {
                    m_lineInfos.emplace_back(dwarfLines[i]);
                }
            }
        } while(siblingOfReturnValue == DW_DLV_OK);
    } while (nextCUHeaderReturnValue == DW_DLV_OK);
}

DwarfInfo::DwarfLineInfo::DwarfLineInfo(Dwarf_Line& dwarfLine) {
    Dwarf_Error dwarfError;

    {
        char* sourceFilePath = nullptr;
        if (dwarf_linesrc(dwarfLine, &sourceFilePath, &dwarfError) == DW_DLV_OK) {
            m_sourceFilePath = sourceFilePath;
            // The documentation says not to free the char*
        }
    }

    {
        Dwarf_Unsigned lineNumber = 0;
        if (dwarf_lineno(dwarfLine, &lineNumber, &dwarfError) == DW_DLV_OK) {
            m_lineNumber = lineNumber;
        }
    }

    {   
        Dwarf_Addr address = 0;
        if (dwarf_lineaddr(dwarfLine, &address, &dwarfError) == DW_DLV_OK) {
            m_address = address;
        }
    }

    {
        Dwarf_Signed offset;
        if (dwarf_lineoff(dwarfLine, &offset, &dwarfError) == DW_DLV_OK) {
            m_offset = offset;
        }
    }
}

std::optional<DwarfInfo::DwarfLineInfo::line_number_t> DwarfInfo::DwarfLineInfo::getLineNumber() const { return m_lineNumber; }
std::optional<DwarfInfo::DwarfLineInfo::address_t> DwarfInfo::DwarfLineInfo::getAddress() const { return m_address; }
std::optional<DwarfInfo::DwarfLineInfo::offset_t> DwarfInfo::DwarfLineInfo::getOffset() const { return m_offset; }
std::optional<std::filesystem::path> DwarfInfo::DwarfLineInfo::getSourceFilePath() const { return m_sourceFilePath; }

const std::vector<DwarfInfo::DwarfLineInfo>& DwarfInfo::getLineInfos() const { return m_lineInfos; }
