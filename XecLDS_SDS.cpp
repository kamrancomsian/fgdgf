#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <bitset>
#include <chrono>
#include <iomanip> // For tabular formatting

// Function to get the size of the file in bytes
std::streampos getFileSize(const std::string &filename) {
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

// Function to apply two-point crossover between 128-bit segments
void single_point_crossover(std::bitset<128> &segment) {
    std::bitset<128> key("01010000011001010110000101100011011001010010000001110111011010010111010001101000001000000110100001101111011011100110111101110010");
    segment ^= key;
}

// Function to apply mutation on a 128-bit segment (flip one bit)
void mutate(std::bitset<128> &segment) {
    size_t bit_to_flip = 50;
    segment.flip(bit_to_flip);
}

// Function to calculate Hamming distance (count number of differing bits)
size_t hamming_distance(const std::bitset<128> &a, const std::bitset<128> &b) {
    return (a ^ b).count(); // XOR the two bitsets and count the number of 1s (differing bits)
}

// Function to convert a 1024-bit chunk (as a string) into multiple 128-bit segments
std::vector<std::bitset<128>> convert_chunk_to_segments(const std::string &chunk) {
    std::vector<std::bitset<128>> segments;
    size_t segment_count = chunk.size() / 128;
    for (size_t i = 0; i < segment_count; ++i) {
        std::bitset<128> segment(chunk.substr(i * 128, 128));
        segments.push_back(segment);
    }
    return segments;
}

// Function to convert multiple 128-bit segments to a 1024-bit chunk
std::string convert_segments_to_chunk(const std::vector<std::bitset<128>> &segments) {
    std::string chunk;
    for (const auto &segment : segments) {
        chunk += segment.to_string();
    }
    return chunk;
}

// Function to read file in chunks sequentially and calculate Avalanche Effect
void read_file_in_chunks(const std::string &filename, std::size_t chunk_size, std::vector<std::string> &output_list, double &total_avalanche_effect, size_t &total_bits_processed) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string chunk;
    while (file.good()) {
        chunk.resize(chunk_size);
        file.read(&chunk[0], chunk_size);
        chunk.resize(file.gcount());

        if (chunk.empty()) {
            break;
        }

        std::string binary_result;
        text_to_binary(chunk, binary_result);
        auto segments = convert_chunk_to_segments(binary_result);

        for (auto &segment : segments) {
            std::bitset<128> original_segment = segment;

            // Apply crossover and mutation
            single_point_crossover(segment);
            mutate(segment);

            // Calculate Hamming distance (Avalanche Effect)
            size_t flipped_bits = hamming_distance(original_segment, segment);
            total_avalanche_effect += flipped_bits;

            total_bits_processed += 128; // Count total bits (128 bits per segment)
        }

        binary_result = convert_segments_to_chunk(segments);
        output_list.push_back(binary_result);
    }
}

// Function to write binary data to a text file
void write_binary_to_file(const std::string &output_filename, const std::vector<std::string> &binary_output) {
    std::ofstream outfile(output_filename);
    if (!outfile) {
        throw std::runtime_error("Cannot open output file");
    }

    for (const auto &binary_chunk : binary_output) {
        outfile << binary_chunk << "\n";
    }
}

// Main function with modified output formatting
int main() {
    // List of dataset files in the "dataset" folder
    std::vector<std::string> datasets = {
         
       // "dataset/D1.txt", 
       // "dataset/D2.txt", 
       // "dataset/D3.txt", 
        "dataset/D4.txt", 
        "dataset/D5.txt", 
       // "dataset/D6.txt",

       // "dataset/D7.txt", 
       // "dataset/D8.txt", 
       // "dataset/D9.txt", 
       // "dataset/D10.txt", 
       // "dataset/D11.txt", 
       // "dataset/D12.txt"
    };

    // Tabular data for output
    std::cout << std::setw(15) << "Dataset" 
              << std::setw(25) << "Encryption Time (s)" 

              << std::setw(30) << "Avalanche Effect (%)" 
              << std::setw(30) << "Throughput (Bytes/sec)" 
              << "\n";

    for (const auto &input_filename : datasets) {
        const std::size_t chunk_size = 1048576; // 1MB chunk size
        std::vector<std::string> binary_output;
        double total_flipped_bits = 0.0;
        size_t total_bits_processed = 0;

        try {
            // Get the plaintext size in bytes
            std::streampos plaintext_size = getFileSize(input_filename);

            // Start encryption timer
            auto start_time = std::chrono::high_resolution_clock::now();
            read_file_in_chunks(input_filename, chunk_size, binary_output, total_flipped_bits, total_bits_processed);
            auto end_time = std::chrono::high_resolution_clock::now();
            
            // Calculate encryption times
            auto encryption_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
            auto encryption_time_ms = encryption_time_us / 1000.0;  // Convert to milliseconds
            double encryption_time_s = encryption_time_us / 1e6;    // Convert to seconds

            // Calculate Average Avalanche Effect
            double avalanche_effect = (total_flipped_bits / total_bits_processed) * 100.0;

            // Calculate Average Encryption Throughput in Bytes per second
            double throughput = plaintext_size / encryption_time_s;

            // Output the result in tabular form without scientific notation
            std::cout << std::setw(15) << input_filename
                      << std::setw(25) << std::fixed << std::setprecision(6) << encryption_time_s
                     << std::setw(30) << std::fixed << std::setprecision(4) << avalanche_effect
                      << std::setw(30) << std::fixed << std::setprecision(2) << throughput
                      << "\n";
        } catch (const std::exception &e) {
            std::cerr << "Error processing " << input_filename << ": " << e.what() << std::endl;
        }
    }

    return 0;
}
