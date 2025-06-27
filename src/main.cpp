#include <string>
#include <vector>

#include "ascii.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "distro.hpp"
#include "formatter.hpp"
#include "kernel.hpp"
#include "pkgcount.hpp"
#include "printer.hpp"
#include "ram.hpp"
#include "terminal.hpp"
#include "uptime.hpp"
#include "userhost.hpp"
#include "wm.hpp"

auto main() -> int {
  auto ascii_art = std::string(kAsciiArtChoice).empty()
                       ? GetAsciiArt()
                       : GetAsciiArt(kAsciiArtChoice);
  // clang-format off
  std::vector<std::string> info = {
    kCol8 + "╭─────┬─────────────────────────────╮" + kColRes,
    kCol8 + "│  " +kCol4+ kIconUser   +kCol8+ "  │  " + kCol2   + Pad(GetUsername() + "@" + GetHostname(), kInfoPad) + kCol8 + "│" + kColRes,
    kCol8 + "│  " +kCol4+ kIconDistro +kCol8+ "  │  " + kColRes + Pad(GetDistroName(), kInfoPad)                     + kCol8 + "│" + kColRes,
    kCol8 + "│  " +kCol4+ kIconKernel +kCol8+ "  │  " + kColRes + Pad(GetKernelName(), kInfoPad)                     + kCol8 + "│" + kColRes,
    kCol8 + "│  " +kCol4+ kIconWM     +kCol8+ "  │  " + kColRes + Pad(GetWindowManagerName(), kInfoPad)              + kCol8 + "│" + kColRes,
    kCol8 + "│  " +kCol4+ kIconRAM    +kCol8+ "  │  " + kColRes + Pad(GetRamUsage(), kInfoPad)                       + kCol8 + "│" + kColRes,
    kCol8 + "│  " +kCol4+ kIconTerm   +kCol8+ "  │  " + kColRes + Pad(GetTerminalEmulatorName(), kInfoPad)           + kCol8 + "│" + kColRes,
    kCol8 + "│  " +kCol4+ kIconUptime +kCol8+ "  │  " + kColRes + Pad(GetUptimeAsString(), kInfoPad)                 + kCol8 + "│" + kColRes,
    kCol8 + "│  " +kCol4+ kIconPkgs   +kCol8+ "  │  " + kColRes + Pad(GetPacmanPackageCount(), kInfoPad)             + kCol8 + "│" + kColRes,
    kCol8 + "├─────┴─────────────────────────────┤" + kColRes,
    kCol8 + "│   "    +  kColorsInfo  + kCol8 + "│" + kColRes,
    kCol8 + "╰───────────────────────────────────╯" + kColRes
  };
  // clang-format on

  PrintSideBySide(ascii_art, info);

  return 0;
}
