#ifndef WFF_H
#define WFF_H

#include <string>
#include <memory>
#include <set>
#include <vector>

class Wff;
typedef std::shared_ptr< Wff > pwff;

class Wff {
public:
  virtual std::string to_string() = 0;
  virtual pwff imp_not_form() = 0;
};

class Var : public Wff {
public:
  Var(std::string name) :
    name(name) {
  }

  std::string to_string() {
    return this->name;
  }

  pwff imp_not_form() {
    return pwff(new Var(this->name));
  }

private:
  std::string name;
};

class Not : public Wff {
public:
  Not(pwff a) :
    a(a) {
  }

  std::string to_string() {
    return "-. " + this->a->to_string();
  }

  pwff imp_not_form() {
    // Already fundamental
    return pwff(new Not(this->a->imp_not_form()));
  }

private:
  pwff a, b;
};

class Imp : public Wff {
public:
  Imp(pwff a, pwff b) :
    a(a), b(b) {
  }

  std::string to_string() {
    return "( " + this->a->to_string() + " -> " + this->b->to_string() + " )";
  }

  pwff imp_not_form() {
    // Already fundamental
    return pwff(new Imp(this->a->imp_not_form(), this->b->imp_not_form()));
  }

private:
  pwff a, b;
};

class Biimp : public Wff {
public:
  Biimp(pwff a, pwff b) :
    a(a), b(b) {
  }

  std::string to_string() {
    return "( " + this->a->to_string() + " <-> " + this->b->to_string() + " )";
  }

  pwff imp_not_form() {
    // Using dfbi1
    auto ain = this->a->imp_not_form();
    auto bin = this->b->imp_not_form();
    return pwff(new Not(pwff(new Imp(pwff(new Imp(ain, bin)), pwff(new Not(pwff(new Imp(bin, ain))))))));
  }

private:
  pwff a, b;
};

class And : public Wff {
public:
  And(pwff a, pwff b) :
    a(a), b(b) {
  }

  std::string to_string() {
    return "( " + this->a->to_string() + " /\\ " + this->b->to_string() + " )";
  }

  pwff imp_not_form() {
    // Using df-an
    return pwff(new Not(pwff(new Imp(this->a->imp_not_form(), pwff(new Not(this->b->imp_not_form()))))));
  }

private:
  pwff a, b;
};

class Or : public Wff {
public:
  Or(pwff a, pwff b) :
    a(a), b(b) {
  }

  std::string to_string() {
    return "( " + this->a->to_string() + " \\/ " + this->b->to_string() + " )";
  }

  pwff imp_not_form() {
    // Using df-or
    return pwff(new Imp(pwff(new Not(this->a->imp_not_form())), this->b->imp_not_form()));
  }

private:
  pwff a, b;
};

class Nand : public Wff {
public:
  Nand(pwff a, pwff b) :
    a(a), b(b) {
  }

  std::string to_string() {
    return "( " + this->a->to_string() + " -/\\ " + this->b->to_string() + " )";
  }

  pwff imp_not_form() {
    // Using df-nan and recurring
    return pwff(new Not(pwff(new Or(this->a->imp_not_form(), this->b->imp_not_form()))))->imp_not_form();
  }

private:
  pwff a, b;
};

class Xor : public Wff {
public:
  Xor(pwff a, pwff b) :
    a(a), b(b) {
  }

  std::string to_string() {
    return "( " + this->a->to_string() + " \\/_ " + this->b->to_string() + " )";
  }

  pwff imp_not_form() {
    // Using df-xor and recurring
    return pwff(new Not(pwff(new Biimp(this->a->imp_not_form(), this->b->imp_not_form()))))->imp_not_form();
  }

private:
  pwff a, b;
};

#endif // WFF_H
