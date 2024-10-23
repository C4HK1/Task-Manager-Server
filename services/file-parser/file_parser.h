#pragma once

#include <string>
#include <sys/types.h>
#include <fstream>

enum FILE_PARSER_EXECUTION_STATUS {
    FILE_PARSER_COMPLETED_SUCCESSFULY,
    FILE_PARSER_FAILED,

    FILE_PARSER_FILE_NOT_FOUND,
    FILE_PARSER_READING_DATA_ERROR,
    FILE_PARSER_WRITING_DATA_ERROR,
};

constexpr size_t FILE_PARSER_SHIFT{3};

class file_parser
{
    static auto get_file_length(std::fstream &input_file) -> u_int64_t;
public:
    static auto read_file_to_string(const std::string& path, std::string &buffer) -> FILE_PARSER_EXECUTION_STATUS;
    static auto write_string_to_file(const std::string& path, std::string &buffer) -> FILE_PARSER_EXECUTION_STATUS;
};