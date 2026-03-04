#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>

// ============ Global Variables ============
std::string g_input = "abc";
std::string g_type = "string";
std::vector<uint8_t> g_bytes;
std::string g_message;
std::string g_padded;
std::vector<std::string> g_blocks;
std::vector<uint32_t> g_hash;
std::string g_digest;
std::string g_state;
std::string g_delay = "normal";

// ============ Utility Functions ============

void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

void delay(const std::string& speed) {
    if (g_delay == "enter") {
        std::cin.get();
    } else if (g_delay == "nodelay") {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    } else {
        double multiplier = 1.0;
        if (g_delay == "fast") multiplier = 0.5;
        
        int sleepTime = 0;
        if (speed == "fastest") sleepTime = 100 * multiplier;
        else if (speed == "fast") sleepTime = 200 * multiplier;
        else if (speed == "normal") sleepTime = 400 * multiplier;
        else if (speed == "slow") sleepTime = 600 * multiplier;
        else if (speed == "slowest") sleepTime = 800 * multiplier;
        else if (speed == "end") sleepTime = 1000 * multiplier;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    }
}

std::string bits(uint32_t x, int n = 32) {
    std::string binary = std::bitset<32>(x).to_string();
    return binary.substr(32 - n, n);
}

std::string hex(uint32_t i) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(8) << i;
    return ss.str();
}

std::string bytesToHex(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

std::string bytesToBinary(const std::vector<uint8_t>& bytes) {
    std::string result;
    for (uint8_t byte : bytes) {
        result += std::bitset<8>(byte).to_string();
    }
    return result;
}

std::string bytesInspect(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < bytes.size(); i++) {
        if (i > 0) ss << ", ";
        ss << static_cast<int>(bytes[i]);
    }
    ss << "]";
    return ss.str();
}

// ============ SHA-256 Operations ============

uint32_t rotr(int n, uint32_t x) {
    return (x >> n) | (x << (32 - n));
}

uint32_t shr(int n, uint32_t x) {
    return x >> n;
}

uint32_t sigma0(uint32_t x) {
    return rotr(7, x) ^ rotr(18, x) ^ shr(3, x);
}

uint32_t sigma1(uint32_t x) {
    return rotr(17, x) ^ rotr(19, x) ^ shr(10, x);
}

uint32_t usigma0(uint32_t x) {
    return rotr(2, x) ^ rotr(13, x) ^ rotr(22, x);
}

uint32_t usigma1(uint32_t x) {
    return rotr(6, x) ^ rotr(11, x) ^ rotr(25, x);
}

uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ ((~x) & z);
}

uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

uint32_t add(uint32_t a, uint32_t b) {
    return (a + b) & 0xFFFFFFFF;
}

uint32_t add(uint32_t a, uint32_t b, uint32_t c) {
    return (a + b + c) & 0xFFFFFFFF;
}

uint32_t add(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    uint64_t total = static_cast<uint64_t>(a) + b + c + d;
    return static_cast<uint32_t>(total & 0xFFFFFFFF);
}

uint32_t add(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
    uint64_t total = static_cast<uint64_t>(a) + b + c + d + e;
    return static_cast<uint32_t>(total & 0xFFFFFFFF);
}

// ============ SHA-256 Constants ============

std::vector<uint32_t> calculateK() {
    std::vector<uint32_t> constants;
    int primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 
                    59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 
                    127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 
                    191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 
                    257, 263, 269, 271, 277, 281, 283, 293, 307, 311};
    
    for (int prime : primes) {
        double cubeRoot = std::pow(prime, 1.0 / 3.0);
        double fractional = cubeRoot - std::floor(cubeRoot);
        uint32_t value = static_cast<uint32_t>(fractional * std::pow(2, 32));
        constants.push_back(value);
    }
    return constants;
}

std::vector<uint32_t> calculateIV() {
    std::vector<uint32_t> initial;
    int primes[] = {2, 3, 5, 7, 11, 13, 17, 19};
    
    for (int prime : primes) {
        double squareRoot = std::sqrt(prime);
        double fractional = squareRoot - std::floor(squareRoot);
        uint32_t value = static_cast<uint32_t>(fractional * std::pow(2, 32));
        initial.push_back(value);
    }
    return initial;
}

const std::vector<uint32_t> K = calculateK();
const std::vector<uint32_t> IV = calculateIV();

// ============ SHA-256 Core Functions ============

std::string padding(const std::string& message) {
    size_t l = message.size();
    int k = (448 - static_cast<int>(l) - 1) % 512;
    if (k < 0) k += 512;
    
    std::string l64 = bits(l, 64);
    return message + "1" + std::string(k, '0') + l64;
}

std::vector<std::string> split(const std::string& message, int size = 512) {
    std::vector<std::string> blocks;
    for (size_t i = 0; i < message.length(); i += size) {
        blocks.push_back(message.substr(i, size));
    }
    return blocks;
}

std::vector<uint32_t> calculate_schedule(const std::string& block) {
    std::vector<uint32_t> schedule;

    for (size_t i = 0; i < 16; i++) {
        std::string word = block.substr(i * 32, 32);
        schedule.push_back(std::bitset<32>(word).to_ulong());
    }

    for (int i = 16; i <= 63; i++) {
        schedule.push_back(add(sigma1(schedule[i - 2]), 
                               schedule[i - 7], 
                               sigma0(schedule[i - 15]), 
                               schedule[i - 16]));
    }
    return schedule;
}

std::vector<uint32_t> compression(const std::vector<uint32_t>& initial, 
                                  const std::vector<uint32_t>& schedule) {
    uint32_t h = initial[7];
    uint32_t g = initial[6];
    uint32_t f = initial[5];
    uint32_t e = initial[4];
    uint32_t d = initial[3];
    uint32_t c = initial[2];
    uint32_t b = initial[1];
    uint32_t a = initial[0];

    for (int i = 0; i < 64; i++) {
        uint32_t t1 = add(schedule[i], K[i], usigma1(e), ch(e, f, g), h);
        uint32_t t2 = add(usigma0(a), maj(a, b, c));

        h = g;
        g = f;
        f = e;
        e = add(d, t1);
        d = c;
        c = b;
        b = a;
        a = add(t1, t2);
    }

    std::vector<uint32_t> hash(8);
    hash[7] = add(initial[7], h);
    hash[6] = add(initial[6], g);
    hash[5] = add(initial[5], f);
    hash[4] = add(initial[4], e);
    hash[3] = add(initial[3], d);
    hash[2] = add(initial[2], c);
    hash[1] = add(initial[1], b);
    hash[0] = add(initial[0], a);

    return hash;
}

// ============ Message Visualization ============

void showMessage() {
    // Frame 1: Simple title
    clearScreen();
    std::cout << "========================" << std::endl;
    std::cout << "STEP 1: Original Message" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "-------" << std::endl;
    std::cout << "message:" << std::endl;
    std::cout << "-------" << std::endl;
    delay("fast");
    
    // Frame 2: Add input
    clearScreen();
    std::cout << "========================" << std::endl;
    std::cout << "STEP 1: Original Message" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "-------" << std::endl;
    std::cout << "message:" << std::endl;
    std::cout << "-------" << std::endl;
    std::cout << "input:   \"" << g_input << "\" (" << g_type << ")" << std::endl;
    delay("normal");
    
    // Frame 3: Add bytes
    clearScreen();
    std::cout << "========================" << std::endl;
    std::cout << "STEP 1: Original Message" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "-------" << std::endl;
    std::cout << "message:" << std::endl;
    std::cout << "-------" << std::endl;
    std::cout << "input:   \"" << g_input << "\" (" << g_type << ")" << std::endl;
    std::cout << "bytes:   " << bytesInspect(g_bytes) << std::endl;
    delay("normal");
    
    // Frame 4: Add binary message
    clearScreen();
    std::cout << "========================" << std::endl;
    std::cout << "STEP 1: Original Message" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "-------" << std::endl;
    std::cout << "message:" << std::endl;
    std::cout << "-------" << std::endl;
    std::cout << "input:   \"" << g_input << "\" (" << g_type << ")" << std::endl;
    std::cout << "bytes:   " << bytesInspect(g_bytes) << std::endl;
    
    // Show binary with byte grouping
    std::cout << "message: ";
    for (size_t i = 0; i < g_bytes.size(); i++) {
        std::cout << std::bitset<8>(g_bytes[i]).to_string();
        if (i < g_bytes.size() - 1) std::cout << " ";
    }
    std::cout << std::endl;
    
    // Show hex representation
    std::cout << "hex:     0x" << bytesToHex(g_bytes) << std::endl;
    delay("end");
}

// ============ Padding Visualization ============

void showPadding() {
    clearScreen();
    std::cout << "========================" << std::endl;
    std::cout << "STEP 2: Message Padding" << std::endl;
    std::cout << "========================" << std::endl;
    
    size_t original_len = g_message.length();
    size_t padded_len = g_padded.length();
    
    std::cout << "Original message length: " << original_len << " bits" << std::endl;
    std::cout << "Padded message length:   " << padded_len << " bits" << std::endl;
    std::cout << std::endl;
    std::cout << "Padding steps:" << std::endl;
    std::cout << "  1. Append '1' bit" << std::endl;
    std::cout << "  2. Append '0' bits until length ≡ 448 mod 512" << std::endl;
    std::cout << "  3. Append 64-bit original length" << std::endl;
    std::cout << std::endl;
    
    // Show first part of padded message
    std::cout << "Padded message (first 64 bits):" << std::endl;
    std::cout << "  " << g_padded.substr(0, 64) << "..." << std::endl;
    
    delay("slow");
}

// ============ Blocks Visualization ============

void showBlocks() {
    clearScreen();
    std::cout << "========================" << std::endl;
    std::cout << "STEP 3: Message Blocks" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << g_blocks.size() << " block(s) of 512 bits" << std::endl;
    std::cout << std::endl;
    
    for (size_t i = 0; i < g_blocks.size(); i++) {
        std::cout << "Block " << i << ":" << std::endl;
        std::string block = g_blocks[i];
        
        // Show block as 16 words of 32 bits
        for (int j = 0; j < 16; j++) {
            std::cout << "  W" << std::setw(2) << j << ": " 
                      << block.substr(j * 32, 32);
            if (j == 3 || j == 7 || j == 11) std::cout << std::endl;
            else std::cout << "  ";
        }
        std::cout << std::endl;
        
        if (i < g_blocks.size() - 1) {
            std::cout << "---" << std::endl;
        }
    }
    delay("slow");
}

// ============ Initial Hash Visualization ============

void showInitialHash() {
    clearScreen();
    std::cout << "========================" << std::endl;
    std::cout << "STEP 4: Initial Hash Values (IV)" << std::endl;
    std::cout << "========================" << std::endl;
    std::cout << "First 32 bits of fractional parts of √primes" << std::endl;
    std::cout << std::endl;
    
    std::string registers[] = {"a", "b", "c", "d", "e", "f", "g", "h"};
    int primes[] = {2, 3, 5, 7, 11, 13, 17, 19};
    
    for (int i = 0; i < 8; i++) {
        std::cout << registers[i] << " = " << bits(IV[i]) 
                  << " (0x" << hex(IV[i]) << ")" << std::endl;
        std::cout << "    √" << primes[i] << " fractional part * 2^32" << std::endl;
    }
    delay("slow");
}

// ============ Message Schedule Visualization ============

void showSchedule(const std::vector<uint32_t>& schedule, int block_num) {
    clearScreen();
    std::cout << "========================" << std::endl;
    std::cout << "STEP 5: Message Schedule (Block " << block_num << ")" << std::endl;
    std::cout << "========================" << std::endl;
    
    // Show first 16 words (from block)
    std::cout << "Words 0-15 (from message block):" << std::endl;
    for (int i = 0; i < 16; i++) {
        std::cout << "  W" << std::setw(2) << i << ": " << bits(schedule[i]);
        if (i % 4 == 3) std::cout << std::endl;
        else std::cout << "  ";
    }
    
    // Show last few words
    std::cout << "\nWords 60-63 (expanded):" << std::endl;
    for (int i = 60; i < 64; i++) {
        std::cout << "  W" << i << ": " << bits(schedule[i]) << std::endl;
    }
    
    delay("normal");
}

// ============ Compression Round Visualization ============

void showCompressionRound(const std::vector<uint32_t>& hash, int round, int total_rounds) {
    clearScreen();
    std::cout << "========================" << std::endl;
    std::cout << "STEP 6: Compression - Round " << round << "/" << total_rounds << std::endl;
    std::cout << "========================" << std::endl;
    
    std::string registers[] = {"a", "b", "c", "d", "e", "f", "g", "h"};
    
    for (int i = 0; i < 8; i++) {
        std::cout << registers[i] << " = " << bits(hash[i]) 
                  << " (0x" << hex(hash[i]) << ")" << std::endl;
    }
    
    delay("fast");
}

// ============ Final Hash Visualization ============

void showFinalHash() {
    clearScreen();
    std::cout << "========================" << std::endl;
    std::cout << "STEP 7: Final SHA-256 Hash" << std::endl;
    std::cout << "========================" << std::endl;
    
    std::string registers[] = {"a", "b", "c", "d", "e", "f", "g", "h"};
    
    // Show each register
    for (int i = 0; i < 8; i++) {
        std::cout << registers[i] << " = " << bits(g_hash[i]) 
                  << " = 0x" << hex(g_hash[i]) << std::endl;
    }
    
    // Build final digest
    std::cout << "\nFinal hash: ";
    g_digest = "";
    for (int i = 0; i < 8; i++) {
        g_digest += hex(g_hash[i]);
        std::cout << hex(g_hash[i]);
        if (i < 7) std::cout << " ";
    }
    std::cout << std::endl;
    std::cout << "Length: " << g_digest.length() << " hex characters (256 bits)" << std::endl;
    std::cout << "========================" << std::endl;
    
    // Verify with known test vector for "abc"
    if (g_input == "abc") {
        std::cout << "\nTest vector for \"abc\":" << std::endl;
        std::cout << "Expected: ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" << std::endl;
        std::cout << "Actual:   " << g_digest << std::endl;
        if (g_digest == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") {
            std::cout << "✓ MATCH" << std::endl;
        } else {
            std::cout << "✗ MISMATCH" << std::endl;
        }
    }
    
    delay("end");
}

// ============ Main SHA-256 Function ============

std::string sha256(const std::string& input) {
    g_input = input;
    g_bytes.clear();
    for (char c : g_input) {
        g_bytes.push_back(static_cast<uint8_t>(c));
    }
    g_message = bytesToBinary(g_bytes);
    
    // Show message visualization
    showMessage();
    
    // Padding
    g_padded = padding(g_message);
    showPadding();
    
    // Split into blocks
    g_blocks = split(g_padded, 512);
    showBlocks();
    
    // Initialize hash
    g_hash = IV;
    showInitialHash();
    
    // Process each block
    for (size_t block_num = 0; block_num < g_blocks.size(); block_num++) {
        // Calculate message schedule
        std::vector<uint32_t> schedule = calculate_schedule(g_blocks[block_num]);
        showSchedule(schedule, block_num);
        
        // Remember starting hash
        std::vector<uint32_t> initial = g_hash;
        
        // Show compression rounds (just a few samples)
        for (int round = 0; round < 64; round += 16) {
            showCompressionRound(g_hash, round, 64);
        }
        
        // Apply compression
        g_hash = compression(initial, schedule);
    }
    
    // Show final hash
    showFinalHash();
    
    return g_digest;
}

// ============ Main ============

int main(int argc, char* argv[]) {
    // Parse command line arguments
    if (argc >= 2) {
        g_input = argv[1];
    }
    
    if (argc >= 3) {
        g_delay = argv[2];
    }
    
    // Note about hitting enter to step
    if (g_delay == "enter") {
        std::cout << "Hit enter to step through each frame." << std::endl;
        std::cin.get();
    }
    
    // Run the complete SHA-256 visualization
    std::string result = sha256(g_input);
    
    return 0;
}
