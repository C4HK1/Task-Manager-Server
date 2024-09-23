#pragma once

#include <string>
#include <sys/types.h>
#include <fstream>

class text_file_parser
{
    static auto get_file_length(std::ifstream &input_file) -> u_int64_t;
public:
    static auto read_file_to_string(const std::string &path) -> std::string;
};