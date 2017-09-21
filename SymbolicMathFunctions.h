#ifndef SYMBOLICMATH_FUNCTIONS_H
#define SYMBOLICMATH_FUNCTIONS_H

#include <string>
#include <vector>
#include <type_traits>

namespace SymbolicMath
{

enum class FunctionType
{
  ABS,
  ACOS,
  ACOSH,
  ARG,
  ASIN,
  ASINH,
  ATAN,
  ATAN2,
  ATANH,
  CBRT,
  CEIL,
  CONJ,
  COS,
  COSH,
  COT,
  CSC,
  EXP,
  EXP2,
  FLOOR,
  HYPOT,
  IF,
  IMAG,
  INT,
  LOG,
  LOG10,
  LOG2,
  MAX,
  MIN,
  PLOG,
  POLAR,
  POW,
  REAL,
  SEC,
  SIN,
  SINH,
  SQRT,
  TAN,
  TANH,
  TRUNC,

  INVALID
};

struct FunctionProperties
{
  const unsigned short _arguments;
  const std::string _form;
};

static const std::vector<FunctionProperties> _functions = {
    {1, "abs"},   {1, "acos"},  {1, "acosh"}, {1, "arg"},   {1, "asin"},  {1, "asinh"}, {1, "atan"},
    {2, "atan2"}, {1, "atanh"}, {1, "cbrt"},  {1, "ceil"},  {1, "conj"},  {1, "cos"},   {1, "cosh"},
    {1, "cot"},   {1, "csc"},   {1, "exp"},   {1, "exp2"},  {1, "floor"}, {2, "hypot"}, {3, "if"},
    {1, "imag"},  {1, "int"},   {1, "log"},   {1, "log10"}, {1, "log2"},  {2, "max"},   {2, "min"},
    {2, "plog"},  {2, "polar"}, {2, "pow"},   {1, "real"},  {1, "sec"},   {1, "sin"},   {1, "sinh"},
    {1, "sqrt"},  {1, "tan"},   {1, "tanh"},  {1, "trunc"}};

FunctionType identifyFunction(const std::string & op);

inline const FunctionProperties &
functionProperty(FunctionType op)
{
  return _functions[static_cast<int>(op)];
}

// end namespace SymbolicMath
}

#endif // SYMBOLICMATH_FUNCTIONS_H
