#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t");
    size_t end   = s.find_last_not_of(" \t");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

static std::string rewriteType(const std::string& type,
                               const std::string& exprBaseName) {
    // Check if type ends with '*' marker for unique_ptr
    if (!type.empty() && type.back() == '*') {
        std::string cleanType = type.substr(0, type.length() - 1);
        return "std::unique_ptr<" + cleanType + ">";
    }

    return type;
}

static void defineNilType(std::ofstream& out, const std::string& exprBaseName) {
    out << "struct Nil" << exprBaseName << " {};\n\n";
}

static void defineVisitor(std::ofstream& out, const std::string& exprBaseName,
                          const std::vector<std::string>& types) {
    out << "\n";
    out << "template <typename Visitor>\n";
    out << "decltype(auto) visit" << exprBaseName << "(" << exprBaseName << "& "
        << static_cast<char>(std::tolower(exprBaseName[0]))
        << exprBaseName.substr(1) << ", Visitor&& visitor) {\n";
    out << "    return std::visit(std::forward<Visitor>(visitor), "
        << static_cast<char>(std::tolower(exprBaseName[0]))
        << exprBaseName.substr(1) << ".node);\n";
    out << "}\n\n";

    out << "template <typename Visitor>\n";
    out << "decltype(auto) visit" << exprBaseName << "(const " << exprBaseName
        << "& " << static_cast<char>(std::tolower(exprBaseName[0]))
        << exprBaseName.substr(1) << ", Visitor&& visitor) {\n";
    out << "    return std::visit(std::forward<Visitor>(visitor), "
        << static_cast<char>(std::tolower(exprBaseName[0]))
        << exprBaseName.substr(1) << ".node);\n";
    out << "}\n";
}

static void defineType(std::ofstream& out, const std::string& spec,
                       const std::string& exprBaseName) {
    auto        colon     = spec.find(':');
    std::string className = trim(spec.substr(0, colon));
    std::string fields    = trim(spec.substr(colon + 1));

    out << "struct " << className << " {\n";

    size_t start = 0;
    while (true) {
        size_t      comma = fields.find(',', start);
        std::string field = trim(fields.substr(start, comma == std::string::npos
                                                          ? std::string::npos
                                                          : comma - start));

        auto        space = field.find_last_of(' ');
        std::string type  = rewriteType(field.substr(0, space), exprBaseName);
        std::string name  = field.substr(space + 1);

        out << "    " << type << " " << name << ";\n";

        if (comma == std::string::npos) break;
        start = comma + 1;
    }

    out << "};\n\n";
}

static void defineExprWrapper(std::ofstream&                  out,
                              const std::string&              exprBaseName,
                              const std::vector<std::string>& types) {
    out << "struct " << exprBaseName << " {\n";
    out << "    using Variant = std::variant<";
    out << "Nil" << exprBaseName;
    if (types.size()) out << ", ";
    for (size_t i = 0; i < types.size(); ++i) {
        auto colon     = types[i].find(':');
        auto className = trim(types[i].substr(0, colon));
        out << className;
        if (i + 1 < types.size()) out << ", ";
    }

    out << ">;\n\n";
    out << "    Variant node;\n";
    out << "};\n\n";
}

// ---------------- Main Generator ----------------
void DefineAst(const std::string& exprBaseName, const std::string& outputDir,
               const std::vector<std::string>& types) {
    std::string   path = outputDir + "/" + exprBaseName + ".hpp";
    std::ofstream out(path);

    if (!out.is_open()) {
        std::cerr << std::format("Unable to open file {}\n", path);
        std::exit(1);
    }

    out << "#pragma once\n\n";
    out << "#include <memory>\n";
    out << "#include <variant>\n\n";
    out << "#include \"popl/lexer/token.hpp\"\n";
    out << "#include \"popl/literal.hpp\"\n\n";

    out << "namespace popl {\n\n";

    // forward declaration for unique_ptr
    out << "struct " << exprBaseName << ";\n\n";
    defineNilType(out, exprBaseName);
    for (const auto& type : types) {
        defineType(out, type, exprBaseName);
    }

    defineExprWrapper(out, exprBaseName, types);
    defineVisitor(out, exprBaseName, types);

    out << "} // namespace popl\n";
}
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: ast_gen  <output_directory>\n";
        std::cout << "Example: ast_gen ./generated\n";
        std::cout << "         ast_gen ./generated\n";
        return 64;
    }

    std::string exprBaseName{"Expr"};
    std::string outputDir = argv[1];

    std::vector<std::string> ExprTypes = {
        "Binary" + exprBaseName + "   : " + exprBaseName +
            "* left, Token op, " + exprBaseName + "* right",
        "Ternary" + exprBaseName + "  : " + exprBaseName +
            "* condition, Token question, " + exprBaseName +
            "* thenBranch, Token colon, " + exprBaseName + "* elseBranch",
        "Grouping" + exprBaseName + " : " + exprBaseName + "* expression",
        "Literal" + exprBaseName + "  : PopLObject value",
        "Unary" + exprBaseName + "    : Token op, " + exprBaseName + "* right",
        "Variable" + exprBaseName + "    : Token name"};

    std::string              stmtBaseName{"Stmt"};
    std::vector<std::string> StmtTypes = {
        "Block" + stmtBaseName + " : std::vector<Stmt> statements",
        "Expression" + stmtBaseName + " : Expr* expression",
        "Print" + stmtBaseName + ": Expr* expression",
        "Var" + stmtBaseName + ": Token name, Expr* initializer",
        "Assign" + stmtBaseName + ": Token name, " + exprBaseName + "* value"};

    DefineAst(exprBaseName, outputDir, ExprTypes);
    DefineAst(stmtBaseName, outputDir, StmtTypes);
}
