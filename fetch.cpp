#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

auto getUptimeAsString() -> std::string {
  std::ifstream uptimeFile("/proc/uptime");
  if (!uptimeFile.is_open()) {
    return "Error: Unable to open /proc/uptime file.";
  }

  double uptime{0};
  uptimeFile >> uptime;

  uptimeFile.close();

  int days = static_cast<int>(uptime / (3600 * 24));
  int hours = static_cast<int>((fmod(uptime, (3600 * 24))) / 3600);
  int mins = static_cast<int>((fmod(uptime, 3600)) / 60);
  int secs = static_cast<int>(fmod(uptime, 60));

  std::stringstream uptimeString;

  if (days > 0) {
    uptimeString << days << " day" << (days > 1 ? "s " : " ");
  }
  if (hours > 0) {
    uptimeString << hours << " hour" << (hours > 1 ? "s " : " ");
  }
  if (mins > 0) {
    uptimeString << mins << " min" << (mins > 1 ? "s" : "");
  }
  if (days == 0 && hours == 0 && mins == 0) {
    uptimeString << secs << " sec" << (secs > 1 ? "s" : "");
  }

  return uptimeString.str();
}

auto padString(const std::string &str, size_t length) -> std::string {
  if (str.length() >= length) {
    return str;
  }

  std::string paddedStr = str;
  paddedStr.resize(length, ' ');
  return paddedStr;
}

auto getDistroName() -> std::string {
  std::ifstream file("/etc/os-release");
  std::string line;
  std::string distro;

  if (file.is_open()) {
    while (getline(file, line)) {
      if (line.find("PRETTY_NAME=") != std::string::npos) {
        distro = line.substr(line.find('=') + 1);
        // Remove quotes from the extracted string
        distro.erase(0, 1); // Remove the opening quote
        distro.pop_back();  // Remove the closing quote
        break;
      }
    }
    file.close();
  } else {
    std::cerr << "Error: Unable to open /etc/os-release file." << '\n';
  }
  return distro;
}

auto getKernelName() -> std::string {
  std::ifstream file("/proc/version");
  std::string line;
  std::string kernelName;

  if (file.is_open()) {
    std::getline(file, line);
    size_t startPos = line.find("version ");
    size_t endPos = line.find(" (");
    if (startPos != std::string::npos && endPos != std::string::npos &&
        startPos < endPos) {
      kernelName = line.substr(startPos + 8, endPos - startPos - 8);
    }
    file.close();
  } else {
    std::cerr << "Error: Unable to open /proc/version file." << '\n';
  }

  return kernelName;
}

auto getUsername() -> std::string {
  char *username = getlogin();
  if (username != nullptr) {
    return {username};
  }
  std::cerr << "Error getting username: " << strerror(errno) << '\n';
  return "";
}

auto getHostname() -> std::string {
  std::array<char, 256> hostname{};
  if (gethostname(hostname.data(), hostname.size()) == 0) {
    return {hostname.data()};
  }
  std::cerr << "Error getting hostname: " << strerror(errno) << '\n';
  return "";
}

struct FFMemoryResult {
  uint64_t bytesTotal;
  uint64_t bytesUsed;
};

auto getRAMUsage() -> std::string {
  std::ifstream meminfoFile("/proc/meminfo");
  if (!meminfoFile.is_open()) {
    return "Error: Unable to open /proc/meminfo file.";
  }

  std::string line;
  uint64_t memTotal = 0;
  uint64_t memAvailable = 0;
  uint64_t shmem = 0;
  uint64_t memFree = 0;
  uint64_t buffers = 0;
  uint64_t cached = 0;
  uint64_t sReclaimable = 0;

  while (std::getline(meminfoFile, line)) {
    std::istringstream iss(line);
    std::string token;
    iss >> token;
    if (token == "MemTotal:") {
      iss >> memTotal;
    } else if (token == "MemAvailable:") {
      iss >> memAvailable;
    } else if (token == "MemFree:") {
      iss >> memFree;
    } else if (token == "Buffers:") {
      iss >> buffers;
    } else if (token == "Cached:") {
      iss >> cached;
    } else if (token == "Shmem:") {
      iss >> shmem;
    } else if (token == "SReclaimable:") {
      iss >> sReclaimable;
    }
  }

  meminfoFile.close();

  if (memAvailable == 0) {
    memAvailable = memFree + buffers + cached + sReclaimable - shmem;
  }

  FFMemoryResult ram{};
  ram.bytesTotal = memTotal * 1024;
  ram.bytesUsed = (memTotal - memAvailable) * 1024;

  // Convert bytes to MiB

  double totalMiB = static_cast<double>(ram.bytesTotal) / (1024.0 * 1024);
  double usedMiB = static_cast<double>(ram.bytesUsed) / (1024.0 * 1024);

  std::stringstream ramUsage;
  ramUsage << std::fixed << std::setprecision(0) << usedMiB << " MiB / "
           << totalMiB << " MiB";

  return ramUsage.str();
}

auto getPacmanPackageCount() -> std::string {
  try {
    constexpr const char *pacmanLocalDir = "/var/lib/pacman/local/";
    auto packageCount = static_cast<std::size_t>(
        std::count_if(std::filesystem::directory_iterator(pacmanLocalDir),
                      std::filesystem::directory_iterator(),
                      [](const auto &entry) { return entry.is_directory(); }));
    return std::to_string(packageCount);
  } catch (const std::filesystem::filesystem_error &e) {
    return "Error: " + std::string(e.what());
  }
}

auto getEnvVar(const char *var) -> std::string {
  const char *value = std::getenv(var);
  return value != nullptr ? std::string(value) : "";
}

auto getTerminalEmulatorName() -> std::string {
  std::string termProgram = getEnvVar("TERM_PROGRAM");
  if (!termProgram.empty()) {
    if (termProgram == "Apple_Terminal") {
      return "Apple Terminal";
    }
    if (termProgram == "iTerm.app") {
      return "iTerm2";
    }
    return termProgram;
  }

  std::string term = getEnvVar("TERM");
  if (!term.empty()) {
    if (term.find("rxvt-unicode") != std::string::npos) {
      return "urxvt";
    }
    if (term.find("linux") != std::string::npos) {
      return "Linux Console";
    }
    if (term.find("screen") != std::string::npos) {
      return "Screen";
    }
    if (term.find("tmux") != std::string::npos) {
      return "tmux";
    }
    if (term.find("st-256color") != std::string::npos) {
      return "st";
    }
    if (term.find("alacritty") != std::string::npos) {
      return "Alacritty";
    }
    if (term.find("foot") != std::string::npos) {
      return "foot";
    }
    if (term.find("xterm-kitty") != std::string::npos) {
      return "kitty";
    }
    if (term.find("wezterm") != std::string::npos) {
      return "WezTerm";
    }
    if (term.find("gnome") != std::string::npos) {
      return "Gnome Terminal";
    }
    if (term.find("konsole") != std::string::npos) {
      return "Konsole";
    }
    if (term.find("hyper") != std::string::npos) {
      return "Hyper";
    }
    if (term.find("xterm-256color") != std::string::npos) {
      return "xterm";
    }
  }

  return "Unknown";
}

auto getWindowManagerName() -> std::string {
  Display *display = XOpenDisplay(nullptr);
  if (display == nullptr) {
    std::cerr << "Cannot open display" << '\n';
    return "";
  }

  Window root = DefaultRootWindow(display);
  Atom atom = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", True);
  Atom actualType = 0;
  int actualFormat = 0;
  uint64_t nitems = 0;
  uint64_t bytesAfter = 0;
  unsigned char *prop = nullptr;

  if (XGetWindowProperty(display, root, atom, 0, (~0L), False, AnyPropertyType,
                         &actualType, &actualFormat, &nitems, &bytesAfter,
                         &prop) != Success) {
    std::cerr << "Cannot get window manager property" << '\n';
    if (prop != nullptr) {
      XFree(prop);
    }
    XCloseDisplay(display);
    return "";
  }

  if (nitems > 0) {
    Window wmWindow = *(reinterpret_cast<Window *>(prop));
    XFree(prop);

    atom = XInternAtom(display, "_NET_WM_NAME", True);
    if (XGetWindowProperty(display, wmWindow, atom, 0, (~0L), False,
                           AnyPropertyType, &actualType, &actualFormat, &nitems,
                           &bytesAfter, &prop) == Success &&
        nitems > 0) {
      std::string wmName(reinterpret_cast<char *>(prop));
      XFree(prop);
      XCloseDisplay(display);
      return wmName;
    }
  }

  if (prop != nullptr) {
    XFree(prop);
  }
  XCloseDisplay(display);
  return "";
}

auto main() -> int {
  std::string color1{"\x1B[31m"};
  std::string color2{"\x1B[32m"};
  std::string color3{"\x1B[33m"};
  std::string color4{"\x1B[34m"};
  std::string color5{"\x1B[35m"};
  std::string color6{"\x1B[36m"};
  std::string color7{"\x1B[37m"};
  std::string color8{"\x1B[90m"};
  std::string colReset{"\x1B[0m"};

  std::string pacman{" 󰮯   "};
  std::string ghost{"󰊠   "};

  std::string colorsInfo = color3 + pacman + color2 + ghost + color1 + ghost +
                           color4 + ghost + color5 + ghost + color6 + ghost +
                           color7 + ghost + color8 + ghost;
  const int infoLength = 27;

  // clang-format off
  std::string title{padString(getUsername() + '@' + getHostname(), infoLength)};
  std::string distro{padString(getDistroName(), infoLength)};
  std::string kernel{padString(getKernelName(), infoLength)};
  std::string wmname{padString(getWindowManagerName(), infoLength)};
  std::string uptime{padString(getUptimeAsString(), infoLength)};
  std::string memory{padString(getRAMUsage(), infoLength)};
  std::string term{padString(getTerminalEmulatorName(), infoLength)};
  std::string pkgs{padString(getPacmanPackageCount() + " (pacman)", infoLength)};

  std::cout << color8 + "    ⠀⠀⠀⠀⠀⠀⢀⢀⠀⣄⢂⣀⣂⣤⣀⢀⢀⠀⠀⠀⠀⠀⠀   ╭─────┬─────────────────────────────╮\n";
  std::cout << color8 + "    ⠀⠀⠀⠀⣄⣰⣝⣉⣙⢱⣞⠜⡖⣌⣌⣁⣻⣠⣀⠀⠀⠀⠀   │  "  + color4 + "" + color8  + "  │  " + color2 + title  + color8 + "│\n";
  std::cout << color8 + "  ⠀ ⠀⠀⢔⢽⣊⣂⣇⡒⠮⢻⣇⠲⢍⣿⢟⣒⣰⣠⣑⡔⣀⠀⠀  ⠀│  "  + color4 + "" + color8  + "  │  " + colReset + distro + color8 + "│\n";
  std::cout << color8 + "  ⠀ ⠀⠹⡸⢙⣕⡐⢂⠍⣩⡻⣿⣆⣾⠟⣉⢣⠪⠲⡰⡋⣢⢆⠀  ⠀│  "  + color4 + "" + color8  + "  │  " + colReset + kernel + color8 + "│\n";
  std::cout << color8 + "  ⠀ ⠀⣪⡹⣵⢦⠽⡓⠶⠶⣧⣹⣿⣏⣴⠶⠛⠟⠽⢛⣒⢮⡙⠀  ⠀│  "  + color4 + "󰙀" + color8  + "  │  " + colReset + wmname + color8 + "│\n";
  std::cout << color8 + "  ⠀ ⠸⢋⢪⠑⢩⠈⠀⠀⠀⠈⣿⣿⣿⠁⠀⠀⠘⢠⠈⠆⢎⢱⠁  ⠀│  "  + color4 + "" + color8  + "  │  " + colReset + memory + color8 + "│\n";
  std::cout << color8 + "⠀⠀⠀ ⠀⠻⡕⡲⡈⠀⠀⠀⠀⠀⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠆⠄⠁  ⠀│  "  + color4 + "" + color8  + "  │  " + colReset + term   + color8 + "│\n";
  std::cout << color8 + "    ⠀⠀⠀⠇⠀⠀⠀⠀⠀⣰⣿⣿⣿⣤⡀⠀⠀⠀⠀⠌⠘⠀⠀  ⠀│  "  + color4 + "󰔚" + color8  + "  │  " + colReset + uptime + color8 + "│\n";
  std::cout << color8 + "    ⠀⠀⠀⢱⠒⡶⡶⢖⣫⡿⢻⠿⡟⢿⣍⡱⢖⠶⠒⡆⠀⠀⠀  ⠀│  "  + color4 + "" + color8  + "  │  " + colReset + pkgs   + color8 + "│\n";
  std::cout << color8 + "  ⠀ ⠀⠀⠀⠀⠁⠬⡃⠴⣇⣴⠏⠖⠙⣦⣸⠦⡼⠣⠉⠀⠀⠀⠀  ⠀│─────┴─────────────────────────────┤\n";
  std::cout << color8 + "    ⠀⠀⠀⠀⠀⠀⠈⠁⠑⠢⠧⠭⠼⠵⠋⠊⠁⠀⠀⠀⠀⠀⠀  ⠀│ "        +  colorsInfo  +       " │\n";
  std::cout << color8 + "     ⠀⠀⠀⠀⠀⠀             ⠀⠀⠀⠀  ╰───────────────────────────────────╯\n";
  // clang-format on

  return 0;
}
