#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio> // Required for std::remove to clean up intermediate files

// Compiler metadata
const std::string COMPILER_VERSION = "0.1.95-alpha";
const std::string COMPILER_NAME = "vcomp";

/**
 * @brief Removes leading and trailing whitespace characters from a string.
 */
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

/**
 * @brief Replaces all occurrences of a substring within a string.
 */
std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

int main(int argc, char* argv[]) {
    // Validate command-line arguments
    if (argc < 2) {
        std::cerr << "Usage: " << COMPILER_NAME << " <filename.nvpp>\n";
        std::cerr << "       " << COMPILER_NAME << " --version\n";
        return 1;
    }

    std::string argument = argv[1];

    // Process version information request
    if (argument == "--version" || argument == "-v") {
        std::cout << COMPILER_NAME << " (Nova++) Compiler Version " << COMPILER_VERSION << "\n";
        return 0;
    }

    // Validate file extension
    if (argument.size() < 5 || argument.substr(argument.find_last_of(".") + 1) != "nvpp") {
        std::cerr << "[ERROR] Invalid file extension. Input file must have a '.nvpp' extension.\n";
        return 1;
    }

    std::ifstream input_file(argument);
    if (!input_file.is_open()) {
        std::cerr << "[ERROR] Failed to open input file: " << argument << "\n";
        return 1;
    }

    // --- FILE INGESTION & VALIDATION ---
    std::string line;
    bool has_nova_include = false;
    std::vector<std::string> file_lines;

    while (std::getline(input_file, line)) {
        file_lines.push_back(line);
        std::string check_line = trim(line);
        if (check_line.rfind("#include <nova>", 0) == 0) {
            has_nova_include = true;
        }
    }
    input_file.close();

    if (!has_nova_include) {
        std::cerr << "[COMPILATION ERROR] Mandatory '#include <nova>' directive is missing.\n";
        std::cerr << " -> Location: Top of file required.\n";
        return 1;
    }
    // ------------------------------------

    std::vector<std::string> cpp_lines;
    
    // Inject C++ Standard Library boilerplate
    cpp_lines.push_back("#include <iostream>");
    cpp_lines.push_back("#include <string>");
    cpp_lines.push_back("#include <cstdlib>");
    cpp_lines.push_back("#include <clocale>"); // Required for UTF-8 character encoding support
    cpp_lines.push_back("\n// Variadic template helper for the 'pf' utility function");
    cpp_lines.push_back("template <typename... Args>");
    cpp_lines.push_back("void pf(Args... args) { (std::cout << ... << args) << \"\\n\"; }\n");

    bool is_no_main = false;

    // Parse Nova++ source code and transpile to standard C++
    for (size_t i = 0; i < file_lines.size(); ++i) {
        size_t current_line_num = i + 1; // Human-readable 1-based line index
        line = trim(file_lines[i]);
        if (line.empty()) continue;

        // Extract inline comments
        std::string comment = "";
        size_t comment_pos = line.find("//");
        if (comment_pos != std::string::npos) {
            comment = " " + line.substr(comment_pos);
            line = trim(line.substr(0, comment_pos));
        }

        // 1. Skip boilerplate directive
        if (line.rfind("#include <nova>", 0) == 0) {
            continue;
        }

        // 2. Transpile entry point directive
        if (line == "using noMain") {
            is_no_main = true;
            cpp_lines.push_back("int main() {");
            cpp_lines.push_back("    std::setlocale(LC_ALL, \".UTF8\"); // Enforce UTF-8 runtime environment");
            continue;
        }

        // 3. Transpile exit statement
        if (line.rfind("exit ", 0) == 0) {
            std::string val = line.substr(5);
            if (!val.empty() && val.back() == ';') val.pop_back();
            cpp_lines.push_back("    std::system(\"pause\");");
            cpp_lines.push_back("    return " + trim(val) + ";" + comment);
            continue;
        }

        // 4. Transpile print function 'pf'
        if (line.rfind("pf ", 0) == 0) {
            std::string content = line.substr(3);
            if (!content.empty() && content.back() == ';') content.pop_back();
            content = trim(content);

            if (content.empty()) {
                std::cerr << "[SYNTAX ERROR] Line " << current_line_num << ": 'pf' function requires arguments.\n";
                std::cerr << " -> Statement: \"" << file_lines[i] << "\"\n";
                return 1;
            }

            if (content.find("+") != std::string::npos && content.find("\"") == std::string::npos) {
                content = replace_all(content, "+", ",");
            }

            cpp_lines.push_back("    pf(" + content + ");" + comment);
            continue;
        }

        // 5. Transpile input function 'input'
        if (line.rfind("input ", 0) == 0) {
            std::string var_target = line.substr(6);
            if (!var_target.empty() && var_target.back() == ';') var_target.pop_back();
            var_target = trim(var_target);

            if (var_target.empty()) {
                std::cerr << "[SYNTAX ERROR] Line " << current_line_num << ": 'input' requires a target variable.\n";
                std::cerr << " -> Statement: \"" << file_lines[i] << "\"\n";
                return 1;
            }

            cpp_lines.push_back("    std::cin >> " + var_target + ";" + comment);
            continue;
        }

        // 6. Transpile conditional structures (if, elif, else)
        if (line.rfind("if ", 0) == 0 || line.rfind("if(", 0) == 0 || line.find("} if") != std::string::npos) {
            cpp_lines.push_back("    " + line + comment);
            continue;
        }
        if (line.find("elif") != std::string::npos) {
            std::string translation = replace_all(line, "elif", "else if");
            cpp_lines.push_back("    " + translation + comment);
            continue;
        }
        if (line.find("else") != std::string::npos) {
            cpp_lines.push_back("    " + line + comment);
            continue;
        }
        if (line == "}") {
            cpp_lines.push_back("    }" + comment);
            continue;
        }

        // 7. Transpile variable assignments and type inference (=)
        size_t eq_pos = line.find("=");
        if (eq_pos != std::string::npos && line.rfind("if", 0) != 0 && line.rfind("elif", 0) != 0) {
            std::string check_comparison = line.substr(eq_pos, 2);
            if (check_comparison != "==") {
                std::string var_name = trim(line.substr(0, eq_pos));
                std::string var_val = trim(line.substr(eq_pos + 1));
                if (!var_val.empty() && var_val.back() == ';') var_val.pop_back();

                if (var_name.empty() || var_val.empty()) {
                    std::cerr << "[SYNTAX ERROR] Line " << current_line_num << ": Malformed variable assignment.\n";
                    std::cerr << " -> Statement: \"" << file_lines[i] << "\"\n";
                    return 1;
                }

                bool is_digit = !var_val.empty() && std::all_of(var_val.begin(), var_val.end(), ::isdigit);
                bool is_bool = (var_val == "true" || var_val == "false");
                bool has_quotes = (!var_val.empty() && var_val.front() == '"' && var_val.back() == '"');

                if (!is_digit && !is_bool && !has_quotes && var_val.find_first_of("+-*/") == std::string::npos) {
                    var_val = "\"" + var_val + "\"";
                }

                cpp_lines.push_back("    auto " + var_name + " = " + var_val + ";" + comment);
                continue;
            }
        }

        // Append standard syntax safely
        if (!line.empty()) {
            if (line.back() != ';' && line.back() != '{' && line.back() != '}') {
                line += ";";
            }
            cpp_lines.push_back("    " + line + comment);
        }
    }

    // Append system pause hook before scope termination if necessary
    if (is_no_main) {
        cpp_lines.push_back("    std::system(\"pause\");");
        cpp_lines.push_back("}");
    }

    // Write translated source to intermediate C++ file
    std::string output_filename = argument.substr(0, argument.find_last_of(".")) + ".cpp";
    std::ofstream output_file(output_filename);
    for (const auto& l : cpp_lines) {
        output_file << l << "\n";
    }
    output_file.close();

    std::cout << "[VCOMP] Successfully generated intermediate target: " << output_filename << "\n";

    // Invoke host GCC system compiler to build the target binary
    std::string binary_name = argument.substr(0, argument.find_last_of("."));
    std::string compile_cmd = "g++ " + output_filename + " -o " + binary_name;
    
    if (std::system(compile_cmd.c_str()) != 0) {
        std::cerr << "[ERROR] Downstream GCC compilation failed.\n";
        return 1;
    }
    
    std::cout << "[VCOMP] Compilation successful. Binary generated: '" << binary_name << "'\n";

    // Clean up: Delete the intermediate .cpp file now that binary is ready
    if (std::remove(output_filename.c_str()) != 0) {
        std::cerr << "[WARNING] Failed to clean up intermediate file: " << output_filename << "\n";
    } else {
        std::cout << "[VCOMP] Cleaned up intermediate source: " << output_filename << "\n";
    }

    return 0;
}