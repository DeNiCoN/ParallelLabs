#include <iostream>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "common.hpp"
#include <fmt/format.h>

using boost::asio::ip::tcp;
using namespace std;
using namespace nlohmann;

double sendRequest(const Request& request) {
    fmt::print("Connecting to {}:{}\n", HOST, PORT);
    boost::asio::ip::tcp::iostream server_stream(HOST, PORT);
    fmt::print("Successfull\n");

    fmt::print("Sending the serialized data\n");
    {
        boost::archive::binary_oarchive oa(server_stream);
        oa << request;
    }

    fmt::print("Reading the response\n");
    Response response;
    {
        boost::archive::binary_iarchive ia(server_stream);
        ia >> response;
    }

    return response.number;
}

int main(int argc, char *argv[]) {

    json array_json;
    std::cout << "Enter an array as json([1, 2, ...]):" << std::endl;
    std::cin >> array_json;
    auto array = array_json.get<std::vector<double>>();

    std::string operation;
    std::cout << "Enter the operation(min, max, average):" << std::endl;
    std::cin >> operation;

    auto result = sendRequest({
            Operation::from_string(operation),
            array
        });
    std::cout << "Result: " << result << std::endl;
    return 0;
}
