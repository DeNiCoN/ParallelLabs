#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "common.hpp"
#include <ranges>
#include <algorithm>
#include <numeric>
#include <fmt/format.h>

using boost::asio::ip::tcp;
using namespace boost::asio;

Response process_request(const Request& request)
{
    Response result;
    if (request.operation == Operation::MIN)
    {
        result.number = *std::ranges::min_element(request.numbers);
    }
    else if (request.operation == Operation::MAX)
    {
        result.number = *std::ranges::max_element(request.numbers);
    }
    else if (request.operation == Operation::AVERAGE)
    {
        auto const count = static_cast<double>(request.numbers.size());
        result.number = std::reduce(request.numbers.begin(),
                                    request.numbers.end(), 0) / count;
    }
    return result;
}

int main(int argc, char* argv[])
{
    boost::asio::io_context ioc;
    ip::tcp::endpoint endpoint(tcp::v4(), 8765);
    ip::tcp::acceptor acceptor(ioc, endpoint);

    for (;;)
    {
        fmt::print("Listening\n");
        ip::tcp::iostream stream;
        acceptor.accept(stream.socket());
        fmt::print("Client connected. Waiting for request\n");

        Request request;
        {
            boost::archive::binary_iarchive ia(stream);
            ia >> request;
        }

        fmt::print("Responding\n");
        auto response = process_request(request);
        {
            boost::archive::binary_oarchive oa(stream);
            oa << response;
        }
    }

    return 0;
}
