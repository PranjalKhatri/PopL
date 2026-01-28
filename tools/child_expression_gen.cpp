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

static std::string rewriteType(const std::string& type) {
    if (type == "Expr" || type == "Expr*") {
        return "std::unique_ptr<Expr>";
    }
    return type;
}

static void defineVisitor(std::ofstream&                  out,
                          const std::vector<std::string>& types) {
    out << "struct ExprVisitor {\n";
    out << "    virtual ~ExprVisitor() = default;\n";
    for (const auto& type : types) {
        auto        colon     = type.find(':');
        std::string className = trim(type.substr(0, colon));
        out << "    virtual void operator()(" << className << "& expr) = 0;\n";
    }
    out << "};\n\n";
}

static void defineType(std::ofstream& out, const std::string& spec) {
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
        std::string type  = rewriteType(field.substr(0, space));
        std::string name  = field.substr(space + 1);

        out << "    " << type << " " << name << ";\n";

        if (comma == std::string::npos) break;
        start = comma + 1;
    }

    out << "};\n\n";
}

static void defineExprWrapper(std::ofstream&                  out,
                              const std::vector<std::string>& types) {
    out << "struct Expr {\n";
    out << "    using Variant = std::variant<";

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

static void defineVisitHelper(std::ofstream& out) {
    out << "inline void visitExpr(Expr& expr, ExprVisitor& visitor) {\n";
    out << "    std::visit(visitor, expr);\n";
    out << "}\n\n";
}

// ---------------- Main Generator ----------------
void DefineExprAst(const std::string&              outputDir,
                   const std::vector<std::string>& types) {
    std::string   path = outputDir + "/Expr.hpp";
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
    out << "struct Expr;\n\n";

    for (const auto& type : types) {
        defineType(out, type);
    }

    defineExprWrapper(out, types);
    defineVisitor(out, types);
    defineVisitHelper(out);

    out << "} // namespace popl\n";
}

// ---------------- Entrypoint ----------------
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: expr_gen <output_directory>\n";
        return 64;
    }

    std::vector<std::string> types = {
        "BinaryExpr   : Expr left, Token op, Expr right",
        "GroupingExpr : Expr expression",
        "LiteralExpr  : PopLObject value",
        "UnaryExpr    : Token op, Expr right",
    };

    DefineExprAst(argv[1], types);
}
