//
// Created by MrMam on 12.09.2022.
//

#include <atomic>
#include <seq_storage/seq_storage.hxx>
#include <algorithm>
#include <iostream>
#include <sstream>


seq_storage* volatile seq_storage::storage = nullptr;
std::mutex seq_storage::_mutex;


bool seq_storage::add_sequence(const ip_address &name, const std::string &query) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(query);
    while (std::getline(tokenStream, token, ' ')) {
        tokens.push_back(token);
    }
    if (tokens.size() != 3) {
        std::cerr << "Wrong query format" << std::endl;
        return false;
    }
    counter start, step, seq_number;
    try {
        seq_number = std::stoi(tokens[0].substr(tokens[0].size() - 1), nullptr, 0);
        start = std::stoull(tokens[1], nullptr, 0);
        step = std::stoull(tokens[2], nullptr, 0);
        if (seq_number == 0 || start == 0 || step == 0) {
            std::cerr << "Invalid sequence parameters: seq_number: " << seq_number << " " << start << " " << step
                      << std::endl;
            return false;
        }
        std::scoped_lock<std::mutex> lock(_mutex);
        _storage[name][seq_number - 1] = sequence(start, step);
    } catch(std::exception& e) {
        std::cerr << "Invalid sequence parameters: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool seq_storage::remove_sequence(const ip_address &name) {
    std::scoped_lock<std::mutex> lock(_mutex);
    if (_storage.find(name) == _storage.end()) {
        return false;
    }
    _storage.erase(name);
    return true;
}

bool seq_storage::contains(const ip_address &name) const {
    std::scoped_lock<std::mutex> lock(_mutex);
    return _storage.find(name) != _storage.end();
}

std::vector<uint8_t> seq_storage::next(const ip_address &name) {
    std::scoped_lock<std::mutex> lock(_mutex);
    std::string result;
    for (auto &seq: _storage[name]) {
        result += std::to_string(seq.next()) + " ";
    }
    return {result.begin(), result.end()};
}

bool seq_storage::create_sequence(const ip_address& addr) {
    std::scoped_lock<std::mutex> lock(_mutex);
    auto [_, added] = (_storage.try_emplace(addr, std::array<sequence, 3>{}));
    return added;
}
