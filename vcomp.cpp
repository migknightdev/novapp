#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio> // Required for std::remove to clean up intermediate files

// Compiler metadata
const std::string COMPILER_VERSION = "0.2.2-beta(bugfix-2)";
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
    std::string raw_line;
    bool has_nova_include = false;
    std::vector<std::string> file_lines;

    while (std::getline(input_file, raw_line)) {
        file_lines.push_back(raw_line);
        std::string check_line = trim(raw_line);
        if (check_line.find("#include <nova>") != std::string::npos) {
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

    std::vector<std::string> cpp_global_lines; // For structures, functions and custom blocks
    std::vector<std::string> cpp_main_lines;   // Procedural commands that MUST execute inside main()

    // Inject C++ Standard Library boilerplate
    cpp_global_lines.push_back("#include <iostream>");
    cpp_global_lines.push_back("#include <string>");
    cpp_global_lines.push_back("#include <cstdlib>");
    cpp_global_lines.push_back("#include <clocale>"); // Required for UTF-8 character encoding support
    cpp_global_lines.push_back("\n// Variadic template helper for the 'pf' utility function");
    cpp_global_lines.push_back("template <typename... Args>");
    cpp_global_lines.push_back("void pf(Args... args) { (std::cout << ... << args) << \"\\n\"; }\n");

    bool is_no_main = false;
    bool inside_function = false;
    int function_brace_count = 0;
    // Rastreamento do main() declarado manualmente pelo usuário (fora do fluxo 'noMain').
    // Precisamos saber exatamente qual '}' fecha esse main manual para poder descartar
    // SOMENTE essa chave específica, sem afetar chaves legítimas de if/elif/else
    // aninhados no escopo procedural.
    bool manual_main_detected = false;
    int manual_main_brace_balance = 0;

    // Parse Nova++ source code and transpile to standard C++
    for (size_t i = 0; i < file_lines.size(); ++i) {
        size_t current_line_num = i + 1;
        std::string original_line = file_lines[i];
        std::string line = trim(original_line);
        if (line.empty()) continue;

        // Ignorar declarações manuais redundantes de int main para evitar duplicidade estrutural
        if (line.find("int main()") != std::string::npos) {
            is_no_main = true;
            manual_main_detected = true;
            int open_b = static_cast<int>(std::count(line.begin(), line.end(), '{'));
            int close_b = static_cast<int>(std::count(line.begin(), line.end(), '}'));
            manual_main_brace_balance += (open_b - close_b);
            continue;
        }

        // Preservar a indentação original capturando os espaços iniciais
        std::string leading_spaces = "";
        size_t first_non_blank = original_line.find_first_not_of(" \t\r\n");
        if (first_non_blank != std::string::npos && first_non_blank > 0) {
            leading_spaces = original_line.substr(0, first_non_blank);
        }

        // Extract inline comments
        std::string comment = "";
        size_t comment_pos = line.find("//");
        if (comment_pos != std::string::npos) {
            comment = " " + line.substr(comment_pos);
            line = trim(line.substr(0, comment_pos));
        }

        // 1. Skip boilerplate directive
        if (line.find("#include <nova>") != std::string::npos) {
            continue;
        }

        // 2. Transpile entry point directive
        if (line == "using noMain") {
            is_no_main = true;
            continue;
        }

        // 3. Transpile custom functions declaration
        if (line.rfind("func ", 0) == 0) {
            inside_function = true;
            function_brace_count = 0;
            std::string func_decl = line.substr(5); // Remove "func "

            if (line.find("{") != std::string::npos) {
                function_brace_count++;
            }

            std::string translated_func = leading_spaces + "void " + func_decl;
            cpp_global_lines.push_back(translated_func + comment);
            continue;
        }

        // Rastreamento dinâmico de balanceamento de chaves para funções de escopo global.
        // FIX: em vez de apenas checar se a linha "contém" um '{', agora contamos o saldo
        // líquido de chaves abertas menos chaves fechadas na MESMA linha. Isso corrige o
        // bug em que linhas como "} else {" ou "} elif (...) {" incrementavam o contador
        // sem nunca compensar o fechamento, fazendo 'inside_function' nunca voltar a false.
        bool is_manual_main_closing_brace = false;
        if (inside_function) {
            int open_braces  = static_cast<int>(std::count(line.begin(), line.end(), '{'));
            int close_braces = static_cast<int>(std::count(line.begin(), line.end(), '}'));
            function_brace_count += (open_braces - close_braces);
        } else if (manual_main_detected && manual_main_brace_balance > 0) {
            // Mesma lógica de saldo líquido, mas aplicada ao main() manual em vez de a
            // uma func. Só marcamos a linha para descarte quando o saldo VOLTA a zero
            // E a linha é exatamente '}' isolado — assim if/elif/else aninhados (que
            // sempre mantêm saldo positivo até fecharem de verdade) nunca são afetados.
            int open_braces  = static_cast<int>(std::count(line.begin(), line.end(), '{'));
            int close_braces = static_cast<int>(std::count(line.begin(), line.end(), '}'));
            manual_main_brace_balance += (open_braces - close_braces);
            if (manual_main_brace_balance <= 0 && line == "}") {
                is_manual_main_closing_brace = true;
            }
        }

        // Seleciona o vetor alvo baseado no escopo ativo atual
        std::vector<std::string>& current_target_vector = (inside_function) ? cpp_global_lines : cpp_main_lines;
        std::string scope_indentation = (!inside_function) ? "    " : "";

        // 4. Transpile exit statement
        if (line.rfind("exit ", 0) == 0) {
            std::string val = line.substr(5);
            if (!val.empty() && val.back() == ';') val.pop_back();
            current_target_vector.push_back(scope_indentation + leading_spaces + "std::system(\"pause\");");
            current_target_vector.push_back(scope_indentation + leading_spaces + "return " + trim(val) + ";" + comment);
            continue;
        }

        // 5. Transpile print function 'pf'
        if (line.rfind("pf ", 0) == 0) {
            std::string content = line.substr(3);
            if (!content.empty() && content.back() == ';') content.pop_back();
            content = trim(content);

            if (content.empty()) {
                std::cerr << "[SYNTAX ERROR] Line " << current_line_num << ": 'pf' function requires arguments.\n";
                return 1;
            }

            // FIX: a versão anterior só convertia '+' em ',' quando a linha NÃO tinha
            // aspas — o que é o caso mais comum e mais quebrado (ex: pf "texto" + var).
            // Agora percorremos a string caractere a caractere, respeitando o estado
            // "dentro de aspas" (pra não mexer no '+' que é texto literal, tipo " + ")
            // e a profundidade de parênteses (pra não quebrar expressões aritméticas
            // como "(num1 + num2)"). Só convertemos '+' em ',' quando ele está fora de
            // aspas e fora de parênteses — ou seja, quando é de fato um operador de
            // concatenação entre argumentos do pf().
            {
                std::string transpiled_content;
                bool inside_quotes = false;
                int paren_depth = 0;
                for (char c : content) {
                    if (c == '"') {
                        inside_quotes = !inside_quotes;
                        transpiled_content += c;
                    } else if (!inside_quotes && c == '(') {
                        paren_depth++;
                        transpiled_content += c;
                    } else if (!inside_quotes && c == ')') {
                        if (paren_depth > 0) paren_depth--;
                        transpiled_content += c;
                    } else if (c == '+' && !inside_quotes && paren_depth == 0) {
                        transpiled_content += ',';
                    } else {
                        transpiled_content += c;
                    }
                }
                content = transpiled_content;
            }

            current_target_vector.push_back(scope_indentation + leading_spaces + "pf(" + content + ");" + comment);
            continue;
        }

        // 6. Transpile input function 'input'
        if (line.rfind("input ", 0) == 0) {
            std::string var_target = line.substr(6);
            if (!var_target.empty() && var_target.back() == ';') var_target.pop_back();
            var_target = trim(var_target);

            if (var_target.empty()) {
                std::cerr << "[SYNTAX ERROR] Line " << current_line_num << ": 'input' requires a target variable.\n";
                return 1;
            }

            current_target_vector.push_back(scope_indentation + leading_spaces + "std::cin >> " + var_target + ";" + comment);
            continue;
        }

        // 7. Transpile conditional structures (if, elif, else)
        if (line.rfind("if ", 0) == 0 || line.rfind("if(", 0) == 0 || line.find("} if") != std::string::npos) {
            current_target_vector.push_back(scope_indentation + leading_spaces + line + comment);
            continue;
        }
        if (line.find("elif") != std::string::npos) {
            std::string translation = replace_all(line, "elif", "else if");
            current_target_vector.push_back(scope_indentation + leading_spaces + translation + comment);
            continue;
        }
        if (line.find("else") != std::string::npos) {
            current_target_vector.push_back(scope_indentation + leading_spaces + line + comment);
            continue;
        }

        // Scope closing tracking
        if (line == "}") {
            if (inside_function) {
                // FIX: o decremento de function_brace_count já foi feito acima, no bloco
                // de balanceamento geral. Removido o decremento duplicado que existia aqui
                // para não contar a mesma chave duas vezes.
                cpp_global_lines.push_back(leading_spaces + "}" + comment);
                if (function_brace_count <= 0) {
                    inside_function = false;
                }
            } else if (is_manual_main_closing_brace) {
                // Esta é exatamente a chave que fecha o int main() escrito manualmente
                // pelo usuário — descartamos só ela, não qualquer '}' solto.
                continue;
            } else {
                cpp_main_lines.push_back(scope_indentation + leading_spaces + "}" + comment);
            }
            continue;
        }

        // Universal open-brace safety passing
        if (line == "{") {
            current_target_vector.push_back(scope_indentation + leading_spaces + line + comment);
            continue;
        }

        // 8. Transpile variable assignments and type inference (=)
        size_t eq_pos = line.find("=");
        if (eq_pos != std::string::npos && line.rfind("if", 0) != 0 && line.rfind("elif", 0) != 0) {
            std::string check_comparison = line.substr(eq_pos, 2);
            if (check_comparison != "==") {
                std::string var_name = trim(line.substr(0, eq_pos));
                std::string var_val = trim(line.substr(eq_pos + 1));
                if (!var_val.empty() && var_val.back() == ';') var_val.pop_back();

                if (var_name.empty() || var_val.empty()) {
                    std::cerr << "[SYNTAX ERROR] Line " << current_line_num << ": Malformed variable assignment.\n";
                    return 1;
                }

                bool is_digit = !var_val.empty() && std::all_of(var_val.begin(), var_val.end(), ::isdigit);
                bool is_bool = (var_val == "true" || var_val == "false");
                bool has_quotes = (!var_val.empty() && var_val.front() == '"' && var_val.back() == '"');

                if (has_quotes) {
                    current_target_vector.push_back(scope_indentation + leading_spaces + "std::string " + var_name + " = " + var_val + ";" + comment);
                } else {
                    if (!is_digit && !is_bool && var_val.find_first_of("+-*/") == std::string::npos) {
                        var_val = "\"" + var_val + "\"";
                        current_target_vector.push_back(scope_indentation + leading_spaces + "std::string " + var_name + " = " + var_val + ";" + comment);
                    } else {
                        current_target_vector.push_back(scope_indentation + leading_spaces + "auto " + var_name + " = " + var_val + ";" + comment);
                    }
                }
                continue;
            }
        }

        // Append standard fallback syntax
        if (!line.empty()) {
            if (line.back() != ';' && line.back() != '{' && line.back() != '}') {
                line += ";";
            }
            current_target_vector.push_back(scope_indentation + leading_spaces + line + comment);
        }
    }

    // --- STRUCTURAL RECONSTRUCTION ---
    std::vector<std::string> cpp_lines = cpp_global_lines;

    cpp_lines.push_back("\nint main() {");
    cpp_lines.push_back("    std::setlocale(LC_ALL, \".UTF8\"); // Enforce UTF-8 runtime environment");

    // FIX: removido o filtro que descartava cegamente QUALQUER linha igual a '}' ou '};'.
    // Esse filtro apagava chaves de fechamento legítimas de if/elif/else aninhados no
    // escopo procedural (main), causando erro de chave desbalanceada no g++. A remoção
    // segura da chave que fecha um main() manual agora é feita durante o parsing (veja
    // 'is_manual_main_closing_brace' acima), então aqui já podemos injetar tudo direto.
    for (const auto& main_l : cpp_main_lines) {
        cpp_lines.push_back(main_l);
    }

    if (is_no_main) {
        cpp_lines.push_back("    std::system(\"pause\");");
    }
    cpp_lines.push_back("    return 0;");
    cpp_lines.push_back("}");

    std::string output_filename = argument.substr(0, argument.find_last_of(".")) + ".cpp";
    std::ofstream output_file(output_filename);
    for (const auto& l : cpp_lines) {
        output_file << l << "\n";
    }
    output_file.close();

    std::cout << "[VCOMP] Successfully generated intermediate target: " << output_filename << "\n";

    std::string binary_name = argument.substr(0, argument.find_last_of("."));
    std::string compile_cmd = "g++ " + output_filename + " -o " + binary_name;

    if (std::system(compile_cmd.c_str()) != 0) {
        std::cerr << "[ERROR] Downstream GCC compilation failed.\n";
        return 1;
    }

    std::cout << "[VCOMP] Compilation successful. Binary generated: '" << binary_name << "'\n";

    if (std::remove(output_filename.c_str()) != 0) {
        std::cerr << "[WARNING] Failed to clean up intermediate file: " << output_filename << "\n";
    } else {
        std::cout << "[VCOMP] Cleaned up intermediate source: " << output_filename << "\n";
    }

    return 0;
}