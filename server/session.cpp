#include "session.h"

#define DEBUG false

using boost::asio::ip::tcp;
using namespace std;
namespace fs = boost::filesystem;

const string datadir = "data";


Session::Session(tcp::socket socket)
    : socket_(std::move(socket)), status_(init), pos_(0){}

void Session::start() {
    line_ = "";
    create_dir_if_not_exist(datadir);
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
        output_ << line_ << "\n";

        if (pos_ >= filesize_) {
            if (DEBUG) cout << "SENDING ACK " << pos_ << endl;
            send_ack();
        }
        break;
    case get:
        fill_msgfilenames();
        send_getcontent();
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

void Session::send_ack(){
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer("A", 1),
                             [this, self](boost::system::error_code ec, std::size_t){
        if (!ec){
            //cout << ec << " " << t << endl;
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

void Session::fill_msgfilenames() {
    std::vector<int> vdir;

    for (auto x : fs::directory_iterator(datadir)) {
        vdir.push_back(std::stoi(x.path().filename().string()));
    }
    std::sort(vdir.begin(), vdir.end());

    for(auto d: vdir) {
        std::vector<int> vtmp;

        for (auto f : fs::directory_iterator(datadir + fs::path::preferred_separator + std::to_string(d))) {
            vtmp.push_back(std::stoi(f.path().filename().string()));
        }
        std::sort(vtmp.begin(), vtmp.end());


        for (int i: vtmp) {
            msgfilenames_.push_back(datadir + fs::path::preferred_separator + std::to_string(d) +
                                    fs::path::preferred_separator + std::to_string(i));
        }
    }
    getindex_ = 0;
}

void Session::send_getcontent(){
    auto self(shared_from_this());

    if (fmsg_.eof()) fmsg_.close();

    if (!fmsg_.is_open()) {
        if( getindex_ < msgfilenames_.size() ) {
            ///cout << "Open File: " << msgfilenames_[getindex_] << endl;
            fmsg_.open(msgfilenames_[getindex_], std::ios::binary);
            getindex_ ++;
        } else {
            socket_.close();
            return;
        }
    }

    char a;
    string line;
    while(fmsg_.read(&a, 1)) {
        line += a;
        if (a == '\n') break;
    }
    size_t line_length = line.length();

    boost::asio::async_write(socket_, boost::asio::buffer(line, line_length),
                             [this, self](boost::system::error_code ec, std::size_t){
        if (!ec){
            send_getcontent();
        }
    });
}

