#include "popl/syntax/visitors/clone_visitor.hpp"

namespace popl {

struct ExprCloner;
struct StmtCloner;

// Expression Cloner
struct ExprCloner {
    Expr operator()(const NilExpr&) const { return Expr{NilExpr{}}; }

    Expr operator()(const BinaryExpr& e) const {
        return Expr{BinaryExpr{
            e.left ? std::make_unique<Expr>(Clone(*e.left)) : nullptr, e.op,
            e.right ? std::make_unique<Expr>(Clone(*e.right)) : nullptr}};
    }

    Expr operator()(const TernaryExpr& e) const {
        return Expr{TernaryExpr{
            e.condition ? std::make_unique<Expr>(Clone(*e.condition)) : nullptr,
            e.question,
            e.thenBranch ? std::make_unique<Expr>(Clone(*e.thenBranch))
                         : nullptr,
            e.colon,
            e.elseBranch ? std::make_unique<Expr>(Clone(*e.elseBranch))
                         : nullptr}};
    }

    Expr operator()(const GroupingExpr& e) const {
        return Expr{GroupingExpr{
            e.expression ? std::make_unique<Expr>(Clone(*e.expression))
                         : nullptr}};
    }

    Expr operator()(const LiteralExpr& e) const {
        return Expr{LiteralExpr{e.value}};
    }

    Expr operator()(const UnaryExpr& e) const {
        return Expr{UnaryExpr{
            e.op, e.right ? std::make_unique<Expr>(Clone(*e.right)) : nullptr}};
    }

    Expr operator()(const CallExpr& e) const {
        std::vector<std::unique_ptr<Expr>> args;
        args.reserve(e.arguments.size());

        for (const auto& arg : e.arguments) {
            args.push_back(arg ? std::make_unique<Expr>(Clone(*arg)) : nullptr);
        }

        return Expr{CallExpr{
            e.callee ? std::make_unique<Expr>(Clone(*e.callee)) : nullptr,
            e.ClosingParen, std::move(args)}};
    }

    Expr operator()(const VariableExpr& e) const {
        return Expr{VariableExpr{e.name}};
    }

    Expr operator()(const LogicalExpr& e) const {
        return Expr{LogicalExpr{
            e.left ? std::make_unique<Expr>(Clone(*e.left)) : nullptr, e.op,
            e.right ? std::make_unique<Expr>(Clone(*e.right)) : nullptr}};
    }

    Expr operator()(const FunctionExpr& e) const {
        std::vector<std::unique_ptr<Stmt>> body;
        body.reserve(e.body.size());

        for (const auto& stmt : e.body) {
            body.push_back(stmt ? std::make_unique<Stmt>(Clone(*stmt))
                                : nullptr);
        }

        return Expr{FunctionExpr{e.params, std::move(body)}};
    }
};

Expr Clone(const Expr& expr) { return visitExpr(expr, ExprCloner{}); }

// Statement Cloner
struct StmtCloner {
    Stmt operator()(const NilStmt&) const { return Stmt{NilStmt{}}; }

    Stmt operator()(const BlockStmt& s) const {
        std::vector<std::unique_ptr<Stmt>> stmts;
        stmts.reserve(s.statements.size());

        for (const auto& stmt : s.statements) {
            stmts.push_back(stmt ? std::make_unique<Stmt>(Clone(*stmt))
                                 : nullptr);
        }

        return Stmt{BlockStmt{std::move(stmts)}};
    }

    Stmt operator()(const ExpressionStmt& s) const {
        return Stmt{ExpressionStmt{
            s.expression ? std::make_unique<Expr>(Clone(*s.expression))
                         : nullptr}};
    }

    Stmt operator()(const PrintStmt& s) const {
        return Stmt{PrintStmt{s.expression
                                  ? std::make_unique<Expr>(Clone(*s.expression))
                                  : nullptr}};
    }

    Stmt operator()(const VarStmt& s) const {
        return Stmt{VarStmt{s.name, s.initializer ? std::make_unique<Expr>(
                                                        Clone(*s.initializer))
                                                  : nullptr}};
    }

    Stmt operator()(const AssignStmt& s) const {
        return Stmt{
            AssignStmt{s.name, s.value ? std::make_unique<Expr>(Clone(*s.value))
                                       : nullptr}};
    }

    Stmt operator()(const IfStmt& s) const {
        return Stmt{IfStmt{
            s.condition ? std::make_unique<Expr>(Clone(*s.condition)) : nullptr,
            s.thenBranch ? std::make_unique<Stmt>(Clone(*s.thenBranch))
                         : nullptr,
            s.elseBranch ? std::make_unique<Stmt>(Clone(*s.elseBranch))
                         : nullptr}};
    }

    Stmt operator()(const WhileStmt& s) const {
        return Stmt{WhileStmt{
            s.condition ? std::make_unique<Expr>(Clone(*s.condition)) : nullptr,
            s.body ? std::make_unique<Stmt>(Clone(*s.body)) : nullptr}};
    }

    Stmt operator()(const BreakStmt& s) const { return Stmt{s}; }

    Stmt operator()(const ContinueStmt& s) const { return Stmt{s}; }

    Stmt operator()(const FunctionStmt& s) const {
        std::vector<std::unique_ptr<Stmt>> body;
        body.reserve(s.func->body.size());

        for (const auto& stmt : s.func->body) {
            body.push_back(stmt ? std::make_unique<Stmt>(Clone(*stmt))
                                : nullptr);
        }

        return Stmt{FunctionStmt{s.name, make_unique<FunctionExpr>(
                                             s.func->params, std::move(body))}};
    }

    Stmt operator()(const ReturnStmt& s) const {
        return Stmt{ReturnStmt{
            s.keyword,
            s.value ? std::make_unique<Expr>(Clone(*s.value)) : nullptr}};
    }
};

Stmt Clone(const Stmt& stmt) { return visitStmt(stmt, StmtCloner{}); }

}  // namespace popl
