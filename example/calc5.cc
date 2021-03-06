//
//  calc5.cc
//
//  Copyright (c) 2015 Yuji Hirose. All rights reserved.
//  MIT License
//

#include <cstdlib>
#include <iostream>
#include <peglib.h>

using namespace peg;

int main(int argc, const char **argv) {
  if (argc < 2 || std::string("--help") == argv[1]) {
    std::cout << "usage: calc5 [formula]" << std::endl;
    return 1;
  }

  std::function<long(const Ast &)> eval = [&](const Ast &ast) {
    if (ast.name == "NUMBER") {
      return ast.token_to_number<long>();
    } else {
      const auto &nodes = ast.nodes;
      auto result = eval(*nodes[0]);
      if (nodes.size() > 1) {
        auto ope = nodes[1]->token[0];
        auto num = eval(*nodes[2]);
        switch (ope) {
        case '+': result += num; break;
        case '-': result -= num; break;
        case '*': result *= num; break;
        case '/': result /= num; break;
        }
      }
      return result;
    }
  };

  parser parser(R"(
        EXPRESSION  <- ATOM (OPERATOR ATOM)* {
                         precedence
                           L - +
                           L / *
                       }
        ATOM        <- NUMBER / '(' EXPRESSION ')'
        OPERATOR    <- < [-+/*] >
        NUMBER      <- < '-'? [0-9]+ >
        %whitespace <- [ \t\r\n]*
    )");

  parser.enable_ast();

  auto expr = argv[1];
  std::shared_ptr<Ast> ast;
  if (parser.parse(expr, ast)) {
    ast = AstOptimizer(true).optimize(ast);
    std::cout << ast_to_s(ast);
    std::cout << expr << " = " << eval(*ast) << std::endl;
    return 0;
  }

  std::cout << "syntax error..." << std::endl;

  return -1;
}

// vim: et ts=4 sw=4 cin cino={1s ff=unix
