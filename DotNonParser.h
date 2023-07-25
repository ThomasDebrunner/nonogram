//
// Created by Thomas Debrunner on 25.07.23.
//

#ifndef NONOGRAM_DOTNONPARSER_H
#define NONOGRAM_DOTNONPARSER_H

#include <fstream>
#include <iostream>
#include <regex>

class DotNonParser {
private:
    std::string _title;
    int _width;
    int _height;
    std::vector<std::vector<int>> _h_blocks;
    std::vector<std::vector<int>> _v_blocks;

public:
    DotNonParser() {};

    ~DotNonParser() {};

    void parse(const char *filename) {
        std::ifstream file(filename);
        if (!file.good()) {
            std::cout << "File " << filename << " does not exist" << std::endl;
            exit(1);
        }

        std::string line;

        std::regex width_regex("^width (\\d+)$");
        std::regex height_regex("^height (\\d+)$");
        std::regex title_regex("^title \"(.*)\"$");
        std::regex columns_regex("^columns$");
        std::regex rows_regex("^rows$");
        std::regex constraints_regex("^[0-9,]*[0-9]$");

        bool in_columns = false;
        bool in_rows = false;

        while (std::getline(file, line)) {
            // match regex
            std::smatch match;
            if (std::regex_match(line, match, width_regex)) {
                _width = std::stoi(match[1]);
            } else if (std::regex_match(line, match, height_regex)) {
                _height = std::stoi(match[1]);
            } else if (std::regex_match(line, match, title_regex)) {
                _title = match[1];
            } else if (std::regex_match(line, match, columns_regex)) {
                in_columns = true;
                in_rows = false;
            } else if (std::regex_match(line, match, rows_regex)) {
                in_rows = true;
                in_columns = false;
            } else if (std::regex_match(line, match, constraints_regex)) {
                std::vector<int> blocks;
                std::string blocks_str = line;
                size_t pos = 0;
                while ((pos = blocks_str.find(',')) != std::string::npos) {
                    std::string token = blocks_str.substr(0, pos);
                    blocks.push_back(std::stoi(token));
                    blocks_str.erase(0, pos + 1);
                }
                blocks.push_back(std::stoi(blocks_str));
                if (in_columns) {
                    _h_blocks.push_back(blocks);
                } else if (in_rows) {
                    _v_blocks.push_back(blocks);
                }
            }
        }

        if (_h_blocks.size() != _width) {
            std::cerr << "Error: width does not match number of columns" << std::endl;
            exit(1);
        }

        if (_v_blocks.size() != _height) {
            std::cerr << "Error: height does not match number of rows" << std::endl;
            exit(1);
        }
    }

    const std::string &title() const {
        return _title;
    }

    int width() const {
        return _width;
    }

    int height() const {
        return _height;
    }

    const std::vector<std::vector<int>> &h_blocks() const {
        return _h_blocks;
    }

    const std::vector<std::vector<int>> &v_blocks() const {
        return _v_blocks;
    }

};

#endif //NONOGRAM_DOTNONPARSER_H
