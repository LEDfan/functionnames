// Copyright (C) 2017-2018 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

// adapted by LEDfan to check for correct function and method names

#include <iostream>
#include <iomanip>

#include "cppast/external/cxxopts/include/cxxopts.hpp"

#include <cppast/libclang_parser.hpp> // for libclang_parser, libclang_compile_config, cpp_entity,...
#include <cppast/visitor.hpp>         // for visit()
#include <cppast/code_generator.hpp>  // for generate_code()
#include <cppast/cpp_entity_kind.hpp>        // for the cpp_entity_kind definition
#include <cppast/cpp_forward_declarable.hpp> // for is_definition()
#include <cppast/cpp_namespace.hpp>          // for cpp_namespace

#define TEXT_NORMAL "\e[0m"
#define TEXT_GREEN "\e[32m"
#define TEXT_RED "\033[31m"

void print_error(const std::string &msg) {
        std::cerr << msg << '\n';
}

bool check(const cppast::cpp_file &file) {
        std::cout << "Checking functions and methods inside '" << file.name() << "':\n";

        bool fileCompliant = true;
        cppast::visit(file, [&](const cppast::cpp_entity &e, cppast::visitor_info info) {
                if (e.kind() == cppast::cpp_entity_kind::file_t || cppast::is_templated(e)
                        || cppast::is_friended(e)) {
                        return true;
                } else {
                        if (e.kind() == cppast::cpp_entity_kind::function_t || e.kind() == cppast::cpp_entity_kind::member_function_t) {
                                if (!e.name().empty()) {
                                        const auto& name = e.name();

                                        bool compliant = false;

                                        if (name == "begin" || name == "end" || name == "cbegin" || name == "cend" || name.substr(0, 8) == "operator") {
                                                compliant = true;
                                        }

                                        if (name[0] == std::toupper(name[0])) {
                                                compliant = true;
                                        }


                                        if (compliant) {
                                                std::cout << " - " <<  TEXT_GREEN << std::left << std::setw(6) << "[OK]" << TEXT_NORMAL;
                                        } else {
                                                std::cout << " - " << TEXT_RED << std::left << std::setw(6) << "[ERR]" << TEXT_NORMAL;
                                        }

                                        std::cout << e.name() << std::endl;

                                        fileCompliant &= compliant;
                                }
                        }
                }
                return true;
        });

        return fileCompliant;

}

// parse a file
std::unique_ptr<cppast::cpp_file> parse_file(const cppast::libclang_compile_config &config,
                                             const cppast::diagnostic_logger &logger,
                                             const std::string &filename, bool fatal_error) {
        // the entity index is used to resolve cross references in the AST
        // we don't need that, so it will not be needed afterwards
        cppast::cpp_entity_index idx;
        // the parser is used to parse the entity
        // there can be multiple parser implementations
        cppast::libclang_parser parser(type_safe::ref(logger));
        // parse the file
        auto file = parser.parse(idx, filename, config);
        if (fatal_error && parser.error())
                return nullptr;
        return file;
}

int main(int argc, char *argv[]) try {
        cxxopts::Options options("cppast",
                                 "cppast - The commandline interface to the cppast library.\n");
        // clang-format off
        options.add_options()
                ("file", "the file that is being parsed (last positional argument)",
                 cxxopts::value<std::string>());

        options.add_options("compilation")
                ("database_dir",
                 "set the directory where a 'compile_commands.json' file is located containing build information",
                 cxxopts::value<std::string>())
                ("database_file",
                 "set the file name whose configuration will be used regardless of the current file name",
                 cxxopts::value<std::string>())
                ("I,include_directory", "add directory to include search path",
                 cxxopts::value<std::vector<std::string>>())
                ("D,macro_definition", "define a macro on the command line",
                 cxxopts::value<std::vector<std::string>>())
                ("U,macro_undefinition", "undefine a macro on the command line",
                 cxxopts::value<std::vector<std::string>>())
                ("gnu_extensions", "enable GNU extensions (equivalent to -std=gnu++XX)")
                ("msvc_extensions", "enable MSVC extensions (equivalent to -fms-extensions)")
                ("msvc_compatibility", "enable MSVC compatibility (equivalent to -fms-compatibility)");

        // clang-format on
        options.parse_positional("file");
        options.parse(argc, argv);

       if (!options.count("file") || options["file"].as<std::string>().empty()) {
                print_error("missing file argument");
                return 1;
        } else {
                // the compile config stores compilation flags
                cppast::libclang_compile_config config;
                if (options.count("database_dir")) {
                        cppast::libclang_compilation_database database(
                                options["database_dir"].as<std::string>());
                        if (options.count("database_file"))
                                config =
                                        cppast::libclang_compile_config(database,
                                                                        options["database_file"].as<std::string>());
                        else
                                config =
                                        cppast::libclang_compile_config(database, options["file"].as<std::string>());
                }

                config.write_preprocessed(true);

                config.fast_preprocessing(true);

                config.remove_comments_in_macro(true);

                if (options.count("include_directory"))
                        for (auto &include : options["include_directory"].as<std::vector<std::string>>())
                                config.add_include_dir(include);
                if (options.count("macro_definition"))
                        for (auto &macro : options["macro_definition"].as<std::vector<std::string>>()) {
                                auto equal = macro.find('=');
                                auto name = macro.substr(0, equal);
                                if (equal == std::string::npos)
                                        config.define_macro(std::move(name), "");
                                else {
                                        auto def = macro.substr(equal + 1u);
                                        config.define_macro(std::move(name), std::move(def));
                                }
                        }
                if (options.count("macro_undefinition"))
                        for (auto &name : options["macro_undefinition"].as<std::vector<std::string>>())
                                config.undefine_macro(name);

                config.set_flags(cppast::cpp_standard::cpp_14);

                // the logger is used to print diagnostics
                cppast::stderr_diagnostic_logger logger;
                if (options.count("verbose"))
                        logger.set_verbose(true);

                auto file = parse_file(config, logger, options["file"].as<std::string>(),
                                       options.count("fatal_errors") == 1);
                if (!file)
                        return 2;

                if (check(*file)) {
                        std::cout << TEXT_GREEN << "Good job, file is compliant!" << TEXT_NORMAL << std::endl;
                        return 0;
                } else {
                        std::cout << TEXT_RED << "Oh-oh! file is NOT compliant!" << TEXT_NORMAL << std::endl;
                        return 1;
                }

        }
}
catch (const cppast::libclang_error &ex) {
        print_error(std::string("[fatal parsing error] ") + ex.what());
        return 2;
}
