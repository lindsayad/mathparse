#ifndef SYMBOLICMATHNODEDATA_H
#define SYMBOLICMATHNODEDATA_H

#include <vector>
#include <memory>
#include <array>
#include <cmath>
#include <type_traits>

#include "SymbolicMathNode.h"
#include "SymbolicMathJITTypes.h"

namespace SymbolicMath
{

class Parser;
class ValueProvider;

/**
 * Node data base class. This class defines a common interface for all node data
 * objects. Node data holds the semantic content of a Node object. Node data
 * utilizes polymorphism to store a variety of different node behaviors.
 */
class NodeData
{
public:
  virtual ~NodeData() {}

  virtual Real value() = 0;
  virtual JITReturnValue jit(JITStateValue & state) = 0;

  virtual std::string format() const = 0;
  virtual std::string formatTree(std::string indent) const = 0;

  virtual NodeDataPtr clone() = 0;

  virtual Node getArg(unsigned int i) = 0;
  virtual std::size_t size() { return 0; }

  virtual bool is(Real) const { return false; };
  virtual bool is(NumberType) const { return false; };
  virtual bool is(UnaryOperatorType) const { return false; };
  virtual bool is(BinaryOperatorType) const { return false; };
  virtual bool is(MultinaryOperatorType) const { return false; };
  virtual bool is(UnaryFunctionType) const { return false; };
  virtual bool is(BinaryFunctionType) const { return false; };
  virtual bool is(ConditionalType) const { return false; };
  virtual bool is(IntegerPowerType) const { return false; };
  virtual bool is(ValueProvider * a) const { return false; }

  virtual bool isValid() const { return true; };

  // virtual Shape shape() = 0;
  virtual Shape shape() { return {1}; };

  // virtual void checkIndex(const std::vector<unsigned int> & index);

  virtual Node simplify() { return Node(); };
  virtual Node D(const ValueProvider & vp) = 0;

  virtual unsigned short precedence() const { return 0; }

  /// amount of net stack pointer movement of this operator
  virtual void stackDepth(std::pair<int, int> & current_max);

  friend Node;
};

/**
 * Data class for empty invalid nodes that are constructed using the Node()
 * default constructor. Invalid nodes are returned by NodeData::simplify()
 * when no simplification can be performed.
 */
class EmptyData : public NodeData
{
public:
  bool isValid() const override { return false; };

  Real value() override { fatalError("invalid node"); };
  JITReturnValue jit(JITStateValue & state) override { fatalError("invalid node"); };

  std::string format() const override { fatalError("invalid node"); };
  std::string formatTree(std::string indent) const override { fatalError("invalid node"); };
  NodeDataPtr clone() override { fatalError("invalid node"); };
  Node getArg(unsigned int i) override { fatalError("invalid node"); }
  Node D(const ValueProvider & vp) override { fatalError("invalid node"); }
};

/**
 * Base class template for NodeData objects with exactly N child nodes
 */
template <typename Enum, std::size_t N>
class FixedArgumentData : public NodeData
{
public:
  template <typename... Args, typename = typename std::enable_if<N == sizeof...(Args), void>::type>
  FixedArgumentData(Enum type, Args &&... args) : _type(type), _args({{args...}})
  {
  }

  Node getArg(unsigned int i) override { return _args[i]; };
  std::size_t size() override { return N; }

  bool is(Enum type) const override { return _type == type || type == Enum::_ANY; }
  void stackDepth(std::pair<int, int> & current_max) override
  {
    for (auto & arg : _args)
      arg.stackDepth(current_max);
    current_max.first -= N - 1;
  }

protected:
  Enum _type;
  std::array<Node, N> _args;
};

/**
 * Base class template for NodeData objects with an arbitrary number of child nodes
 */
template <typename Enum>
class MultinaryData : public NodeData
{
public:
  MultinaryData(Enum type, std::vector<Node> args) : _type(type), _args(args) {}

  Node getArg(unsigned int i) override { return _args[i]; };
  std::size_t size() override { return _args.size(); }

  bool is(Enum type) const override { return _type == type || type == Enum::_ANY; }
  void stackDepth(std::pair<int, int> & current_max) override
  {
    for (auto & arg : _args)
      arg.stackDepth(current_max);
    current_max.first -= _args.size() - 1;
  }

protected:
  Enum _type;
  std::vector<Node> _args;
};

/**
 * Base class for any childless node that can be evaluated directly
 */
class ValueProvider : public NodeData
{
public:
  ValueProvider(const std::string & name) : _name(name) {}
  Node getArg(unsigned int i) override { fatalError("Node has no arguments"); };

  std::string format() const override { return _name != "" ? _name : "{V}"; }
  std::string formatTree(std::string indent) const override;

  // used to determine if two value providers are of the same type
  bool is(ValueProvider * a) const override { return getTypeID() == a->getTypeID(); }

  void stackDepth(std::pair<int, int> & current_max) override { current_max.first++; }

protected:
  std::string _name;

  // we roll our own typeid system to avoid relying on RTTI
  virtual void * getTypeID() const = 0;

  /// The parser needs to be able to read the name of the object upon registration
  friend Parser;
};

/**
 * All actual value providers need to inherit from this helper template
 * which implements the custom typeid system.
 */
template <class T>
class ValueProviderDerived : public ValueProvider
{
public:
protected:
  ValueProviderDerived(const std::string & name) : ValueProvider(name) { (void)_vp_typeinfo_tag; }
  virtual void * getTypeID() const { return reinterpret_cast<void *>(&_vp_typeinfo_tag); };

private:
  static int _vp_typeinfo_tag;
};
template <class T>
int ValueProviderDerived<T>::_vp_typeinfo_tag;

/**
 * Purely symbolic node that cannot be evaluated or compiled by substitted and
 * derived w.r.t.
 */
class SymbolData : public ValueProviderDerived<SymbolData>
{
public:
  SymbolData(const std::string & name) : ValueProviderDerived<SymbolData>(name) {}

  Real value() override { fatalError("Node cannot be evaluated"); }
  JITReturnValue jit(JITStateValue & state) override { fatalError("Node cannot be compiled"); }

  NodeDataPtr clone() override { return std::make_shared<SymbolData>(_name); };

  Node D(const ValueProvider & vp) override;
};

/**
 * Simple value provider that fetches its contents from a referenced Real value
 */
class RealReferenceData : public ValueProviderDerived<RealReferenceData>
{
public:
  RealReferenceData(const Real & ref, const std::string & name = "")
    : ValueProviderDerived<RealReferenceData>(name), _ref(ref)
  {
  }

  Real value() override { return _ref; };
  JITReturnValue jit(JITStateValue & state) override;

  NodeDataPtr clone() override { return std::make_shared<RealReferenceData>(_ref, _name); };

  Node D(const ValueProvider & vp) override;

protected:
  const Real & _ref;
};

/**
 * Simple value provider that fetches its contents from a referenced Real array value
 * and a referenced index variable
 */
class RealArrayReferenceData : public ValueProviderDerived<RealArrayReferenceData>
{
public:
  RealArrayReferenceData(const Real & ref, const int & index, const std::string & name = "")
    : ValueProviderDerived<RealArrayReferenceData>(name), _ref(ref), _index(index)
  {
  }

  Real value() override { return (&_ref)[_index]; };
  JITReturnValue jit(JITStateValue & state) override;

  NodeDataPtr clone() override
  {
    return std::make_shared<RealArrayReferenceData>(_ref, _index, _name);
  };

  Node D(const ValueProvider & vp) override;

protected:
  const Real & _ref;
  const int & _index;
};

/**
 * Base class for any childless nodes that represent a constant quantity
 */
class NumberData : public NodeData
{
public:
  Shape shape() override { return {1}; }

  Node getArg(unsigned int i) override { fatalError("Node has no arguments"); };

  Node D(const ValueProvider &) override { return Node(0.0); }

  void stackDepth(std::pair<int, int> & current_max) override { current_max.first++; }

protected:
  NumberType _type;
};

/**
 * Floating point constant node
 */
class RealNumberData : public NumberData
{
public:
  RealNumberData(Real value) : NumberData(), _value(value) {}

  Real value() override { return _value; };
  JITReturnValue jit(JITStateValue & state) override;

  std::string format() const override { return stringify(_value); };
  std::string formatTree(std::string indent) const override;

  NodeDataPtr clone() override { return std::make_shared<RealNumberData>(_value); };

  bool is(NumberType type) const override;
  bool is(Real value) const override { return value == _value; };

  void setValue(Real value) { _value = value; }

protected:
  Real _value;
};

/**
 * Operators o of the form 'oA'
 */
class UnaryOperatorData : public FixedArgumentData<UnaryOperatorType, 1>
{
  using FixedArgumentData<UnaryOperatorType, 1>::FixedArgumentData;

public:
  Real value() override;
  JITReturnValue jit(JITStateValue & state) override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(const ValueProvider &) override;

  unsigned short precedence() const override { return 3; }
};

/**
 * Operators o of the form 'A o B'
 */
class BinaryOperatorData : public FixedArgumentData<BinaryOperatorType, 2>
{
  using FixedArgumentData<BinaryOperatorType, 2>::FixedArgumentData;

public:
  Real value() override;
  JITReturnValue jit(JITStateValue & state) override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(const ValueProvider &) override;

  unsigned short precedence() const override;
};

/**
 * Operators o of the form 'A o B o B o C o D ... o Z'
 */
class MultinaryOperatorData : public MultinaryData<MultinaryOperatorType>
{
  using MultinaryData<MultinaryOperatorType>::MultinaryData;

public:
  Real value() override;
  JITReturnValue jit(JITStateValue & state) override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(const ValueProvider & vp) override;

  unsigned short precedence() const override;

private:
  void simplifyHelper(std::vector<Node> & new_args, Node arg);
};

/**
 * Functions o of the form 'F(A)'
 */
class UnaryFunctionData : public FixedArgumentData<UnaryFunctionType, 1>
{
  using FixedArgumentData<UnaryFunctionType, 1>::FixedArgumentData;

public:
  Real value() override;
  JITReturnValue jit(JITStateValue & state) override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(const ValueProvider &) override;

  unsigned short precedence() const override { return 3; }
};

/**
 * Functions o of the form 'F(A,B)'
 */
class BinaryFunctionData : public FixedArgumentData<BinaryFunctionType, 2>
{
  using FixedArgumentData<BinaryFunctionType, 2>::FixedArgumentData;

public:
  Real value() override;
  JITReturnValue jit(JITStateValue & state) override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(const ValueProvider &) override;
};

/**
 * Binary branch if(A, B, C). The condition A is evaluated first and iff A is
 * true B is evaluates otherwise C is evaluated.
 */
class ConditionalData : public FixedArgumentData<ConditionalType, 3>
{
  using FixedArgumentData<ConditionalType, 3>::FixedArgumentData;

public:
  Real value() override;
  JITReturnValue jit(JITStateValue & state) override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr clone() override;

  Node simplify() override;
  Node D(const ValueProvider &) override;

  void stackDepth(std::pair<int, int> & current_max) override;
};

/**
 * Integer power class for faster JIT code generation
 */
class IntegerPowerData : public NodeData
{
public:
  IntegerPowerData(Node arg, int exponent) : NodeData(), _arg(arg), _exponent(exponent) {}

  Real value() override { return std::pow(_arg.value(), Real(_exponent)); };
  JITReturnValue jit(JITStateValue & state) override;

  std::string format() const override;
  std::string formatTree(std::string indent) const override;

  NodeDataPtr clone() override { return std::make_shared<IntegerPowerData>(_arg, _exponent); };

  Node getArg(unsigned int i) override;
  std::size_t size() override { return 1; }

  bool is(IntegerPowerType) const override { return true; };

  Node simplify() override;
  Node D(const ValueProvider &) override;

  void stackDepth(std::pair<int, int> & current_max) override { _arg.stackDepth(current_max); }

protected:
  Node _arg;
  int _exponent;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHNODEDATA_H
