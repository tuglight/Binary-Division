#include <iostream>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <cassert>
#include <fstream>

using namespace std::chrono;

/**
 * @name - binaryToValue(std::string s)
 * @desc - This function takes in the binary value of a number in a string
 *         and returns the number value as an unsigned 16 bit integer
 * @param("s") - the binary number represented as a string
 * @return - the integral value of the string represented binary
 */
uint64_t binaryToValue(std::string s) {
    uint64_t ret = 0;
    for(uint32_t i = s.size(); i > 0; i--) {
        if(s[i - 1] == '1') {
            ret += std::pow(2, s.size() - i);
        } else if(s[i - 1] == '0') {
            continue;
        } else {
            throw std::string("Invalid format for binary operand input.");
        }
    }
    return ret;
}

/**
 * @name - template<class T> valueToBinary(T val)
 * @desc - This function takes in the integral value of a number and returns the
 *     binary representation in string format.
 * @param("val") - the integral value to be converted to binary in the form of a
 *     string
 * @return - the string representation of the numerical value in binary
 */
template<class T>
std::string valueToBinary(T val) {
    std::string out;
    for(int i = sizeof(T) * 8; i >= 0; i--) {
        if(val/static_cast<double>(pow(2, i)) >= 1) {
            val -= pow(2, i);
            out += "1";
        } else {
            out += "0";
        }
    }
    return out;
}

/**
 * @name - std::string snip(std::string s, char delim)
 * @desc - Takes the string s and takes every instance of delim off of the
 *         beginning of the string.
 * @param("s") - the string to snip any number of delimiter characters
 * @param("delim") - the character delimiter to snip off the front of the string
 * @return - the string with all the leading delimiters snipped off
 */
std::string snip(std::string s, char delim) {
    auto iter = s.begin();
    while(*iter == delim) {
        iter = s.erase(iter);
    }
    return s;
}

/**
 * @name - restoreMethod
 * @desc - Takes the values of the dividen and divisor and utilizes a
 *     restoring method of devision using bitwise operators and bit packing
 * @param("dividend") - the value to use as the dividend
 * @param("divisor") - the value to use as the divisor
 * @param("wsize") - generally the length of the dividend in bits
 * @param("ops") - an output that is the number of math operations
 * @param("quotient") - quotient portion of the answer after the operation
 * @param("remainder") - remainder portion of the answer after the operation
 */
void restoreMethod(int64_t dividend, int32_t divisor, int wsize, int& ops, int32_t& quotient, int32_t& remainder) {
    uint32_t sign1 = dividend >> (wsize * 2);
    uint32_t sign2 = divisor >> wsize;
    uint32_t leftHalf = (dividend >> wsize) & static_cast<uint32_t>(pow(2, wsize) - 1);
    uint32_t divisorMag = divisor & static_cast<uint32_t>(pow(2, wsize) - 1);

    if(leftHalf >= divisorMag)
        throw std::string("divide overflow");
    
    dividend &= static_cast<uint32_t>(pow(2,wsize*2) - 1);
    uint32_t compdivisor = (~divisor & static_cast<uint32_t>(pow(2, wsize + 1) - 1)) + 1;
    uint64_t aligndivisor = divisorMag << wsize;
    uint64_t alignCompDivisor = compdivisor << wsize;
    alignCompDivisor |= (1 << (wsize * 2)); // always set the ebit for the divisor to 1

    for(int i = 0; i < wsize; i++) {
        dividend &= static_cast<uint64_t>(pow(2, wsize * 2 + 1) - 1);
        dividend <<= 1;

        dividend += alignCompDivisor;
        ops++;

        uint64_t ebit = (dividend >> (2 * wsize)) & 1;
        if(ebit == 0) {
            dividend |= 1;
            continue;
        }

        dividend |= 0;
        dividend += aligndivisor;
        ops++;
    }

    dividend &= static_cast<uint64_t>(pow(2, wsize *2) - 1);
    quotient = dividend & static_cast<uint32_t>(pow(2, wsize) - 1);
    if(sign1 ^ sign2 == 1) {
        quotient = -quotient;
    }
    remainder = (dividend >> wsize) & static_cast<uint32_t>(pow(2, wsize) - 1);
}

/**
 * @name - nonRestoreMethod
 * @desc - Takes the values of the dividen and divisor and utilizes a non
 *     restoring method of devision using bitwise operators and bit packing
 * @param("dividend") - the value to use as the dividend
 * @param("divisor") - the value to use as the divisor
 * @param("wsize") - generally the length of the dividend in bits
 * @param("ops") - an output that is the number of math operations
 * @param("quotient") - quotient portion of the answer after the operation
 * @param("remainder") - remainder portion of the answer after the operation
 */
void nonRestoreMethod(int64_t dividend, int32_t divisor, int wsize, int& ops, int32_t& quotient, int32_t& remainder) {
    uint32_t sign1 = dividend >> (wsize * 2);
    uint32_t sign2 = divisor >> wsize;
    uint32_t leftHalf = (dividend >> wsize) & static_cast<uint32_t>(pow(2, wsize) - 1);
    uint32_t divisorMag = divisor & static_cast<uint32_t>(pow(2, wsize) - 1);

    if(leftHalf >= divisorMag)
        throw std::string("divide overflow");

    dividend &= static_cast<uint32_t>(pow(2, wsize*2) - 1);
    uint32_t compdivisor = (~divisor & static_cast<uint32_t>(pow(2, wsize + 1) - 1)) + 1;
    uint64_t aligndivisor = divisorMag << wsize;
    uint64_t alignCompDivisor = compdivisor << wsize;
    alignCompDivisor |= (1 << (wsize * 2)); // always set the ebit for the divisor to 1
    uint64_t ebit;

    for(int i = 0; i < wsize; i++) {
        ebit = (dividend >> (2 * wsize)) & 1;
        dividend <<= 1;
        dividend &= static_cast<uint64_t>(pow(2, wsize *2 +1) - 1);
        
        if(ebit == 0) {
            dividend += alignCompDivisor;
            ops++;
        } else {
            dividend += aligndivisor;
            ops++;
        }
        dividend &= static_cast<uint64_t>(pow(2, wsize *2 +1) - 1);
        
        ebit = (dividend >> (2 * wsize)) & 1;
        if(ebit == 0) {
            dividend |= 1;
        } else {
            dividend |= 0;
        }

    }
    
    ebit = (dividend >> (2 * wsize)) & 1;
    if(ebit == 1) {
        dividend += aligndivisor;
    }

    dividend &= static_cast<uint64_t>(pow(2, wsize *2) - 1);
    quotient = dividend & static_cast<uint32_t>(pow(2, wsize) - 1);
    if(sign1 ^ sign2 == 1) {
        quotient = -quotient;
    }
    remainder = (dividend >> wsize) & static_cast<uint32_t>(pow(2, wsize) - 1);
}

int main(int argc, char* argv[]) {
    std::string b1, b2;
    int64_t u1;
    int32_t u2, remainder, quotient;
    high_resolution_clock::time_point p1, p2;
    microseconds result;
    int32_t wsize, ops, mag1, mag2;
    char sign1, sign2;

    std::fstream restore("restore_method.csv", std::fstream::out);
    if(!restore.is_open())
      throw std::string("Cannot open the restore_method.csv file.");

    // add and shift
    restore << "dividend,divisor,quotient,remainder,iterations,ops" << std::endl;

    std::fstream nonrestore("non_restore_method.csv", std::fstream::out);
    if(!nonrestore.is_open())
      throw std::string("Cannot open the non_restore_method.csv file.");

    // add and shift
    nonrestore << "dividend,divisor,quotient,remainder,iterations,ops" << std::endl;

    while(std::cin.good()) {
        std::cin >> b1;
        std::cin >> b2;

        if(std::cin.eof()) break;
        
        u1 = binaryToValue(b1);
        u2 = binaryToValue(b2);
        assert(9 <= b1.size() && b1.size() <= 25);
        assert(5 <= b2.size() && b2.size() <= 13);
        wsize = b2.size() - 1;
        
        ops = 0;
        sign1 = b1[0];
        b1.erase(b1.begin());
        mag1 = binaryToValue(b1);

        sign2 = b2[0];
        b2.erase(b2.begin());
        mag2 = binaryToValue(b2);

        try {
            std::cout << valueToBinary(u1) << std::endl;
            restoreMethod(u1, u2, wsize, ops, quotient, remainder);
            restore << ((sign1 == '0')?mag1:-mag1) << "," << ((sign2 == '0')?mag2:-mag2) << "," << quotient << ',' << remainder
                << "," << wsize << "," << ops << std::endl;
        } catch (std::string) {
            restore << ((sign1 == '0')?mag1:-mag1) << "," << ((sign2 == '0')?mag2:-mag2) << "," << "overflow, overflow"
                << ",null,null" << std::endl;
        }

          
        ops = 0;
        try {
            nonRestoreMethod(u1, u2, wsize, ops, quotient, remainder);
            nonrestore << ((sign1 == '0')?mag1:-mag1) << "," << ((sign2 == '0')?mag2:-mag2) << "," << quotient << ',' << remainder
                << "," << wsize << "," << ops << std::endl;
        } catch(std::string s) {
            nonrestore << ((sign1 == '0')?mag1:-mag1) << "," << ((sign2 == '0')?mag2:-mag2) << "," << "overflow, overflow"
                << ",null,null" << std::endl;
        }
    }

    return 0;
}
