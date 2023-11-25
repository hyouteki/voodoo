#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <regex>

typedef struct Todo Todo;

void error(std::string);
std::string section_fmt(std::string);
void add(const Todo, const std::string = "VooDoo");
std::string trim(std::string);
void parse();
void list(std::string = "VooDoo", bool = false);
void save();
void del(std::string, std::string = "VooDoo");
void mark(std::string, std::string = "VooDoo");
void unmark(std::string, std::string = "VooDoo");
void usage();

struct Todo {
    std::string task;
    bool done;
public:
    Todo() {}
    Todo(const std::string task, const bool done) {
        this->task = task;
        this->done = done;
    }
    Todo(const std::string line) {
        if (line.size() < 5) return;
        this->done = line.substr(0, 5) == "- [x]";
        this->task = trim(line.substr(5));
    }
    std::string to_str(bool stylized = false) const {
        std::string out;
        if (this->done) {
            if (stylized) out += "\033[90m";
            out += "- [x] ";
        }
        else out += "- [ ] ";
        out += this->task;
        if (this->done && stylized) out += "\033[39m";
        return out;
    }
};

#define filename "voodoo.md"
std::map<std::string, std::vector<Todo>> todos;

void error(std::string message) {
    std::cerr << "\033[31m" + message + "\033[0m" << std::endl;
    exit(1);
}

std::string section_fmt(std::string text) {
    return "\033[35m\033[3m" + text + "\033[39m\033[0m";
}

void add(const Todo todo, const std::string section) {
    if (todos.find(section) == todos.end())
        todos[section] = {};
    todos[section].push_back(todo);
}

std::string trim(std::string str) {
    std::string tmp = "";
    bool flag = false;
    for (char ch: str) {
        if (flag) tmp.push_back(ch);
        else if (ch != ' ') {
            tmp.push_back(ch);
            flag = true;
        }
    }
    return tmp;
}

void parse() {
    std::ifstream file(filename);
    std::string line;
    while(getline(file, line)) {
        line = trim(line);
        if (line.size() == 0) continue;
        if (line[0] == '#') {
            std::string section_name = trim(line.substr(1));
            while(getline(file, line)) {
                line = trim(line);
                if (line.size() == 0 || line[0] != '-') break;
                add(Todo(line), section_name);
            }
        }
    }
    file.close();
}

void list(std::string section, bool all) {
    if (all) {
        for (auto itr: todos) {
            std::string section = itr.first;
            std::vector<Todo> tasks = itr.second;
            std::cout << section_fmt("# " + section) << std::endl;
            for (Todo todo: tasks)
                std::cout << todo.to_str(true) << std::endl;
            std::cout << std::endl;
        }
        return;
    }
    if (todos.find(section) == todos.end())
        error("Error: Section name does not exist");
    std::cout << section_fmt("# " + section) << std::endl;
    for (Todo todo: todos[section])
        std::cout << todo.to_str(true) << std::endl;
    std::cout << std::endl;
}

void save() {
    std::ofstream file(filename);
    for (auto itr: todos) {
        std::string section = itr.first;
        std::vector<Todo> tasks = itr.second;
        if (tasks.size() == 0) continue;
        file << "# " << section << std::endl;
        for (Todo todo: tasks)
            file << todo.to_str() << std::endl;
        file << std::endl;
    }
    file.close();
}

void del(std::string text, std::string section) {
    std::regex target(text);
    if (todos.find(section) == todos.end())
        error("Error: Section name does not exist");
    for (size_t i = 0; i < todos.at(section).size(); ++i) {
        Todo todo = todos.at(section)[i];
        if (std::regex_match(todo.task, target)) {
            todos[section].erase(todos[section].begin()+i);
            return;
        }
    }
    error("Error: No matching todo");
}

void mark(std::string text, std::string section) {
    std::regex target(text);
    if (todos.find(section) == todos.end())
        error("Error: Section name does not exist");
    for (size_t i = 0; i < todos.at(section).size(); ++i) {
        Todo todo = todos.at(section)[i];
        if (std::regex_match(todo.task, target)) {
            todos[section][i].done = true;
            return;
        }
    }
    error("Error: No matching todo");
}

void unmark(std::string text, std::string section) {
    std::regex target(text);
    if (todos.find(section) == todos.end())
        error("Error: Section name does not exist");
    for (size_t i = 0; i < todos.at(section).size(); ++i) {
        Todo todo = todos.at(section)[i];
        if (std::regex_match(todo.task, target)) {
            todos[section][i].done = false;
            return;
        }
    }
    error("Error: No matching todo");
}

void usage() {
    std::cout << "usage: voodoo <command> [<args>]" << std::endl;
    std::cout << "commands: help list add del mark unmark" << std::endl << std::endl;
    std::cout << "-\thelp" << std::endl;
    std::cout << "-\tlist\t\"<section:optional>\"" << std::endl;
    std::cout << "-\tadd\t\"<section:optional>\"\t\"<todo>\"" << std::endl;
    std::cout << "-\tdel\t\"<section:optional>\"\t\"<todo>\"" << std::endl;
    std::cout << "-\tmark\t\"<section:optional>\"\t\"<todo>\"" << std::endl;
    std::cout << "-\tunmark\t\"<section:optional>\"\t\"<todo>\"" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        usage();
        exit(1);
    }
    parse();
    if (std::string("add") == std::string(argv[1])) {
        switch (argc) {
            case 3:
                add(Todo(argv[2], false));
                break;
            default:
                add(Todo(argv[3], false), argv[2]);
        }
    } else if (std::string("del") == std::string(argv[1])) {
        switch (argc) {
            case 3:
                del(argv[2]);
                break;
            default:
                del(argv[3], argv[2]);
        }
    } else if (std::string("mark") == std::string(argv[1])) {
        switch (argc) {
            case 3:
                mark(argv[2]);
                break;
            default:
                mark(argv[3], argv[2]);
        }
    } else if (std::string("unmark") == std::string(argv[1])) {
        switch (argc) {
            case 3:
                unmark(argv[2]);
                break;
            default:
                unmark(argv[3], argv[2]);
        }
    } else if (std::string("list") == std::string(argv[1])) {
        switch (argc) {
            case 2:
                list("VooDoo", true);
                break;
            default:
                list(argv[2], false);
        }
    }
    else if (std::string("help") == std::string(argv[1])) usage();
    save();
    return 0;
}
