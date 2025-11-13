#include <iostream> 
#include <fstream>
#include <vector>
#include <thread>
#include <sstream>
#include <bitset>
#include <mutex>
#include <random>
#include <chrono>
#include <iomanip> // For tabular formatting

// Function to get the size of the file in bytes
std::streampos getFileSize(const std::string& filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

// Function to convert a text chunk to binary
void text_to_binary(const std::string &chunk, std::string &binary_result) {
    std::stringstream binary_stream;
    for (char ch : chunk) {
        binary_stream << std::bitset<8>(ch);
    }
    binary_result = binary_stream.str();
}

// Function to apply two-point crossover between 256-bit segments
void single_point_crossover(std::bitset<256> &segment) {
    std::bitset<256> key("110100101101111010010111101010111000110011001011110110001111010000111001010011111100110100101100100110100011100101011110110101011011001001100010111010110011111010001001110100100010010011000101100110101010011011000111101001100100010110111110110001011001101");
    segment ^= key;
}
 
// Function to apply mutation on a 256-bit segment (flip one bit)
void mutate(std::bitset<256> &segment) {
    std::vector<size_t> bits_to_flip = {50, 100, 150}; // Example positions
    for (size_t bit : bits_to_flip) {
        if (bit < segment.size()) {  // Ensure bit index is within bounds
            segment.flip(bit);
        }
    }
}

// Function to calculate Hamming distance (count number of differing bits)
size_t hamming_distance(const std::bitset<256> &a, const std::bitset<256> &b) {
    return (a ^ b).count(); // XOR the two bitsets and count the number of 1s (differing bits)
}

// Function to convert a 1024-bit chunk (as a string) into multiple 256-bit segments
std::vector<std::bitset<256>> convert_chunk_to_segments(const std::string &chunk) {
    std::vector<std::bitset<256>> segments;
    size_t segment_count = chunk.size() / 256;
    for (size_t i = 0; i < segment_count; ++i) {
        std::bitset<256> segment(chunk.substr(i * 256, 256));
        segments.push_back(segment);
    }
    return segments;
}

// Function to convert multiple 256-bit segments to a 1024-bit chunk
std::string convert_segments_to_chunk(const std::vector<std::bitset<256>> &segments) {
    std::string chunk;
    for (const auto &segment : segments) {
        chunk += segment.to_string();
    }
    return chunk;
}

// Function to read file in chunks using threading and calculate Avalanche Effect
void read_file_in_chunks(const std::string &filename, std::size_t chunk_size, std::vector<std::string> &output_list, std::mutex &mtx, double &total_avalanche_effect, size_t &total_bits_processed) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::vector<std::thread> threads;
    std::string chunk;
    size_t index = 0;
    double local_avalanche_effect = 0.0;
    size_t processed_segments = 0;

    std::streampos fileSize = getFileSize(filename);
    double fileSizeMB = static_cast<double>(fileSize) / (1024 * 1024);

    while (file.good()) {
        chunk.resize(chunk_size);
        file.read(&chunk[0], chunk_size);
        chunk.resize(file.gcount());

        if (chunk.empty()) {
            break;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            output_list.push_back(""); // Placeholder for the binary result
        }

        threads.emplace_back([chunk, &output_list, &mtx, index, &local_avalanche_effect, &processed_segments, &total_bits_processed]() mutable {
            std::string binary_result;
            text_to_binary(chunk, binary_result);
            auto segments = convert_chunk_to_segments(binary_result);

            for (size_t i = 0; i < segments.size(); i++) {
                std::bitset<256> original_segment = segments[i];

                single_point_crossover(segments[i]);
                mutate(segments[i]);

                size_t flipped_bits = hamming_distance(original_segment, segments[i]);
                local_avalanche_effect += flipped_bits;

                processed_segments++;
                total_bits_processed += 256;
            }

            binary_result = convert_segments_to_chunk(segments);
            {
                std::lock_guard<std::mutex> lock(mtx);
                output_list[index] = binary_result;
            }
        });

        index++;
    }

    for (auto &thread : threads) {
        thread.join();
    }

    std::lock_guard<std::mutex> lock(mtx);
    if (processed_segments > 0) {
        total_avalanche_effect += local_avalanche_effect;
    }
}

// Function to write binary data to a text file in the 'output' folder
void write_binary_to_file(const std::string &output_filename, const std::vector<std::string> &binary_output) {
    std::ofstream outfile("output/" + output_filename);
    if (!outfile) {
        throw std::runtime_error("Cannot open output file");
    }

    for (const auto &binary_chunk : binary_output) {
        outfile << binary_chunk << "\n";
    }
}

// Main function with modified output formatting
int main() {
    std::vector<std::string> datasets = {
                  
  // "dataset/D1.txt", 
       // "dataset/D2.txt", 
    //    "dataset/D3.txt", 
        "dataset/D4.txt", 
        "dataset/D5.txt", 
      //  "dataset/D6.txt",
     //   "dataset/D7.txt", 
      //  "dataset/D8.txt", 
       // "dataset/D9.txt", 
     //   "dataset/D10.txt", 
    //    "dataset/D11.txt", 
     //   "dataset/D12.txt"
    };

    std::cout << std::setw(15) << "Dataset" 
              << std::setw(25) << "Encryption Time (s)" 
              << std::setw(25) << "Encryption Time (ms)"
              << std::setw(25) << "Encryption Time (μs)"
              << std::setw(30) << "Average Avalanche Effect (%)" 
              << std::setw(30) << "Throughput (Bytes/sec)" 
              << "\n";
    
    for (const auto& input_filename : datasets) {
        const std::size_t chunk_size = 1048576; // 1MB chunk size
        std::vector<std::string> binary_output;
        std::mutex mtx;
        double total_flipped_bits = 0.0;
        size_t total_bits_processed = 0;

        try {
            std::streampos plaintext_size = getFileSize(input_filename);
            auto start_time = std::chrono::high_resolution_clock::now();
            read_file_in_chunks(input_filename, chunk_size, binary_output, mtx, total_flipped_bits, total_bits_processed);
            
            // Define a unique output filename for each dataset
            std::string output_filename = "encrypted_" + input_filename.substr(input_filename.find_last_of("/") + 1);
            write_binary_to_file(output_filename, binary_output);

            auto end_time = std::chrono::high_resolution_clock::now();
            auto encryption_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
            auto encryption_time_ms = encryption_time_us / 1000.0;
            double encryption_time_s = encryption_time_us / 1e6;

            double avalanche_effect = (total_flipped_bits / total_bits_processed) * 100.0;
            double throughput = plaintext_size / encryption_time_s;

            std::cout << std::setw(15) << input_filename
                      << std::setw(25) << std::fixed << std::setprecision(6) << encryption_time_s
                      << std::setw(25) << std::fixed << std::setprecision(3) << encryption_time_ms
                      << std::setw(25) << encryption_time_us
                      << std::setw(30) << std::fixed << std::setprecision(4) << avalanche_effect
                      << std::setw(30) << std::fixed << std::setprecision(2) << throughput
                      << "\n";
        } catch (const std::exception &e) {
            std::cerr << "Error processing " << input_filename << ": " << e.what() << std::endl;
        }
    }

    return 0;
}

