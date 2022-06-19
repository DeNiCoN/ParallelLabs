#pragma once
#include <boost/serialization/vector.hpp>

const char* HOST = "localhost";
const char* PORT = "8765";

namespace Operation {
    enum Operation {
        MIN,
        MAX,
        AVERAGE,
    };

    inline Operation from_string(const std::string& str) {
        if (str == "min")
            return MIN;
        if (str == "max")
            return MAX;
        if (str == "average")
            return AVERAGE;

        throw std::runtime_error("Unknown operation " + str);
    };
};

struct Request {
    Operation::Operation operation;
    std::vector<double> numbers;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & operation;
        ar & numbers;
    }
};

struct Response {
    double number;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & number;
    }
};
