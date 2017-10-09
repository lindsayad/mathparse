#include "SymbolicMathNode.h"
#include "SymbolicMathNodeData.h"
#include "SymbolicMathUtils.h"

#include <cmath>
#include <iostream> // debug

namespace SymbolicMath
{

Node::Node() : _data(std::make_shared<EmptyData>()) {}

Node::Node(const Node & copy) : _data(copy._data->clone()) {}

Node::Node(Real val) : _data(new RealNumberData(val)) {}

Node::Node(UnaryOperatorType type, Node arg) : _data(std::make_shared<UnaryOperatorData>(type, arg))
{
}

Node::Node(BinaryOperatorType type, Node arg0, Node arg1)
  : _data(std::make_shared<BinaryOperatorData>(type, arg0, arg1))
{
}

Node::Node(MultinaryOperatorType type, std::vector<Node> args)
  : _data(std::make_shared<MultinaryOperatorData>(type, args))
{
}

Node::Node(UnaryFunctionType type, Node arg) : _data(std::make_shared<UnaryFunctionData>(type, arg))
{
}

Node::Node(BinaryFunctionType type, Node arg0, Node arg1)
  : _data(std::make_shared<BinaryFunctionData>(type, arg0, arg1))
{
}

Node::Node(ConditionalType type, Node arg0, Node arg1, Node arg2)
  : _data(std::make_shared<ConditionalData>(type, arg0, arg1, arg2))
{
}

Node
Node::operator+(Node r)
{
  return Node(MultinaryOperatorType::ADDITION, {*this, r});
}

Node
Node::operator-(Node r)
{
  return Node(BinaryOperatorType::SUBTRACTION, *this, r);
}

Node Node::operator*(Node r) { return Node(MultinaryOperatorType::MULTIPLICATION, {*this, r}); }

Node
Node::operator/(Node r)
{
  return Node(BinaryOperatorType::DIVISION, *this, r);
}

Node Node::operator[](unsigned int i) { return _data->getArg(i); }

Real
Node::value() const
{
  return _data->value();
}

std::string
Node::format() const
{
  return _data->format();
}

std::string
Node::formatTree(std::string indent) const
{
  return _data->formatTree(indent);
}

bool
Node::is(Real t) const
{
  return _data->is(t);
}

bool
Node::is(NumberType t) const
{
  return _data->is(t);
}

bool
Node::is(UnaryOperatorType t) const
{
  return _data->is(t);
}

bool
Node::is(BinaryOperatorType t) const
{
  return _data->is(t);
}

bool
Node::is(MultinaryOperatorType t) const
{
  return _data->is(t);
}

bool
Node::is(UnaryFunctionType t) const
{
  return _data->is(t);
}

bool
Node::is(BinaryFunctionType t) const
{
  return _data->is(t);
}

bool
Node::is(ConditionalType t) const
{
  return _data->is(t);
}

bool
Node::isValid() const
{
  return _data->isValid();
}

Node
Node::D(unsigned int id) const
{
  return Node(_data->D(id));
}

void
Node::simplify()
{
  auto simplified = _data->simplify();
  if (simplified.isValid())
    _data = simplified._data;
}

unsigned short
Node::precedence() const
{
  return _data->precedence();
}

// void
// Node::checkIndex(const std::vector<unsigned int> & index)
// {
//   auto s = shape();
//
//   if (index.size() > s.size())
//     fatalError("Index exceeds object dimensions");
//
//   for (std::size_t i = 0; i < index.size(); ++i)
//     if (index[i] >= s[i])
//       fatalError("Index out of range");
// }

// end namespace SymbolicMath
}