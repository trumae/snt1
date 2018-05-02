#ifndef __SESSION__
#define __SESSION__

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

class Session
        : public std::enable_shared_from_this<Session>{
public:
    Session(boost::asio::ip::tcp::socket socket);
    void start();
private:
    std::string create_filename(std::string dir);
    void create_dir_if_not_exist(std::string dir);
    void open_file();
    void handle_line();
    void do_read();
    void send_getcontent();
    void send_ack();
    void send_fail();
    void fill_msgfilenames();

    boost::asio::ip::tcp::socket socket_;
    enum { max_length = 1024 };
    enum {
        init,
        get,        
        put,
        putcontent
    } status_;
    char data_[max_length];
    std::string line_;
    size_t filesize_;
    size_t pos_;
    int id_;
    std::ofstream output_;
    std::string fname_;

    std::vector<std::string> msgfilenames_;
    std::size_t getindex_;
    std::ifstream fmsg_;
};

#endif
