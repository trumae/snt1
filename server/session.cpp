#include "session.h"

using boost::asio::ip::tcp;
using namespace std;
namespace fs = boost::filesystem;

const string datadir = "data";


Session::Session(tcp::socket socket)
    : socket_(std::move(socket)), status_(init), pos_(0){}

void Session::start() {
    line_ = "";
    do_read();
}

string Session::create_filename(string dir) {
    int i = 0;

    for (auto x : fs::directory_iterator(dir)) i++;

    return std::to_string(i + 1);
}

void Session::create_dir_if_not_exist(string dir) {
    if (!fs::is_directory(dir)) {
        bool ok = fs::create_directory(dir);
        if (!ok) {
            cout << "Fail creating directory - " << dir;
        }
    }
}

void Session::open_file()
{
    create_dir_if_not_exist(datadir);

    string dir = datadir + fs::path::preferred_separator + std::to_string(id_);
    create_dir_if_not_exist(dir);

    fname_ =  dir + fs::path::preferred_separator + create_filename(dir);
    output_.open(fname_ , std::ios::binary);
}


void Session::handle_line() {
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

void Session::do_read()   {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            [this, self](boost::system::error_code ec, std::size_t length){
        if (!ec){
            for(std::size_t i = 0; i < length; i++) {
                char a = data_[i];
                if (a == '\0') {
                    break;
                }
                line_ += a;
                if (a == '\n') {
                    handle_line();
                    continue;
                }                
            }

            do_read();
        }
    });
}

void Session::send_ack(){
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer("A", 1),
                             [this, self](boost::system::error_code ec, std::size_t t){
        if (!ec){
            cout << ec << " " << t << endl;
            output_.close();
        }
    });
}

void Session::send_fail(){
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer("F", 1),
                             [this, self](boost::system::error_code ec, std::size_t){
        if (!ec){
            output_.close();
            fs::remove(fname_);
        }
    });
}

