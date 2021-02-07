#include "swimps-dwarf.h"

#include <functional>

#include <libdwarf/dwarf.h>

#include "swimps-dwarf-debug-raii.h"

using swimps::dwarf::DwarfInfo;
using swimps::io::File;

DwarfInfo::DwarfInfo(File&& executableFile) {
    DwarfDebugRAII dwarfDebugRAII(executableFile);

    if (! dwarfDebugRAII.get_initialised()) {
        return;
    }

    const auto getFunctionInfo = [](Dwarf_Die currentDwarfDie, std::vector<DwarfInfo::DwarfFunctionInfo>& functionInfos){
        Dwarf_Error dwarfError{};
        Dwarf_Half dwarfTag{};
        if (dwarf_tag(currentDwarfDie, &dwarfTag, &dwarfError) == DW_DLV_OK) {
            Dwarf_Attribute lowPCAttribute{};
            Dwarf_Attribute highPCAttribute{};
            Dwarf_Attribute nameAttribute{};
            if (dwarfTag == DW_TAG_subprogram) {
                if (dwarf_attr(currentDwarfDie, DW_AT_low_pc, &lowPCAttribute, &dwarfError) == DW_DLV_OK
                && dwarf_attr(currentDwarfDie, DW_AT_high_pc, &highPCAttribute, &dwarfError) == DW_DLV_OK
                && dwarf_attr(currentDwarfDie, DW_AT_name, &nameAttribute, &dwarfError) == DW_DLV_OK) {

                    Dwarf_Addr lowPC{};

                    // In DWARF 2, the format for the high PC is apparently different.
                    // TODO: add support for DWARF 2.
                    Dwarf_Unsigned highPCOffset{};

                    // Documentation says not to free the memory for the string
                    char* name = nullptr;

                    if (dwarf_formaddr(lowPCAttribute, &lowPC, &dwarfError) == DW_DLV_OK 
                        && dwarf_formudata(highPCAttribute, &highPCOffset, &dwarfError) == DW_DLV_OK
                        && dwarf_formstring(nameAttribute, &name, &dwarfError) == DW_DLV_OK) {
                        functionInfos.emplace_back(lowPC, lowPC + highPCOffset, name);
                    }
                }
            }
        }
    };

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

    for (;;) {
        auto nextCUHeaderReturnValue = dwarf_next_cu_header_c(
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

        if (nextCUHeaderReturnValue == DW_DLV_ERROR) {
            break;
        }

        Dwarf_Die currentDwarfDie = NULL;
        Dwarf_Die nextDwarfDie = NULL;
        for(;;) {
            const auto siblingOfReturnValue = dwarf_siblingof(
                dwarfDebug,
                currentDwarfDie,
                &nextDwarfDie,
                &dwarfError
            );

            if (siblingOfReturnValue == DW_DLV_ERROR) {
                break;
            }

            currentDwarfDie = nextDwarfDie;

            Dwarf_Line* dwarfLines = nullptr;
            Dwarf_Signed dwarfNumberOfLines{};
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

            const auto childReturnValue = dwarf_child(
                currentDwarfDie,
                &nextDwarfDie,
                &dwarfError
            );

            currentDwarfDie = nextDwarfDie;

            getFunctionInfo(currentDwarfDie, m_functionInfos);

            if (childReturnValue == DW_DLV_ERROR) {
                break;
            }

            for (;;) {
                const auto subSiblingOfReturnValue = dwarf_siblingof(
                    dwarfDebug,
                    currentDwarfDie,
                    &nextDwarfDie,
                    &dwarfError
                );

                if (subSiblingOfReturnValue == DW_DLV_ERROR) {
                    break;
                }

                currentDwarfDie = nextDwarfDie;

                getFunctionInfo(currentDwarfDie, m_functionInfos);

                if (subSiblingOfReturnValue == DW_DLV_NO_ENTRY) {
                    break;
                }
            }

            if (siblingOfReturnValue == DW_DLV_NO_ENTRY) {
                break;
            }
        }

        if (nextCUHeaderReturnValue == DW_DLV_NO_ENTRY) {
            break;
        }
    }
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

DwarfInfo::DwarfFunctionInfo::DwarfFunctionInfo(
    Dwarf_Addr lowPC,
    Dwarf_Addr highPC,
    std::string name)
: m_lowPC(lowPC),
  m_highPC(highPC),
  m_name(name) {

}

std::optional<DwarfInfo::DwarfLineInfo::line_number_t> DwarfInfo::DwarfLineInfo::getLineNumber() const { return m_lineNumber; }
std::optional<DwarfInfo::address_t> DwarfInfo::DwarfLineInfo::getAddress() const { return m_address; }
std::optional<DwarfInfo::DwarfLineInfo::offset_t> DwarfInfo::DwarfLineInfo::getOffset() const { return m_offset; }
std::optional<std::filesystem::path> DwarfInfo::DwarfLineInfo::getSourceFilePath() const { return m_sourceFilePath; }

const std::vector<DwarfInfo::DwarfLineInfo>& DwarfInfo::getLineInfos() const { return m_lineInfos; }

DwarfInfo::address_t DwarfInfo::DwarfFunctionInfo::getLowPC() const { return m_lowPC; }
DwarfInfo::address_t DwarfInfo::DwarfFunctionInfo::getHighPC() const { return m_highPC; }
std::string DwarfInfo::DwarfFunctionInfo::getName() const { return m_name; }

const std::vector<DwarfInfo::DwarfFunctionInfo>& DwarfInfo::getFunctionInfos() const { return m_functionInfos; }
