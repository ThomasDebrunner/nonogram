#include <iostream>
#include <vector>
#include <unordered_set>
#include "DotNonParser.h"

enum class CellState {
    EMPTY,
    FILLED,
    UNKNOWN
};

class Dictionary {
private:
    int _size;
    std::vector<std::unordered_set<uint64_t>> _dicts;

    static void _buildDict(
            const std::vector<int> &blocks, int blocks_pos, uint64_t entry, int entry_pos, int entry_size, std::unordered_set<uint64_t>& dict) {
        if (blocks_pos >= blocks.size()) {
            // we made it
            dict.insert(entry);
            return;
        }

        int block = blocks[blocks_pos];
        if (block > entry_size - entry_pos) {
            // no more space
            return;
        }

        // try out all options where we can move this block from here
        for (int i=entry_pos; i<=entry_size-block; i++) {
            uint64_t new_entry = entry;
            for (int j=i; j<i+block; j++) {
                new_entry |= (1ULL << j);
            }
            _buildDict(blocks, blocks_pos+1, new_entry, i+block+1, entry_size, dict);
        }
    }

    void _printPattern(uint64_t p) {
        for (int i=0; i<_size; i++) {
            if ((p >> i) & 1) {
                std::cout << "██";
            } else {
                std::cout << "  ";
            }
        }
        std::cout << "  " << p << std::endl;
    }

public:
    Dictionary() = default;

    static Dictionary fromConstraints(const std::vector<std::vector<int>>& constraints, int size) {
        Dictionary result;

        // build dictionaries
        for (const auto& blocks : constraints) {
            std::unordered_set<uint64_t> dict;
            Dictionary::_buildDict(blocks, 0, 0, 0, size, dict);
            result._dicts.push_back(dict);
            result._size = size;
        }
        return result;
    }

    void print(int pos) {
        if (pos >= _dicts.size()) {
            return;
        }
        auto &dict = _dicts[pos];
        std::cout << "Pos " << pos << ": n=" << _dicts[pos].size() << std::endl;
        std::cout << "  ";
        for (const auto p : dict) {
            _printPattern(p);
        }
        std::cout << "-------------------" << std::endl;

    }

    CellState getEvidence(int idx1, int idx2) {
        if (_dicts[idx1].empty()) {
            std::cout << "No solution possible. Nonogram has errors" << std::endl;
            exit(1);
        }

        uint64_t pos_mask = 0xFFFFFFFFFFFFFFFF;
        uint64_t neg_mask = 0xFFFFFFFFFFFFFFFF;
        for (const auto& entry : _dicts[idx1]) {
            pos_mask &= entry;
            neg_mask &= ~entry;
        }

        if ((pos_mask >> idx2) & 1) {
            return CellState::FILLED;
        } else if ((neg_mask >> idx2) & 1) {
            return CellState::EMPTY;
        } else {
            return CellState::UNKNOWN;
        }
    }

    void cleanEntries(int idx1, int idx2, bool verified_state) {
        // remove all entries that do not match the verified state
        auto &dict = _dicts[idx1];
        auto it = dict.begin();
        while (it != dict.end()) {
            if (((*it) >> idx2) & 1) {
                if (!verified_state) {
                    it = dict.erase(it);
                    continue;
                }
            } else {
                if (verified_state) {
                    it = dict.erase(it);
                    continue;
                }
            }
            it++;
        }
    }
};


class Nonogram {
private:
    std::vector<std::vector<int>> _h_constraints;
    std::vector<std::vector<int>> _v_constraints;
    int _width;
    int _height;

    Dictionary _h_dicts;
    Dictionary _v_dicts;

    std::vector<CellState> _board;

    CellState _computeEvidenceAndClearSingleDim(Dictionary &work_dict, Dictionary &clear_dict, int idx1, int idx2) {
        CellState result = work_dict.getEvidence(idx1, idx2);
        if (result == CellState::EMPTY) {
            clear_dict.cleanEntries(idx2, idx1, false);
            return CellState::EMPTY;
        }
        if (result == CellState::FILLED) {
            clear_dict.cleanEntries(idx2, idx1, true);
            return CellState::FILLED;
        }
        return CellState::UNKNOWN;
    }

    CellState _computeEvidenceAndClear(int x, int y) {
        CellState h_result = _computeEvidenceAndClearSingleDim(_h_dicts, _v_dicts, x, y);
        if (h_result == CellState::EMPTY || h_result == CellState::FILLED) {
            return h_result;
        }
        CellState v_result = _computeEvidenceAndClearSingleDim(_v_dicts, _h_dicts, y, x);
        if (v_result == CellState::EMPTY || v_result == CellState::FILLED) {
            return v_result;
        }
        return CellState::UNKNOWN;
    }

    int _x(int pos) {
        return pos % _width;
    }

    int _y(int pos) {
        return pos / _width;
    }

    bool _solve(int pos) {
        if (pos >= _board.size()) {
            // we made it
            return false;
        }

        if (at(_x(pos), _y(pos)) != CellState::UNKNOWN) {
            // this is already solved
            return _solve(pos+1);
        }

        CellState new_state = _computeEvidenceAndClear(_x(pos), _y(pos));
        bool we_changed = new_state != CellState::UNKNOWN;
        _board[pos] = new_state;
        bool child_changed = _solve(pos+1);
        return we_changed || child_changed;
    }

public:
    Nonogram(std::vector<std::vector<int>> h_constraints, std::vector<std::vector<int>> v_constraints) :
            _h_constraints(std::move(h_constraints)), _v_constraints(std::move(v_constraints)) {
        _width = _h_constraints.size();
        _height = _v_constraints.size();
        _board.resize(_width * _height, CellState::UNKNOWN);
    }

    bool checkSolved() const {
        for (const auto &state : _board) {
            if (state == CellState::UNKNOWN) {
                return false;
            }
        }
        return true;
    }

    bool solve() {
        // build dictionaries
        _h_dicts = Dictionary::fromConstraints(_h_constraints, _height);
        _v_dicts = Dictionary::fromConstraints(_v_constraints, _width);

        bool any_changed;
        do {
            any_changed = _solve(0);
            printBoard();
        } while(any_changed);
        return checkSolved();
    }

    CellState& at(int x, int y) {
        return _board[y*_width + x];
    }

    const CellState& at(int x, int y) const {
        return _board[y*_width + x];
    }


    void printBoard() const {
        std::cout << "=====================================================================================" << std::endl;
        for (int y=0; y<_height; y++) {
            for (int x=0; x<_width; x++) {
                switch (at(x, y)) {
                    case CellState::EMPTY:
                        std::cout << "  ";
                        break;
                    case CellState::FILLED:
                        std::cout << "██";
                        break;
                    case CellState::UNKNOWN:
                        std::cout << " .";
                        break;
                }
            }
            std::cout << std::endl;
        }
    }

};



int main(int argc, char **argv) {

    // check if we have a filename argument
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

//    Dictionary test_dict = Dictionary::fromConstraints({
//        {15, 14}
//        }, 35);
//    test_dict.print(0);
//
//    return 0;

    // parse the file
    DotNonParser parser;
    parser.parse(argv[1]);

    Nonogram nonogram(parser.h_blocks(), parser.v_blocks());

    bool solved = nonogram.solve();

    if (solved) {
        std::cout << "Solved!" << std::endl;
        return 0;
    } else {
        std::cout << "Not solved!" << std::endl;
        return 1;
    }
}
