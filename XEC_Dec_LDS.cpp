#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <chrono>
#include <iomanip>
#include <string>

// Key for encryption and decryption
std::bitset<256> key("110100101101111010010111101010111000110011001011110110001111010000111001010011111100110100101100100110100011100101011110110101011011001001100010111010110011111010001001110100100010010011000101100110101010011011000111101001100100010110111110110001011001101");

// Function to reverse mutation on a 256-bit segment
void reverse_mutate(std::bitset<256> &segment) {
    std::vector<size_t> bits_to_flip = {50, 100, 150}; // Original mutation positions
    for (size_t bit : bits_to_flip) {
        if (bit < segment.size()) {
            segment.flip(bit);
        }
    }
}

// Function to reverse crossover (XOR with key)
void reverse_crossover(std::bitset<256> &segment) {
    segment ^= key; // XOR with key again to revert
}

// Function to decrypt a binary chunk by reversing encryption steps
std::string decrypt_chunk(const std::string &binary_chunk) {
    std::vector<std::bitset<256>> segments;
    size_t segment_count = binary_chunk.size() / 256;

    for (size_t i = 0; i < segment_count; ++i) {
        std::bitset<256> segment(binary_chunk.substr(i * 256, 256));

        // Reverse mutation and crossover
        reverse_mutate(segment);
        reverse_crossover(segment);

        segments.push_back(segment);
    }

    std::string decrypted_chunk;
    for (const auto &segment : segments) {
        decrypted_chunk += segment.to_string();
    }

    return decrypted_chunk;
}

// Function to read an encrypted file, decrypt it, and measure decryption time
void decrypt_file(const std::string &encrypted_filename, const std::string &decrypted_filename, 
                  double &decryption_time_s, double &throughput) {
    std::ifstream encrypted_file("output/" + encrypted_filename);
    std::ofstream decrypted_file("plaintext/" + decrypted_filename);
    if (!encrypted_file || !decrypted_file) {
        throw std::runtime_error("Cannot open encrypted or decrypted file.");
    }

    std::string binary_chunk;
    auto start = std::chrono::high_resolution_clock::now(); // Start timing
    
    while (std::getline(encrypted_file, binary_chunk)) {
        std::string decrypted_chunk = decrypt_chunk(binary_chunk);
        decrypted_file << decrypted_chunk;
    }

    auto end = std::chrono::high_resolution_clock::now(); // End timing
    std::chrono::duration<double> duration = end - start;
    decryption_time_s = duration.count();

    // Calculate throughput as (file size in bytes / decryption time)
    encrypted_file.clear();
    encrypted_file.seekg(0, std::ios::end);
    double file_size_bytes = encrypted_file.tellg();
    throughput = file_size_bytes / decryption_time_s;
}

// Main function to decrypt each file in the list of datasets and calculate metrics
int main() {
    std::vector<std::string> encrypted_datasets = {

     "encrypted_D1.txt", "encrypted_D2.txt", "encrypted_D3.txt",
        "encrypted_D4.txt", "encrypted_D5.txt", "encrypted_D6.txt",

        "encrypted_D7.txt", "encrypted_D8.txt", "encrypted_D9.txt",
        "encrypted_D10.txt", "encrypted_D11.txt", "encrypted_D12.txt"

    };

    std::cout << std::setw(15) << "Dataset" 
              << std::setw(20) << "Decryption Time (s)"
           
              << std::setw(25) << "Throughput (Bytes/sec)" << std::endl;

    for (const auto &encrypted_filename : encrypted_datasets) {
        try {
            std::string decrypted_filename = "decrypted_" + encrypted_filename.substr(encrypted_filename.find_last_of("_") + 1);
            double decryption_time_s = 0.0, throughput = 0.0;
            decrypt_file(encrypted_filename, decrypted_filename, decryption_time_s, throughput);

            // Convert decryption time to milliseconds and microseconds
            double decryption_time_ms = decryption_time_s * 1000;
       

            std::cout << std::setw(15) << encrypted_filename 
                      << std::fixed << std::setprecision(6) << std::setw(20) << decryption_time_s
                
                     
                      << std::setw(25) << throughput << std::endl;

        } catch (const std::exception &e) {
            std::cerr << "Error decrypting " << encrypted_filename << ": " << e.what() << std::endl;
        }
    }

    return 0;
}

