#include <sstream>
#include <algorithm>
#include <random>
#include <climits>
#include <iostream>
#include "Factorizator.h"

Factorizator::Factorizator(const uint64_t number) : number(number) {}

const uint64_t Factorizator::getNumber() const {
    return number;
}

const std::vector<std::pair<short, uint32_t>> &Factorizator::getFactors() const {
    return factors;
}

const std::vector<std::pair<short, uint32_t>> Factorizator::getFactors() {
    return factors;
}

const std::string Factorizator::toString() const {
    std::string exp;
    std::ostringstream in;
    in << "1";
    if (factors[0].first == 0 && factors[0].second == 1) {
        in << " * " << number;
    }
    for (const auto &factor : factors) {
        for (int i = 0; i < factor.first; ++i) {
            in << " * " << factor.second;
        }
    }

    in << " = " << number;
    exp.append(in.str());
    return exp;
}

const std::string Factorizator::toReducedString() const {
	std::string exp;
	std::ostringstream in;
	in << "1";
	if (factors[0].first == 0 && factors[0].second == 1) {
		in << " * " << number;
	}
	for (const auto &factor : factors) {
		in << " * " << factor.second << "^(" << factor.first << ")";		
	}
	in << " = " << number;
	exp.append(in.str());
	return exp;
}

bool Factorizator::isCorrectFactorization() const {
    uint64_t res = 1;
    for (const auto &factor: factors) {
        for (int i = 0; i < factor.first; ++i) {
            res *= factor.second;
        }
    }
    return res == number;
}

/**
 * Use Pollard factorization algorithm while
 * the number greater than INT_MAX
 */
void Factorizator::factorization() {
    uint64_t temp = number;
    while (temp > INT_MAX) {
        pollardFactorization(temp);
    }
    naiveFactorization(temp);
    optimization();
}

/**
 * Just a O(sqrt(num)) algorithm
 * Return true if num is even, false overwise
 */
bool Factorizator::naiveFactorization(uint64_t num) {
    auto temp = static_cast<int>(num);
    for (uint32_t i = 2; i * i <= temp; ++i) {
        if (num % i == 0) {
            factors.emplace_back(0, i);
            while (num % i == 0) {
                num /= i;
                factors[factors.size() - 1].first++;
            }
        }
    }
    if (num == 0 || num > 1) {
        factors.emplace_back(std::make_pair(1, num));
        return false;
    }    
    return true;
}

uint64_t Factorizator::gcd(uint64_t a, uint64_t b) {
    uint64_t remainder;
    while (b != 0) {
        remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

/*
 * Finds one divisor, but it may not be a prime number
 * So, we need the optimization() function
 */
void Factorizator::pollardFactorization(uint64_t& num) {
    std::uniform_int_distribution<> uni(0, INT_MAX);
    std::random_device rd;
    std::mt19937 gen(rd());
    uint64_t x_fixed = 2, cycle_size = 2, x = uni(gen) % num, factor = 1;

    while (factor == 1) {
        for (int count = 1; count <= cycle_size && factor <= 1; ++count) {
            x = ((x * x + 1) % num);
            factor = gcd(x - x_fixed, num);
        }

        cycle_size *= 2;
        x_fixed = x;
    }
    if (factor < UINT_MAX) {
        if (!factors.empty() && factors.back().second == factor) {
            factors.back().first++;
        } else {
            factors.emplace_back(std::make_pair(1, factor));
        }
    } else {
	    // I store divisors in uint32_t variables, which capacity are [0; 2^32 -1]
	    // But the number may not be a prime and be greate than uint32_t capacity
	    // Thus, I use some crutch to deal with it))
	    // The program interprets this pair as the input number itself
        factors.emplace_back(std::make_pair(0, 1));
    }
    num /= factor;
}

void Factorizator::optimization() {
    int i = 0;
    size_t vectorSize = factors.size();
	// Checks if there are not prime numbers in the factors list
    while (naiveFactorization(static_cast<uint64_t>(factors[i].second)) && i < factors.size()) {
        for (size_t j = factors.size() - 1; j > vectorSize; --j) {
            factors[j].first *= factors[i].first;
        }
        vectorSize = factors.size();
        i++;
    }
    factors.back().first *= factors[i].first;
    factors.erase(factors.begin(), factors.begin() + i + 1);
    std::sort(factors.begin(), factors.end(), [](const std::pair<short, uint32_t>& a,
		const std::pair<short, uint32_t>& b) {
        return a.second < b.second;
    });
	// Union all of identical divisors
    auto it = factors.begin();
    while (it != factors.end() - 1)
        if (it->second == (it + 1)->second) {
            (it + 1)->first += it->first;
            it = factors.erase(it);
        }
        else {
            ++it;
        }
}
