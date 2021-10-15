#pragma once

#include <libdwarf/libdwarf.h>

namespace signalsafe {
    class File;
}

namespace swimps::dwarf {
    class DwarfDebugRAII final {
    public:
        DwarfDebugRAII(signalsafe::File& executableFile);
        ~DwarfDebugRAII();

        Dwarf_Debug& get_dwarf_debug() noexcept;
        bool get_initialised() const noexcept;

    private:
        bool m_initialised = false;
        Dwarf_Debug m_dwarfDebug{};
    };
}
