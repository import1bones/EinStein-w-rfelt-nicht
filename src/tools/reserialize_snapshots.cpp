// Small utility to reserialize existing snapshot files using nlohmann::json
// This tool is only compiled when nlohmann::json is available (USE_NLOHMANN_JSON).

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

#ifdef USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

int main(int argc, char** argv) {
    namespace fs = std::filesystem;
    fs::path snapshots_dir = "snapshots";
    if (argc > 1) snapshots_dir = argv[1];

    if (!fs::exists(snapshots_dir) || !fs::is_directory(snapshots_dir)) {
        std::cerr << "Snapshots directory not found: " << snapshots_dir << "\n";
        return 2;
    }

#ifndef USE_NLOHMANN_JSON
    std::cerr << "nlohmann::json support is not enabled.\n"
              << "Please vendor the single-header into include/third_party/json.hpp and re-run CMake with USE_NLOHMANN_JSON=ON." << std::endl;
    return 2;
#else
    fs::path out_dir = snapshots_dir / "reserialized";
    fs::create_directories(out_dir);

    int total = 0, success = 0, skipped = 0;
    for (auto &entry : fs::directory_iterator(snapshots_dir)) {
        if (!entry.is_regular_file()) continue;
        auto path = entry.path();
        if (path.extension() != ".snapshot") continue;
        ++total;

        std::ifstream in(path);
        if (!in) { std::cerr << "Failed to open " << path << "\n"; ++skipped; continue; }
        std::ostringstream ss; ss << in.rdbuf();
        std::string content = ss.str();

        try {
            auto j = nlohmann::json::parse(content);
            std::string out = j.dump(2);
            fs::path dest = out_dir / path.filename();
            std::ofstream o(dest);
            if (!o) { std::cerr << "Failed to write " << dest << "\n"; ++skipped; continue; }
            o << out;
            ++success;
            std::cout << "Reserialized " << path.filename() << " -> " << dest << "\n";
        } catch (const std::exception &e) {
            std::cerr << "Failed to parse " << path.filename() << ": " << e.what() << "\n";
            ++skipped;
        }
    }

    std::cout << "Done. Processed=" << total << " succeeded=" << success << " skipped=" << skipped << "\n";
    return 0;
#endif
}
