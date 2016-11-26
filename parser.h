#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <set>
#include <unordered_map>

#include "library.h"

std::vector< std::string > tokenize(std::string in);

/*
 * Some notes on this Metamath parser:
 *  + At the moment it does not support file inclusion. File inclusions
 *    are treated (more or less) like comments.
 *  + It does not require that label tokens do not match any math token.
 *  + It does not support unknown points in proofs.
 */

class FileTokenizer {
public:
    FileTokenizer(std::istream &in);
    std::string next();
private:
    std::istream &in;
    bool white;
    std::vector< char > buf;
    std::string finalize_token();
};

struct ParserStackFrame {
    std::set< SymTok > vars;
    std::set< std::pair< SymTok, SymTok > > dists;     // Must be stored ordered, for example using minmax()
    std::vector< LabTok > types;
    std::vector< LabTok > hyps;
};

class Parser {
public:
    Parser(FileTokenizer &ft);
    void run();
    const Library &get_library() const;

private:
    void parse_c();
    void parse_v();
    void parse_f();
    void parse_e();
    void parse_d();
    void parse_a();
    void parse_p();

    bool check_var(SymTok tok) const;
    bool check_const(SymTok tok) const;
    std::set<SymTok> collect_mand_vars(const std::vector<SymTok> &sent) const;
    void collect_vars(std::set<SymTok> &vars, const std::vector<SymTok> &sent) const;
    std::pair< int, std::vector<LabTok> > collect_mand_hyps(std::set<SymTok> vars) const;
    std::set<std::pair<SymTok, SymTok> > collect_mand_dists(std::set<SymTok> vars) const;

    FileTokenizer &ft;
    Library lib;
    LabTok label;
    std::vector< std::string > toks;

    std::vector< ParserStackFrame > stack;
    //std::set< SymTok > consts;
};

class CompressedEncoder {
public:
    std::string push_code(CodeTok x);
};

class CompressedDecoder {
public:
    CodeTok push_char(char c);
private:
    int current = 0;
};

#endif // PARSER_H
