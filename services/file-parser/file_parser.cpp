#include <fstream>
#include <iostream>
#include <string>
#include <sys/types.h>

#include "file_parser.h"

auto file_parser::get_file_length(std::fstream &input_file) -> u_int64_t {
    input_file.seekg ({}, input_file.end);
    auto length = input_file.tellg();
    input_file.seekg ({}, input_file.beg);

    return length;
}

auto file_parser::read_file_to_string(const std::string& path, std::string &buffer) -> FILE_PARSER_EXECUTION_STATUS {
    try {
        std::fstream input_file{path, std::ifstream::binary};

        if (input_file) {
            auto length = get_file_length(input_file);

            buffer.resize(length);
            input_file.read(&buffer[0], length);

            if (!input_file) {
                input_file.close();
                
                return FILE_PARSER_READING_DATA_ERROR;
            }
        } else {
            return FILE_PARSER_FILE_NOT_FOUND;
        }

        input_file.close();

        return FILE_PARSER_COMPLETED_SUCCESSFULY;
    } catch(std::fstream::failure &exeption) {
        return FILE_PARSER_FAILED;
    }
}

auto file_parser::write_string_to_file(const std::string& path, std::string &buffer) -> FILE_PARSER_EXECUTION_STATUS {
    try {
        std::fstream output_files{path, std::ofstream::binary};

        if (output_files) {
            output_files.write(&buffer[0], buffer.size());

            if (!output_files) {
                output_files.close();
                
                return FILE_PARSER_WRITING_DATA_ERROR;
            }
        } else {
            return FILE_PARSER_FILE_NOT_FOUND;
        }

        output_files.close();

        return FILE_PARSER_COMPLETED_SUCCESSFULY;
    } catch(std::fstream::failure &exeption) {
        return FILE_PARSER_FAILED;
    }
}