#include "swimps-dwarf-debug-raii.h"

#include  <libelf.h>

using swimps::dwarf::DwarfDebugRAII;
using swimps::io::File;

DwarfDebugRAII::DwarfDebugRAII(File& executableFile) {
    Dwarf_Error dwarfError;

    m_initialised = DW_DLV_OK == dwarf_init(
        executableFile.getFileDescriptor(),
        DW_DLC_READ,
        [](Dwarf_Error, Dwarf_Ptr){ swimps_assert(false); },
        nullptr,
        &m_dwarfDebug,
        &dwarfError
    );
}

DwarfDebugRAII::~DwarfDebugRAII() {
    Dwarf_Error dwarfError{};
    Elf* elf = nullptr;
    dwarf_get_elf(m_dwarfDebug, &elf, &dwarfError);
    dwarf_finish(m_dwarfDebug, &dwarfError);
    elf_end(elf);
}

Dwarf_Debug& DwarfDebugRAII::get_dwarf_debug() noexcept {
    swimps_assert(get_initialised());
    return m_dwarfDebug;
}

bool DwarfDebugRAII::get_initialised() const noexcept {
    return m_initialised;
}
