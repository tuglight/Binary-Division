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
void restoreMethod(uint64_t dividend, uint32_t divisor, int wsize, int& ops, uint32_t& quotient, uint32_t& remainder) {
    uint32_t leftHalf = dividend >> wsize;
    uint32_t compdivisor = (~divisor & static_cast<uint32_t>(pow(2, wsize + 1) - 1)) + 1;
    uint64_t aligndivisor = divisor << wsize;
    uint64_t alignCompDivisor = compdivisor << wsize;

    if(leftHalf >= divisor)
        throw std::string("divide overflow");

    for(int i = 0; i < wsize; i++) {
        dividend &= static_cast<uint64_t>(pow(2, wsize *2 + 1) - 1);
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
void nonRestoreMethod(uint64_t dividend, uint32_t divisor, int wsize, int& ops, uint32_t& quotient, uint32_t& remainder) {
    uint32_t leftHalf = dividend >> wsize;
    uint32_t compdivisor = (~divisor & static_cast<uint32_t>(pow(2, wsize + 1) - 1)) + 1;
    uint64_t aligndivisor = divisor << wsize;
    uint64_t alignCompDivisor = compdivisor << wsize;
    uint64_t ebit;

    if(leftHalf >= divisor)
        throw std::string("divide overflow");

    dividend <<= 1;
    dividend += alignCompDivisor;
    dividend &= static_cast<uint64_t>(pow(2, wsize *2 +1) - 1);

    for(int i = 0; i < wsize; i++) {
        ebit = (dividend >> (2 * wsize)) & 1;

        dividend &= static_cast<uint64_t>(pow(2, wsize *2 +1) - 1);
        dividend <<= 1;

        if(ebit == 0) {
            dividend += alignCompDivisor;
            dividend |= 1;
            ops++;
        } else {
            dividend += aligndivisor;
            dividend |= 0;
            ops++;
        }
    }

    ebit = (dividend >> (2 * wsize)) & 1;
    if(ebit == 1) {
        dividend += aligndivisor;
    }

    dividend &= static_cast<uint64_t>(pow(2, wsize *2 + 1) - 1);
    quotient = dividend & static_cast<uint32_t>(pow(2, wsize) - 1);
    remainder = (dividend >> wsize) & static_cast<uint32_t>(pow(2, wsize) - 1);
}

int main(int argc, char* argv[]) {
    std::string b1, b2;
    uint64_t u1;
    uint32_t u2, remainder, quotient;
    high_resolution_clock::time_point p1, p2;
    microseconds result;
    int32_t ops;

    try {
          // input file
          std::fstream in("input.txt", std::fstream::in);
          if(!in.is_open())
              throw std::string("input.txt does not exist.");

          // output for add and shift
          std::fstream out("restore_method.csv", std::fstream::out);
          if(!out.is_open())
              throw std::string("Cannot open the results.csv file.");

          // add and shift
          out << "dividend,divisor,quotient,remainder,length,time,iterations,ops"
              << std::endl;

          while(!in.eof()) {
             in >> b1;
             in >> b2;

             u1 = binaryToValue(b1);
             u2 = binaryToValue(b2);
             assert(9 <= b1.size() && b1.size() <= 25);
             assert(5 <= b2.size() && b2.size() <= 13);

             try {
                 p1 = high_resolution_clock::now();
                 for(int i = 0; i < 100000; i++) {
                    ops = 0;
                    restoreMethod(u1, u2, b2.size() - 1, ops, quotient, remainder);
                 }
                 p2 = high_resolution_clock::now();
                 result = duration_cast<microseconds>(p2 - p1);
                 out << b1 << "," << b2 << "," << quotient << ',' << remainder
                     << "," << b1.size() << "," << result.count() << ","
                     << b1.size() + 1 << "," << ops << std::endl;
             } catch(std::string s) {
                 out << b1 << "," << b2 << "," << "overflow, overflow"
                     << "," << b1.size() << ",null,"
                     << b1.size() + 1 << ",null" << std::endl;
             }

          }

          in.close();
          in.open("input.txt", std::fstream::in);
          if(!in.is_open())
              throw std::string("input.txt does not exist.");

          out.close();
          out.open("non_restore_method.csv", std::fstream::out);
          if(!out.is_open())
              throw std::string("Cannot open the results.csv file.");

          out << "dividend,divisor,quotient,remainder,length,time,iterations,ops"
              << std::endl;

          while(!in.eof()) {
             in >> b1;
             in >> b2;
             u1 = binaryToValue(b1);
             u2 = binaryToValue(b2);
             assert(9 <= b1.size() && b1.size() <= 25);
             assert(5 <= b2.size() && b2.size() <= 13);

             try {
                 p1 = high_resolution_clock::now();
                 for(int i = 0; i < 100000; i++) {
                    ops = 0;
                    nonRestoreMethod(u1, u2, b2.size() - 1, ops, quotient, remainder);
                 }
                 p2 = high_resolution_clock::now();
                 result = duration_cast<microseconds>(p2 - p1);
                 out << b1 << "," << b2 << "," << quotient << ',' << remainder
                     << "," << b1.size() << "," << result.count() << ","
                     << b1.size() + 1 << "," << ops << std::endl;
             } catch(std::string s) {
                 out << b1 << "," << b2 << "," << "overflow, overflow"
                     << "," << b1.size() << ",null,"
                     << b1.size() + 1 << ",null" << std::endl;
             }
          }

    } catch(std::string s) {
        std::cout << "exception caught\n" << std::endl;
        std::cout << s << std::endl;
        return 1;
    }

    return 0;
}
