#ifndef POLLARD_S_RHO_ALGORITHM_FACTORIZATOR_H
#define POLLARD_S_RHO_ALGORITHM_FACTORIZATOR_H

#include <cstdint>
#include <vector>
#include <string>

class Factorizator {
public:

    explicit Factorizator(const uint64_t number);

    const uint64_t getNumber() const;

    const std::vector<std::pair<short, uint32_t> > &getFactors() const;

    const std::vector<std::pair<short, uint32_t>> getFactors();

    const std::string toString() const;

    const std::string toReducedString() const;

    bool isCorrectFactorization() const;

    void factorization();


private:
    uint64_t number;
    std::vector<std::pair<short, uint32_t>> factors;

    uint64_t gcd(uint64_t a, uint64_t b);

    bool naiveFactorization(uint64_t num);

    void pollardFactorization(uint64_t& num);

    void optimization();
};

#endif //POLLARD_S_RHO_ALGORITHM_FACTORIZATOR_H
