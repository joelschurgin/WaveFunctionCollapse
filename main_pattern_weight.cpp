#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "stb_image.h"
#include "stb_image_write.h"


// Pattern size (NxN)
const int N = 3;

struct Pattern {
    std::vector<unsigned char> data;
    int frequency;

    bool operator==(const Pattern &other) const { return data == other.data; }

    bool operator<(const Pattern &other) const { return data < other.data; }
};

struct PatternHash {
    size_t operator()(const Pattern &p) const {
        size_t hash = 0;
        for (unsigned char c : p.data) {
            hash = hash * 31 + c;
        }
        return hash;
    }
};

class WaveFunctionCollapse {
   private:
    int width, height, channels;
    std::vector<std::vector<std::set<Pattern>>> wave;
    std::vector<Pattern> patterns;
    std::unordered_map<Pattern, int, PatternHash> pattern_frequencies;
    std::mt19937 rng;

    // Cache for pattern compatibility
    struct CompatibilityCache {
        std::vector<std::vector<std::vector<bool>>> cache;
        void initialize(size_t pattern_count) {
            cache.resize(pattern_count,
                         std::vector<std::vector<bool>>(
                             pattern_count, std::vector<bool>(8, false)));
        }
    } compatibility_cache;

    // Pattern index mapping
    std::unordered_map<Pattern, size_t, PatternHash> pattern_to_index;

    // Entropy cache
    struct EntropyInfo {
        double entropy;
        double sum_weights;
        std::vector<double> weights;
    };
    std::vector<std::vector<EntropyInfo>> entropy_cache;

    struct WaveState {
        std::vector<std::vector<std::set<Pattern>>> wave;
        std::vector<std::vector<EntropyInfo>> entropy_cache;
        int x, y;
        Pattern chosen_pattern;
    };
    std::vector<WaveState> state_history;

    void extractPatterns(const unsigned char *input, int input_width,
                         int input_height) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int y = 0; y < input_height - N + 1; y++) {
            for (int x = 0; x < input_width - N + 1; x++) {
                Pattern pattern;
                pattern.data.resize(N * N * channels);

                for (int dy = 0; dy < N; dy++) {
                    for (int dx = 0; dx < N; dx++) {
                        for (int c = 0; c < channels; c++) {
                            pattern.data[(dy * N + dx) * channels + c] =
                                input[((y + dy) * input_width + (x + dx)) *
                                          channels +
                                      c];
                        }
                    }
                }

                pattern_frequencies[pattern]++;
            }
        }

        // Create pattern index mapping
        size_t index = 0;
        for (const auto &[pattern, freq] : pattern_frequencies) {
            patterns.push_back(pattern);
            patterns.back().frequency = freq;
            pattern_to_index[pattern] = index++;
        }

        // Initialize compatibility cache
        compatibility_cache.initialize(patterns.size());

        // Pre-compute pattern compatibilities
        for (size_t i = 0; i < patterns.size(); i++) {
            for (size_t j = 0; j < patterns.size(); j++) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int dir = (dy + 1) * 3 + (dx + 1);
                        if (dir > 4) dir--;  // Skip center
                        compatibility_cache.cache[i][j][dir] =
                            isCompatible(patterns[i], patterns[j], dx, dy);
                    }
                }
            }
        }
    }

    bool isCompatible(const Pattern &p1, const Pattern &p2, int dx, int dy) {
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                if (x + dx >= 0 && x + dx < N && y + dy >= 0 && y + dy < N) {
                    for (int c = 0; c < channels; c++) {
                        if (p1.data[(y * N + x) * channels + c] !=
                            p2.data[((y + dy) * N + (x + dx)) * channels + c]) {
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }

    void updateEntropyCache(int x, int y) {
        auto &cell = wave[y][x];
        auto &cache = entropy_cache[y][x];

        cache.sum_weights = 0;
        cache.weights.clear();
        cache.weights.reserve(cell.size());

        for (const Pattern &p : cell) {
            double weight = p.frequency;
            cache.weights.push_back(weight);
            cache.sum_weights += weight;
        }

        // Calculate entropy using the sum of weights
        cache.entropy = -std::log(cache.sum_weights);
    }

    void propagate(int x, int y) {
        auto start = std::chrono::high_resolution_clock::now();
        std::queue<std::pair<int, int>> to_propagate;
        to_propagate.push({x, y});

        // Keep track of which cells we've already queued to avoid duplicates
        std::vector<std::vector<bool>> queued(height,
                                              std::vector<bool>(width, false));
        queued[y][x] = true;

        int iterations = 0;
        const int MAX_ITERATIONS = 1000;  // Prevent infinite loops

        while (!to_propagate.empty() && iterations < MAX_ITERATIONS) {
            auto [px, py] = to_propagate.front();
            to_propagate.pop();
            queued[py][px] = false;

            // Get the current cell's patterns once
            const auto &current_patterns = wave[py][px];
            if (current_patterns.empty()) continue;

            // Pre-compute which patterns are possible in current cell
            std::vector<size_t> current_indices;
            current_indices.reserve(current_patterns.size());
            for (const Pattern &p : current_patterns) {
                current_indices.push_back(pattern_to_index[p]);
            }

            // Check each neighbor
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;

                    int nx = px + dx;
                    int ny = py + dy;
                    if (nx < 0 || nx >= width || ny < 0 || ny >= height)
                        continue;

                    int dir = (dy + 1) * 3 + (dx + 1);
                    if (dir > 4) dir--;  // Skip center

                    // Get neighbor's patterns
                    auto &neighbor_patterns = wave[ny][nx];
                    if (neighbor_patterns.empty()) continue;

                    // Create a temporary set for new possibilities
                    std::set<Pattern> new_possibilities;

                    // Check each pattern in the neighbor cell
                    for (const Pattern &p2 : neighbor_patterns) {
                        size_t idx2 = pattern_to_index[p2];
                        bool is_compatible = false;

                        // Check if any current pattern is compatible with this
                        // neighbor pattern
                        for (size_t idx1 : current_indices) {
                            if (compatibility_cache.cache[idx1][idx2][dir]) {
                                is_compatible = true;
                                break;
                            }
                        }

                        if (is_compatible) {
                            new_possibilities.insert(p2);
                        }
                    }

                    // Only update if we actually removed some possibilities
                    if (new_possibilities.size() < neighbor_patterns.size()) {
                        neighbor_patterns = std::move(new_possibilities);
                        updateEntropyCache(nx, ny);

                        // Only queue if not already queued
                        if (!queued[ny][nx]) {
                            to_propagate.push({nx, ny});
                            queued[ny][nx] = true;
                        }
                    }
                }
            }

            iterations++;
        }
    }

    std::pair<int, int> findLowestEntropy() {
        double min_entropy = std::numeric_limits<double>::max();
        std::vector<std::pair<int, int>> candidates;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (wave[y][x].size() <= 1) continue;

                double entropy = entropy_cache[y][x].entropy;
                if (entropy < min_entropy) {
                    min_entropy = entropy;
                    candidates.clear();
                    candidates.push_back({x, y});
                } else if (std::abs(entropy - min_entropy) < 1e-10) {
                    candidates.push_back({x, y});
                }
            }
        }

        if (candidates.empty()) return {-1, -1};
        return candidates[std::uniform_int_distribution<>(
            0, candidates.size() - 1)(rng)];
    }

    void saveState(int x, int y, const Pattern &chosen) {
        WaveState state;
        state.wave = wave;
        state.entropy_cache = entropy_cache;
        state.x = x;
        state.y = y;
        state.chosen_pattern = chosen;
        state_history.push_back(std::move(state));
    }

    bool restoreState() {
        if (state_history.empty()) return false;

        WaveState &state = state_history.back();
        wave = std::move(state.wave);
        entropy_cache = std::move(state.entropy_cache);

        // Remove the chosen pattern from possibilities at that position
        wave[state.y][state.x].erase(state.chosen_pattern);
        updateEntropyCache(state.x, state.y);

        state_history.pop_back();
        return true;
    }

    bool tryLocalRepair(int x, int y) {
        // Try to repair by allowing more patterns in neighboring cells
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;

                int nx = x + dx;
                int ny = y + dy;
                if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;

                // If neighbor is empty, try to restore some patterns
                if (wave[ny][nx].empty()) {
                    wave[ny][nx] =
                        std::set<Pattern>(patterns.begin(), patterns.end());
                    updateEntropyCache(nx, ny);
                    propagate(nx, ny);

                    // If still empty after repair attempt, restore state and
                    // try next neighbor
                    if (wave[ny][nx].empty()) {
                        if (!restoreState()) {
                            return false;
                        }
                        continue;
                    }
                }
            }
        }
        return true;
    }

   public:
    WaveFunctionCollapse(int w, int h, int c,
                         unsigned int seed = std::random_device{}())
        : width(w), height(h), channels(c), rng(seed) {
        wave.resize(height, std::vector<std::set<Pattern>>(width));
        entropy_cache.resize(height, std::vector<EntropyInfo>(width));
    }

    void initialize(const unsigned char *input, int input_width,
                    int input_height) {
        extractPatterns(input, input_width, input_height);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                wave[y][x] =
                    std::set<Pattern>(patterns.begin(), patterns.end());
                updateEntropyCache(x, y);
            }
        }
    }

    bool generate() {
        int iteration = 0;
        int backtrack_count = 0;

        while (true) {
            auto [x, y] = findLowestEntropy();
            if (x == -1) {
                return true;
            }

            if (wave[y][x].empty()) {
                // Try local repair first
                if (tryLocalRepair(x, y)) {
                    continue;
                }

                // If repair failed, try backtracking
                if (!restoreState()) {
                    return false;
                }

                backtrack_count++;
                continue;
            }

            // Select a pattern based on frequency using pre-computed weights
            const auto &weights = entropy_cache[y][x].weights;
            double total_weight = entropy_cache[y][x].sum_weights;

            std::uniform_real_distribution<double> dist(0, total_weight);
            double r = dist(rng);
            double sum = 0;
            size_t selected = 0;

            for (size_t i = 0; i < weights.size(); i++) {
                sum += weights[i];
                if (r <= sum) {
                    selected = i;
                    break;
                }
            }

            std::vector<Pattern> possible_patterns(wave[y][x].begin(),
                                                   wave[y][x].end());
            Pattern chosen = possible_patterns[selected];

            // Save state before making changes
            saveState(x, y, chosen);

            wave[y][x] = {chosen};
            updateEntropyCache(x, y);
            propagate(x, y);

            iteration++;
        }
    }

    void getResult(std::vector<unsigned char> &output) {
        output.resize(width * height * channels);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (wave[y][x].empty()) {
                    // Fill with a visible error pattern (red)
                    for (int c = 0; c < channels; c++) {
                        output[((height - 1 - y) * width + x) * channels + c] =
                            (c == 0) ? 255 : 0;  // Red for error
                    }
                    continue;
                }

                const Pattern &p = *wave[y][x].begin();
                for (int c = 0; c < channels; c++) {
                    output[((height - 1 - y) * width + x) * channels + c] =
                        p.data[c];
                }
            }
        }
    }

    bool hasEmptyWave(int x, int y) { return wave[y][x].empty(); }
};

struct GenerationResult {
    std::vector<unsigned char> output;
    bool success;
    int attempt;
};

GenerationResult tryGeneration(const unsigned char *input, int input_width,
                               int input_height, int width, int height,
                               int channels, unsigned int seed) {
    GenerationResult result;
    result.success = false;
    result.attempt = seed;

    WaveFunctionCollapse wfc(width, height, channels, seed);
    wfc.initialize(input, input_width, input_height);

    if (wfc.generate()) {
        wfc.getResult(result.output);

        // Check if we have any empty waves
        bool has_empty_waves = false;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (wfc.hasEmptyWave(x, y)) {
                    has_empty_waves = true;
                    break;
                }
            }
            if (has_empty_waves) break;
        }

        if (!has_empty_waves) {
            result.success = true;
        }
    }

    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_image> --size=<number>"
                  << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];
    std::string size_arg = argv[2];

    // Parse size argument
    if (size_arg.substr(0, 7) != "--size=") {
        std::cerr << "Error: Second argument must be in format --size=<number>"
                  << std::endl;
        return 1;
    }

    int size;
    try {
        size = std::stoi(size_arg.substr(7));
        if (size <= 0) throw std::invalid_argument("Size must be positive");
    } catch (const std::exception &) {
        std::cerr << "Error: Invalid size value. Must be a positive number."
                  << std::endl;
        return 1;
    }

    // Read the input image
    int flowers_width, flowers_height, flowers_channels;
    unsigned char *flowers_data =
        stbi_load(input_filename.c_str(), &flowers_width, &flowers_height,
                  &flowers_channels, 4);

    if (!flowers_data) {
        std::cerr << "Failed to load " << input_filename << ": "
                  << stbi_failure_reason() << std::endl;
        return 1;
    }

    // Create output image dimensions
    const int width = size;
    const int height = size;
    const int channels = 4;  // RGBA

    const int MAX_ATTEMPTS =
        1;  // Maximum number of attempts with different seeds
    // std::vector<std::future<GenerationResult>> futures;
    // std::mutex cout_mutex;  // For synchronized console output

    const int MAX_ATTEMPTS =
        1;  // Maximum number of attempts with different seeds
    // std::vector<std::future<GenerationResult>> futures;
    // std::mutex cout_mutex;  // For synchronized console output

    GenerationResult best_result;
    bool found_success = false;

    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        unsigned int seed = std::random_device{}();
        GenerationResult result =
            tryGeneration(flowers_data, flowers_width, flowers_height, width,
                          height, channels, seed);
        if (result.success && !found_success) {
            best_result = std::move(result);
            found_success = true;
        }
    }

    // Free the loaded image data
    stbi_image_free(flowers_data);

    // Write the final image to a PNG file
    if (stbi_write_png("output.png", width, height, channels,
                       best_result.output.data(), width * channels)) {
        std::cout << "Successfully created output.png" << std::endl;
    } else {
        std::cerr << "Failed to create output.png" << std::endl;
        return 1;
    }

    return 0;
}