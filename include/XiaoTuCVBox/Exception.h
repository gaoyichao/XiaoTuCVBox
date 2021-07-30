#ifndef XTCVB_EXCEPTION_H
#define XTCVB_EXCEPTION_H

#include <exception>


namespace xiaotu {
namespace cv {

    struct Exception : public std::exception {
        Exception(std::string const & str)
            : msg(str) {}

        const char * what() const noexcept {
            return msg.c_str();
        }

        std::string msg;
    };

}
}

#endif


