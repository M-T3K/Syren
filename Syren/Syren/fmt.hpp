#pragma once

#include <stdexcept>
#include <stdarg.h>
#include <iostream>
#include <ios>
#include <fstream>

// Single Header formatting library inspired by Go's fmt.

namespace fmt {

    // Structs



    // Functions

    auto fprint(const char *file, char const *fmt, ...) -> int {

        char msg[2048];
        
        std::ofstream f;
        va_list va;
        va_start(va, fmt);
        vsnprintf_s(msg, sizeof(msg), fmt, va);
        va_end(va);
        
        f.open(file, std::ios::out | std::ios::app);       

        f << msg;

        return strlen(msg);
    }

    auto fprint(std::fstream& f, char const *fmt, ...) -> int {

        char msg[2048];
		va_list va;
        va_start(va, fmt);
        vsnprintf_s(msg, sizeof(msg), fmt, va);
        va_end(va);

        f << msg;

        return strlen(msg);
    }

    auto fprintLn(const char *file, char const *fmt, ...) -> int {

        char msg[2048];
        
        std::ofstream f;
        va_list va;
        va_start(va, fmt);
        vsnprintf_s(msg, sizeof(msg), fmt, va);
        va_end(va);
        
        f.open(file, std::ios::out | std::ios::app);       

        f << msg << std::endl;
        f.close();

        return strlen(msg);
    }

    auto fprintLn(std::fstream& f, char const *fmt, ...) -> int {

        char msg[2048];
		va_list va;
        va_start(va, fmt);
        vsnprintf_s(msg, sizeof(msg), fmt, va);
        va_end(va);

        f << msg << std::endl;

        return strlen(msg);
    }

    // Classes

    class error: std::exception {

        char err_msg[2048]; // This buffer size should be more than enough
    
    public:
    
        error(char const *fmt, ...) {

            va_list va;
            va_start(va, fmt);
            vsnprintf_s(err_msg, sizeof(err_msg), fmt, va);
            va_end(va);
        }

        char const *what() const throw() {

            return err_msg;
        }

	};

}
