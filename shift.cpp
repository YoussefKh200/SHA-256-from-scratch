#include <iostream>
#include <string>
#include <bitset>
#include <thread>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <cmath>

// ============ Global Variables ============
std::string g_delay = "normal";
bool g_showBinary = true;
bool g_showHex = true;
bool g_showDecimal = false;

// ============ Utility Functions ============

void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

void delay(int ms) {
    if (g_delay != "nodelay" && g_delay != "enter") {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
}

void waitForEnter() {
    if (g_delay == "enter") {
        std::cin.get();
    }
}

std::string bits(uint32_t x) {
    return std::bitset<32>(x).to_string();
}

std::string bitsWithSpaces(uint32_t x) {
    std::string b = bits(x);
    std::string result;
    for (size_t i = 0; i < b.length(); i++) {
        if (i > 0 && i % 8 == 0) result += " ";
        result += b[i];
    }
    return result;
}

std::string hex(uint32_t x) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(8) << x;
    return ss.str();
}

// ============ Shift Right Function ============

uint32_t shr(int n, uint32_t x) {
    return x >> n;
}

// ============ Interactive Mode ============

void interactiveMode() {
    std::string input;
    uint32_t x;
    int n;
    
    std::cout << "=== Shift Right (SHR) Interactive Demo ===" << std::endl;
    std::cout << "Enter a 32-bit number (binary with 0b prefix, hex with 0x prefix, or decimal): ";
    std::getline(std::cin, input);
    
    if (input.substr(0, 2) == "0x") {
        // Hex input
        x = std::stoul(input, nullptr, 16);
    } else if (input.substr(0, 2) == "0b") {
        // Binary input
        std::string bin = input.substr(2);
        bin.erase(std::remove_if(bin.begin(), bin.end(), ::isspace), bin.end());
        if (bin.length() > 32) {
            std::cout << "Error: Maximum 32 bits" << std::endl;
            return;
        }
        if (bin.length() < 32) {
            bin = std::string(32 - bin.length(), '0') + bin;
        }
        x = std::bitset<32>(bin).to_ulong();
    } else {
        // Decimal input
        x = std::stoul(input);
    }
    
    std::cout << "Enter number of shifts to show (1-32): ";
    std::cin >> n;
    if (n < 1) n = 1;
    if (n > 32) n = 32;
    
    std::cout << "\nShowing SHR for x = " << bitsWithSpaces(x) << " (0x" << hex(x) << ")" << std::endl;
    std::cout << "Press Enter to start...";
    std::cin.ignore();
    std::cin.get();
    
    // Show animation
    for (int i = 1; i <= n; i++) {
        clearScreen();
        
        uint32_t result = shr(i, x);
        
        std::cout << "========================================" << std::endl;
        std::cout << "Shift Right (SHR) - Step " << i << " of " << n << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;
        
        // Show operation
        std::cout << "SHR(" << i << ", x) = x >> " << i << std::endl;
        std::cout << std::endl;
        
        // Show original
        std::cout << "Original x: " << bitsWithSpaces(x) << " (0x" << hex(x) << ")" << std::endl;
        
        // Show binary representation with visual guide
        std::string x_bits = bits(x);
        std::string result_bits = bits(result);
        
        std::cout << "\nVisual representation:" << std::endl;
        
        // Original with bits that will be lost highlighted
        std::cout << "  x:       ";
        for (int j = 0; j < 32; j++) {
            if (j < 32 - i) {
                std::cout << x_bits[j];
            } else {
                std::cout << "\033[31m" << x_bits[j] << "\033[0m"; // Red for bits to be lost
            }
            if ((j + 1) % 8 == 0) std::cout << " ";
        }
        std::cout << std::endl;
        
        // Show the shift operation
        std::cout << "           ";
        for (int j = 0; j < 32 - i; j++) {
            std::cout << "↓";
            if ((j + 1) % 8 == 0 && j < 31) std::cout << " ";
        }
        std::cout << "  ← shift right by " << i << std::endl;
        
        // Result with new zeros highlighted
        std::cout << "  result:  ";
        for (int j = 0; j < 32; j++) {
            if (j < i) {
                std::cout << "\033[32m0\033[0m"; // Green for new zeros
            } else {
                std::cout << result_bits[j];
            }
            if ((j + 1) % 8 == 0) std::cout << " ";
        }
        std::cout << std::endl;
        
        std::cout << "\n  Lost " << i << " LSB" << (i == 1 ? "" : "s") << ": ";
        for (int j = 32 - i; j < 32; j++) {
            std::cout << "\033[31m" << x_bits[j] << "\033[0m";
        }
        std::cout << std::endl;
        
        std::cout << "\n  New zeros inserted: " << i << " MSB" << (i == 1 ? "" : "s") << std::endl;
        
        // Show mathematical interpretation
        std::cout << "\nMathematical interpretation:" << std::endl;
        std::cout << "  x >> " << i << " = " << x << " / 2^" << i << " = " << (x >> i) << std::endl;
        std::cout << "  (integer division, remainder lost)" << std::endl;
        
        waitForEnter();
        delay(200);
    }
}

// ============ Simple Animation ============

void showShrSimple(uint32_t x, int n) {
    std::string s = std::to_string(n);
    if (s.length() == 1) s = " " + s;
    
    for (int i = 1; i <= n; i++) {
        clearScreen();
        
        uint32_t result = shr(i, x);
        
        std::cout << "      x: " << bits(x) << std::endl;
        std::cout << " SHR " << s << ": " << bits(result) << std::endl;
        
        waitForEnter();
        delay(100);
    }
    delay(400);
}

// ============ Main ============

int main(int argc, char* argv[]) {
    // Check for interactive mode
    if (argc >= 2 && std::string(argv[1]) == "--interactive") {
        interactiveMode();
        return 0;
    }
    
    uint32_t x = 0b11111111000000001111111100000000; // default
    int n = 32; // default
    
    // Parse command line arguments
    if (argc >= 2) {
        std::string x_str = argv[1];
        
        // Check for hex prefix
        if (x_str.substr(0, 2) == "0x") {
            x = std::stoul(x_str, nullptr, 16);
        } else {
            // Binary input
            if (x_str.substr(0, 2) == "0b") {
                x_str = x_str.substr(2);
            }
            
            // Check bit length
            if (x_str.length() > 32) {
                std::cout << "We only operate on 32-bit words in SHA-256. ";
                std::cout << "Your x is " << x_str.length() << " bits." << std::endl;
                return 1;
            }
            
            // Pad with leading zeros if needed
            if (x_str.length() < 32) {
                x_str = std::string(32 - x_str.length(), '0') + x_str;
            }
            
            x = std::bitset<32>(x_str).to_ulong();
        }
    }
    
    if (argc >= 3) {
        n = std::stoi(argv[2]);
        if (n > 32) n = 32;
        if (n < 1) n = 1;
    }
    
    if (argc >= 4) {
        g_delay = argv[3];
    }
    
    // Show title
    clearScreen();
    std::cout << "========================================" << std::endl;
    std::cout << "Shift Right (SHR) Visualization" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "x = " << bitsWithSpaces(x) << " (0x" << hex(x) << ")" << std::endl;
    std::cout << "Showing shifts 1 through " << n << std::endl;
    std::cout << std::endl;
    
    if (g_delay == "enter") {
        std::cout << "Press Enter to step through each shift." << std::endl;
        std::cin.get();
    } else {
        std::cout << "Starting animation in 2 seconds..." << std::endl;
        delay(2000);
    }
    
    // Run animation with detailed first few steps
    int detailed = std::min(5, n);
    
    // Detailed steps with explanation
    for (int i = 1; i <= detailed; i++) {
        clearScreen();
        
        uint32_t result = shr(i, x);
        
        std::cout << "========================================" << std::endl;
        std::cout << "Shift Right (SHR) - Step " << i << " of " << n << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;
        
        std::cout << "SHR(" << i << ", x) = x >> " << i << std::endl;
        std::cout << std::endl;
        
        std::string x_bits = bits(x);
        std::string result_bits = bits(result);
        
        // Show original with visual guide
        std::cout << "Original: ";
        for (int j = 0; j < 32; j++) {
            if (j < 32 - i) {
                std::cout << x_bits[j];
            } else {
                std::cout << "\033[31m" << x_bits[j] << "\033[0m"; // Red for bits to be lost
            }
            if ((j + 1) % 8 == 0) std::cout << " ";
        }
        std::cout << std::endl;
        
        // Show shift arrow
        std::cout << "          ";
        for (int j = 0; j < 32 - i; j++) {
            std::cout << "↓";
            if ((j + 1) % 8 == 0 && j < 31) std::cout << " ";
        }
        std::cout << "  ← shift " << i << std::endl;
        
        // Show result
        std::cout << "Result:   ";
        for (int j = 0; j < 32; j++) {
            if (j < i) {
                std::cout << "\033[32m0\033[0m"; // Green for new zeros
            } else {
                std::cout << result_bits[j];
            }
            if ((j + 1) % 8 == 0) std::cout << " ";
        }
        std::cout << std::endl;
        
        std::cout << "\nBits lost: ";
        for (int j = 32 - i; j < 32; j++) {
            std::cout << "\033[31m" << x_bits[j] << "\033[0m";
        }
        std::cout << " (" << i << " LSB" << (i == 1 ? "" : "s") << ")" << std::endl;
        
        std::cout << "New zeros: " << i << " inserted at MSB" << std::endl;
        
        waitForEnter();
        delay(200);
    }
    
    // Quick animation for remaining steps
    if (n > detailed) {
        for (int i = detailed + 1; i <= n; i++) {
            clearScreen();
            
            uint32_t result = shr(i, x);
            
            std::string s = std::to_string(i);
            if (s.length() == 1) s = " " + s;
            
            std::cout << "      x: " << bits(x) << std::endl;
            std::cout << " SHR " << s << ": " << bits(result) << std::endl;
            
            // Show simple visual
            std::cout << "\nShifted right by " << i << ", lost " << i << " LSBs" << std::endl;
            
            waitForEnter();
            delay(100);
        }
    }
    
    // Final summary
    clearScreen();
    std::cout << "========================================" << std::endl;
    std::cout << "SHR - Final Summary" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Original x: " << bitsWithSpaces(x) << " (0x" << hex(x) << ")" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Shift | Result (binary)                          | Result (hex)" << std::endl;
    std::cout << "------|------------------------------------------|-------------" << std::endl;
    
    for (int i = 1; i <= std::min(8, n); i++) {
        uint32_t result = shr(i, x);
        std::cout << "  " << std::setw(2) << i << "  | " << bits(result) << " | 0x" << hex(result) << std::endl;
    }
    
    if (n > 8) {
        std::cout << "  ... | ...                                      | ..." << std::endl;
        uint32_t result = shr(n, x);
        std::cout << "  " << std::setw(2) << n << "  | " << bits(result) << " | 0x" << hex(result) << std::endl;
    }
    
    std::cout << std::endl;
    delay(1000);
    
    return 0;
}
