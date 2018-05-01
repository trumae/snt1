#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <boost/asio.hpp>

using namespace std;
using boost::asio::ip::tcp;

enum { max_length = 1024 };

const string server = "localhost";
const string port = "1234";

int main(int argc, char* argv[]){
    try{
        if (argc != 3) {
            std::cerr << "Usage: client <id> <path to file>\n";
            return 1;
        }

        string id = argv[1];
        string filename = argv[2];

        // open file for reading
        ifstream input(filename, std::ios::binary);

        if (!input.is_open()) {
            cout << "Failed to open " << filename << endl;
            return 1;
        }

        boost::asio::io_service io_service;
        tcp::socket s(io_service);
        tcp::resolver resolver(io_service);
        boost::asio::connect(s, resolver.resolve({server, port}));


        char a;
        string line;
        while(input.read(&a, 1)) {
            line += a;
            if (a == '\n') {
                size_t line_length = line.length();
                size_t write_lenght = boost::asio::write(s, boost::asio::buffer(line, line_length));
                cout << line ;
                line = "";
            }
        }

       /* for (std::array<char, max_length + 1> a; input.getline(&a[0], max_length); ) {
            string line = &a[0];
            line += '\n';
            size_t line_length = line.length();

            size_t write_lenght = boost::asio::write(s, boost::asio::buffer(line, line_length));

            cout << line_length << " " << write_lenght << "|" << line ;
        }*/

/*        char reply[max_length];
        size_t reply_length = boost::asio::read(s, boost::asio::buffer(reply, request_length));
        std::cout << "Reply is: ";
        std::cout.write(reply, reply_length);
        std::cout << "\n";
*/
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
