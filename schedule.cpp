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
#include <algorithm>

// ============ Global Variables ============
std::string g_input = "abc";
std::string g_message;
std::string g_padded;
std::vector<std::string> g_blocks;
int g_block_number = 0;
std::string g_block;
std::vector<uint32_t> g_schedule;
std::vector<std::vector<uint32_t>> g_memory;
std::string g_state;
std::string g_delay = "normal";
std::string indent = "  ";

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

std::string bits(uint32_t x) {
    return std::bitset<32>(x).to_string();
}

std::string formatWord(int i) {
    std::string s = std::to_string(i);
    if (s.length() == 1) return " " + s;
    return s;
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

uint32_t add(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    uint64_t total = static_cast<uint64_t>(a) + b + c + d;
    return static_cast<uint32_t>(total & 0xFFFFFFFF);
}

// ============ Padding and Block Functions ============

std::string stringToBinary(const std::string& str) {
    std::string result;
    for (unsigned char c : str) {
        result += std::bitset<8>(c).to_string();
    }
    return result;
}

std::string padding(const std::string& message) {
    size_t l = message.size();
    int k = (448 - static_cast<int>(l) - 1) % 512;
    if (k < 0) k += 512;
    
    std::string l64 = std::bitset<64>(l).to_string();
    return message + "1" + std::string(k, '0') + l64;
}

std::vector<std::string> split(const std::string& message, int size = 512) {
    std::vector<std::string> blocks;
    for (size_t i = 0; i < message.length(); i += size) {
        blocks.push_back(message.substr(i, size));
    }
    return blocks;
}

// ============ Message Schedule Calculation ============

void calculateSchedule() {
    // Clear previous schedule and memory
    g_schedule.clear();
    g_memory.clear();
    
    // First 16 words from block
    for (int i = 0; i < 16; i++) {
        std::string word = g_block.substr(i * 32, 32);
        g_schedule.push_back(std::bitset<32>(word).to_ulong());
    }
    
    // Initialize memory array (first 16 entries are empty)
    g_memory.resize(16);
    
    // Calculate remaining 48 words
    for (int i = 16; i <= 63; i++) {
        uint32_t s1 = sigma1(g_schedule[i - 2]);
        uint32_t w7 = g_schedule[i - 7];
        uint32_t s0 = sigma0(g_schedule[i - 15]);
        uint32_t w16 = g_schedule[i - 16];
        
        uint32_t word = add(s1, w7, s0, w16);
        g_schedule.push_back(word);
        
        // Store the values used in calculation
        std::vector<uint32_t> memory_entry = {s1, w7, s0, w16};
        g_memory.push_back(memory_entry);
    }
}

// ============ Visualization ============

void showMessageSchedule() {
    // Frame 1: Show block
    clearScreen();
    if (!g_state.empty()) {
        std::cout << g_state << "\n" << std::endl;
    }
    std::cout << indent << "-------" << std::endl;
    std::cout << indent << "block #" << g_block_number << ":" << std::endl;
    std::cout << indent << "-------" << std::endl;
    std::cout << indent << g_block << std::endl;
    delay("slowest");
    
    // Frame 2: Add message schedule title
    clearScreen();
    if (!g_state.empty()) {
        std::cout << g_state << "\n" << std::endl;
    }
    std::cout << indent << "-------" << std::endl;
    std::cout << indent << "block #" << g_block_number << ":" << std::endl;
    std::cout << indent << "-------" << std::endl;
    std::cout << indent << g_block << std::endl;
    std::cout << std::endl;
    std::cout << indent << "----------------" << std::endl;
    std::cout << indent << "message schedule:" << std::endl;
    std::cout << indent << "----------------" << std::endl;
    delay("slowest");
    
    // Frame 3: Build schedule word by word
    for (int i = 0; i < 64; i++) {
        clearScreen();
        
        // Show state if exists
        if (!g_state.empty()) {
            std::cout << g_state << "\n" << std::endl;
        }
        
        // Show block
        std::cout << indent << "-------" << std::endl;
        std::cout << indent << "block #" << g_block_number << ":" << std::endl;
        std::cout << indent << "-------" << std::endl;
        
        if (i <= 15) {
            // Show remaining part of block
            std::string remaining = g_block.substr((i + 1) * 32);
            std::cout << indent << remaining << std::string(g_block.length() - remaining.length(), ' ') << std::endl;
        } else {
            std::cout << indent << std::string(g_block.length(), ' ') << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << indent << "----------------" << std::endl;
        std::cout << indent << "message schedule:" << std::endl;
        std::cout << indent << "----------------" << std::endl;
        
        // Show schedule words up to current i
        for (int j = 0; j <= i; j++) {
            if (i <= 15) {
                // First 16 words come directly from block
                std::cout << indent << "W" << formatWord(j) << " " << bits(g_schedule[j]) << std::endl;
            } else {
                // Words 16-63 with their calculation
                if (j < i - 16) {
                    // Show nothing special
                    std::cout << indent << "W" << formatWord(j) << " " << bits(g_schedule[j]) << std::endl;
                } else if (j == i - 16) {
                    std::cout << indent << "W" << formatWord(j) << " " << bits(g_schedule[j]) 
                              << " ->    " << bits(g_memory[i][3]) << std::endl;
                } else if (j == i - 15) {
                    std::cout << indent << "W" << formatWord(j) << " " << bits(g_schedule[j]) 
                              << " -> σ0 " << bits(g_memory[i][2]) << std::endl;
                } else if (j == i - 7) {
                    std::cout << indent << "W" << formatWord(j) << " " << bits(g_schedule[j]) 
                              << " ->    " << bits(g_memory[i][1]) << std::endl;
                } else if (j == i - 2) {
                    std::cout << indent << "W" << formatWord(j) << " " << bits(g_schedule[j]) 
                              << " -> σ1 " << bits(g_memory[i][0]) << std::endl;
                } else if (j == i) {
                    std::cout << indent << "W" << formatWord(j) << " " << bits(g_schedule[j]) 
                              << " = σ1(t-2) + (t-7) + σ0(t-15) + (t-16)" << std::endl;
                } else {
                    std::cout << indent << "W" << formatWord(j) << " " << bits(g_schedule[j]) << std::endl;
                }
            }
        }
        
        // Pause after showing first 16 words
        if (i == 15) {
            delay("normal");
        } else {
            delay("fastest");
        }
    }
    
    // Frame 4: Show final schedule (last 16 words)
    clearScreen();
    if (!g_state.empty()) {
        std::cout << g_state << "\n" << std::endl;
    }
    std::cout << indent << "-------" << std::endl;
    std::cout << indent << "block #" << g_block_number << ":" << std::endl;
    std::cout << indent << "-------" << std::endl;
    std::cout << indent << std::string(g_block.length(), ' ') << std::endl;
    std::cout << std::endl;
    std::cout << indent << "----------------" << std::endl;
    std::cout << indent << "message schedule:" << std::endl;
    std::cout << indent << "----------------" << std::endl;
    
    for (int i = 47; i <= 63; i++) {
        std::cout << indent << "W" << formatWord(i) << " " << bits(g_schedule[i]) << std::endl;
    }
    delay("end");
    
    // Save final state
    std::stringstream ss;
    ss << g_state << "\n";
    ss << indent << "-------" << std::endl;
    ss << indent << "block #" << g_block_number << ":" << std::endl;
    ss << indent << "-------" << std::endl;
    ss << indent << std::string(g_block.length(), ' ') << std::endl;
    ss << std::endl;
    ss << indent << "----------------" << std::endl;
    ss << indent << "message schedule:" << std::endl;
    ss << indent << "----------------" << std::endl;
    
    for (int i = 47; i <= 63; i++) {
        ss << indent << "W" << formatWord(i) << " " << bits(g_schedule[i]) << std::endl;
    }
    g_state = ss.str();
    
    // Final clear and display
    clearScreen();
    std::cout << g_state;
}

// ============ Main ============

int main(int argc, char* argv[]) {
    // Parse command line arguments
    if (argc >= 2) {
        std::string arg = argv[1];
        
        // Check if it's a 512-bit binary string
        bool isBinary = true;
        for (char c : arg) {
            if (c != '0' && c != '1') {
                isBinary = false;
                break;
            }
        }
        
        if (isBinary && arg.length() == 512) {
            g_block = arg;
            g_input = "(binary block)";
        } else {
            g_input = arg;
        }
    }
    
    if (argc >= 3) {
        g_delay = argv[2];
    }
    
    // If no block provided, generate from default input
    if (g_block.empty()) {
        g_message = stringToBinary(g_input);
        g_padded = padding(g_message);
        g_blocks = split(g_padded, 512);
        g_block_number = 0;
        g_block = g_blocks[g_block_number];
    }
    
    // Calculate the message schedule
    calculateSchedule();
    
    // Note about hitting enter to step
    if (g_delay == "enter") {
        std::cout << "Press Enter to step through each frame." << std::endl;
        std::cin.get();
    }
    
    // Show visualization
    showMessageSchedule();
    
    return 0;
}
