/**
 * @file std.h
 * @brief Classes that are a part of the standard library (std)
 */

#pragma once

#include "types.h"
#include <string>
#include <set>
#include "stdio.h"

struct StringLengthCompare
{
    bool operator () (const std::string & p_lhs, const std::string & p_rhs)
    {
        const size_t lhsLength = p_lhs.length() ;
        const size_t rhsLength = p_rhs.length() ;

        if(lhsLength == rhsLength)
        {
            return (p_lhs > p_rhs) ; // when two strings have the same
                                     // length, defaults to the normal
                                     // string comparison
        }

        return (lhsLength > rhsLength) ; // compares with the length
    }
} ;

namespace std
{
    template <typename I> std::string num2hexstr(I w, size_t hex_len = sizeof(I)<<1) {
        static const char* digits = "0123456789ABCDEF";
        std::string rc(hex_len,'0');
        for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
            rc[i] = digits[(w>>j) & 0x0f];
        return rc;
    };
        
    struct __va_list;


    namespace __l
    {
        template<typename T, typename T2>
        class __tree_node;
        
        template<typename T, typename T2>
        class list
        {
        public:
            list(T const &);
        };

        template<typename T, typename T2>
        class pair
        {
        public:
            ~pair();
        };  

        template<typename T, typename T2>
        class vector
        {
        public:
            void reserve(u64);

            void __push_back_slow_path(T const &);
        };

        template<typename T, typename T2, typename T3>
        class __tree
        {
        public:
            void destroy(std::__l::__tree_node<T, void *> *);
        };

        template<typename T, typename T2>
        void __sort(T2, T2, T2, T);

        template<typename T, typename T2>
        void __sort3(T2, T2, T);

        template<typename T, typename T2>
        void __sort5(T2, T2, T2, T2, T2, T);

        template<typename T, typename T2>
        void __insertion_sort_incomplete(T2, T2, T);
    }; 
};