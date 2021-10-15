#include "swimps-dwarf/swimps-dwarf-debug-raii.h"

#include  <libelf.h>

#include <signalsafe/file.hpp>

#include "swimps-assert/swimps-assert.h"

using signalsafe::File;

using swimps::dwarf::DwarfDebugRAII;

DwarfDebugRAII::DwarfDebugRAII(File& executableFile) {
    Dwarf_Error dwarfError;

    m_initialised = DW_DLV_OK == dwarf_init(
        executableFile.get_file_descriptor(),
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
