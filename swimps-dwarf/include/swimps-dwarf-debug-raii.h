#pragma once

#include <libdwarf/libdwarf.h>

#include "swimps-io-file.h"

namespace swimps::dwarf {
    class DwarfDebugRAII final {
    public:
        DwarfDebugRAII(swimps::io::File& executableFile);
        ~DwarfDebugRAII();

        Dwarf_Debug& get_dwarf_debug() noexcept;
        bool get_initialised() const noexcept;

    private:
        bool m_initialised = false;
        Dwarf_Debug m_dwarfDebug{};
    };
}