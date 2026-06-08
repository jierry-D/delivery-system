#pragma once
#include <string>

struct Node {
    int id;
    std::string name;
    std::string address;

    Node() : id(0) {}
    Node(int id, const std::string& name, const std::string& address)
        : id(id), name(name), address(address) {}
};
