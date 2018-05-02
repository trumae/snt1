#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/filesystem.hpp>
#include <sstream>

#define DEBUG true

using boost::asio::ip::tcp;
using namespace std;
namespace fs = boost::filesystem;

const string datadir = "data";

class session
        : public std::enable_shared_from_this<session>{
public:
    session(tcp::socket socket)
        : socket_(std::move(socket)), status_(init), pos_(0){}

    void start() {
        line_ = "";
        do_read();
    }

private:
    string create_filename(string dir) {
        int i = 0;

        for (auto x : fs::directory_iterator(dir)) i++;

        return std::to_string(i + 1);
    }

    void create_dir_if_not_exist(string dir)
    {
        if (!fs::is_directory(dir)) {
            bool ok = fs::create_directory(dir);
            if (!ok) {
                cout << "Fail creating directory - " << dir;
            }
        }
    }

    void open_file()
    {
        create_dir_if_not_exist(datadir);

        string dir = datadir + fs::path::preferred_separator + std::to_string(id_);
        create_dir_if_not_exist(dir);

        fname_ =  dir + fs::path::preferred_separator + create_filename(dir);
        output_.open(fname_ , std::ios::binary);
    }

    void handle_line() {
        stringstream ss;
        switch(status_) {
        case init:
            if (line_ == "P") status_ = put;
            if (line_ == "G") status_ = get;
            if (line_ != "P" && line_ != "G") {
                socket_.close();
            }
            break;
        case put:
            ss.str(line_);
            ss >> id_ >> filesize_;
            open_file();
            status_ = putcontent;

            break;
        case putcontent:
            pos_ += line_.length() + 1;

            //process content
            output_ << line_;

            if (pos_ >= filesize_) {
                if (DEBUG) cout << "SENDING ACK " << pos_ << endl;
                send_ack();
            }
            break;
        case get:
            break;
        }

        line_ = "";
    }

    void do_read()   {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                [this, self](boost::system::error_code ec, std::size_t length){
            if (!ec){
                for(std::size_t i = 0; i < length; i++) {
                    char a = data_[i];
                    if (a == '\0') {
                        break;
                    }
                    if (a == '\n') {
                        handle_line();
                        continue;
                    }
                    line_ += a;
                }

                do_read();
            }
        });
    }

    void send_ack(){
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer("A", 1),
                                 [this, self](boost::system::error_code ec, std::size_t t){
            if (!ec){
                cout << ec << " " << t << endl;
                output_.close();
            }
        });
    }

    void send_fail(){
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer("F", 1),
                                 [this, self](boost::system::error_code ec, std::size_t){
            if (!ec){
                output_.close();
                fs::remove(fname_);
            }
        });
    }

    tcp::socket socket_;
    enum { max_length = 1024 };
    enum {
        init,
        get,
        put,
        putcontent
    } status_;
    char data_[max_length];
    string line_;
    size_t filesize_;
    size_t pos_;
    int id_;
    ofstream output_;
    string fname_;
};

class server {
public:
    server(boost::asio::io_service& io_service, short port)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
          socket_(io_service) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(socket_,
                               [this](boost::system::error_code ec){
            if (!ec) {
                std::make_shared<session>(std::move(socket_))->start();
            }

            do_accept();
        });
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
};

int main(){
    try {
        boost::asio::io_service io_service;

        server s(io_service, 1234);

        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
