#include "swimps-dwarf.h"

#include  <libelf.h>

using swimps::dwarf::DwarfInfo;
using swimps::io::File;

DwarfInfo::DwarfInfo(File&& executableFile)
: m_executableFile(std::move(executableFile)) {
    Dwarf_Error dwarfError{};

    const auto initReturnValue = dwarf_init(
        m_executableFile.getFileDescriptor(),
        DW_DLC_READ,
        [](Dwarf_Error, Dwarf_Ptr){ swimps_assert(false); },
        nullptr,
        &m_dwarfDebug,
        &dwarfError
    );

    if (initReturnValue != DW_DLV_OK) {
        return;
    }

    Dwarf_Die currentDwarfDie = NULL;
    Dwarf_Die nextDwarfDie = NULL;

    Dwarf_Unsigned cuLength;
    Dwarf_Half cuVersion;
    Dwarf_Off cuAbbrevOffset;
    Dwarf_Half cuPointerSize;
    Dwarf_Half cuOffsetSize;
    Dwarf_Half cuExtensionSize;
    Dwarf_Sig8 typeSignature;
    Dwarf_Unsigned typeOffset;
    Dwarf_Unsigned cuNextOffset;

    // TODO: make this work for multiple CUs

    const auto nextCUHeaderReturnValue = dwarf_next_cu_header_c(
        m_dwarfDebug,
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

    if (nextCUHeaderReturnValue != DW_DLV_OK) {
        return;
    }

    // TODO: do we want to check each sibling?

    while (dwarf_siblingof(m_dwarfDebug, currentDwarfDie, &nextDwarfDie, &dwarfError) == DW_DLV_OK) {
        currentDwarfDie = nextDwarfDie;
    }

    Dwarf_Line* dwarfLines;
    Dwarf_Signed dwarfNumberOfLines;

    const auto srcLinesReturnValue = dwarf_srclines(
        currentDwarfDie,
        &dwarfLines,
        &dwarfNumberOfLines,
        &dwarfError
    );

    for (Dwarf_Signed i = 0; i < dwarfNumberOfLines; ++i) {
        m_lineInfos.emplace_back(dwarfLines[i]);
    }

    if (srcLinesReturnValue != DW_DLV_OK) {
        return;
    }
}

DwarfInfo::~DwarfInfo() {
    Dwarf_Error dwarfError;
    Elf* elf = nullptr;
    dwarf_get_elf(m_dwarfDebug, &elf, &dwarfError);
    dwarf_finish(m_dwarfDebug, &dwarfError);
    elf_end(elf);
}

DwarfInfo::DwarfLineInfo::DwarfLineInfo(Dwarf_Line& dwarfLine) {
    Dwarf_Error dwarfError;

    {
        char* fileName = nullptr;
        if (dwarf_linesrc(dwarfLine, &fileName, &dwarfError) == DW_DLV_OK) {
            m_sourceFile = fileName;
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
std::optional<std::string> DwarfInfo::DwarfLineInfo::getSourceFile() const { return m_sourceFile; }

const std::vector<DwarfInfo::DwarfLineInfo>& DwarfInfo::getLineInfos() const { return m_lineInfos; }
