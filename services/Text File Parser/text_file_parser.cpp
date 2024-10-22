#include <fstream>
#include <iostream>
#include <sys/types.h>

#include "text_file_parser.h"

auto text_file_parser::get_file_length(std::ifstream &input_file) -> u_int64_t
{
    input_file.seekg (0, input_file.end);
    auto length = input_file.tellg();
    input_file.seekg (0, input_file.beg);

    return length;
}

auto text_file_parser::read_file_to_string(const std::string& path) -> std::string
{
    std::string contents;
    std::ifstream input_file{path, std::ifstream::binary};

    if (input_file) {
        auto length = get_file_length(input_file);

        contents.resize(length);
        input_file.read(&contents[0], length);

        if (!input_file) {
            input_file.close();
            
            return {};
        }
    } else {
        std::cerr << "Файл не найден!" << std::endl;
    }

    input_file.close();

    return contents;
}