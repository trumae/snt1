#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#define DEBUG false

using namespace std;
using boost::asio::ip::tcp;

enum { max_length = 1024 };

const string server = "localhost";
const string port = "1234";

void put_text(string id, string filename)
{
    ifstream input(filename, std::ios::binary);

    if (!input.is_open()) {
        cout << "Failed to open " << filename << endl;
        exit(1);
    }

    boost::asio::io_service io_service;
    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({server, port}));

    boost::asio::write(s, boost::asio::buffer("P\n", 2));

    // send header
    stringstream ss;
    ss << id << " " << boost::filesystem::file_size(filename) << endl;
    string header = ss.str();
    boost::asio::write(s, boost::asio::buffer(header, header.length()));

    // send content
    char a;
    string line;
    while(input.read(&a, 1)) {
        line += a;
        if (a == '\n') {
            size_t line_length = line.length();
            size_t write_lenght = boost::asio::write(s, boost::asio::buffer(line, line_length));
            if (DEBUG) cout << write_lenght << "|" << line ;
            line = "";
        }
    }

    cout <<  "Waiting ack" << endl;
    char reply;
    boost::asio::read(s, boost::asio::buffer(&reply, 1));
    cout << "OK - Success" << endl;

    exit(0);
}

void get_text() {
    cout << "getText" << endl;
    exit(0);
}

void usage() {
    cout << "Usage: client [put|get] <option>" << endl;
    cout << " commands:" << endl;
    cout << "   put <id> <path to text file>     put text to server" << endl;
    cout << "   get                              get text from server" << endl;
    exit(1);
}

int main(int argc, char* argv[]){
    try{
        if (argc == 2) {
            string cmd = argv[1];
            if (cmd == "get") {
                get_text();
            }
        }

        if (argc == 4) {
            string cmd = argv[1];
            if (cmd == "put") {
                string id = argv[2];
                string filename = argv[3];

                // open file for reading
                put_text(id, filename);
            }
        }

        usage();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
