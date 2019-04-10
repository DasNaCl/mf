#pragma once

#include <SourceRange.hpp>
#include <util.hpp>
#include <memory>

class Type : public std::enable_shared_from_this<Type>
{
public:
  using Ptr = std::shared_ptr<Type>;
  
  Type();

  std::uint_fast64_t gid() const;
private:
  static std::uint_fast64_t gid_counter;

  std::uint_fast64_t id;
};

class Statement : public std::enable_shared_from_this<Statement>
{
public:
  using Ptr = std::shared_ptr<Statement>;

  Statement(SourceRange loc);

  std::uint_fast64_t gid() const;
private:
  static std::uint_fast64_t gid_counter;

  std::uint_fast64_t id;
  SourceRange loc;
};

class Identifier : public Statement
{
public:
  Identifier(SourceRange loc, Symbol symbol);

private:
  Symbol symbol;
};

class Var : public Statement
{
public:
  Var(SourceRange range, Identifier::Ptr identifier, Type::Ptr type);
private:
  Identifier::Ptr identifier;
  Type::Ptr type;
};

class Block : public Statement
{
public:
  Block(SourceRange range, const std::vector<Statement::Ptr>& statements);
private:
  std::vector<Statement::Ptr> statements;
};

class Function : public Statement
{
public:
  Function(SourceRange loc, const std::vector<Statement::Ptr>& data);
private:
  std::vector<Statement::Ptr> data;
};
