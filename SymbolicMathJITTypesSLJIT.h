#ifndef SYMBOLICMATHTYPESSLJIT_H
#define SYMBOLICMATHTYPESSLJIT_H

#include "contrib/sljit_src/sljitLir.h"

namespace SymbolicMath
{

const std::string jit_backend_name = "SLJIT";

using JITFunctionPtr = Real SLJIT_FUNC (*)();

using JITReturnValue = void;

struct JITStateValue
{
  /// current stack entry (as array index)
  int sp;

  /// SLJIT compiler context
  struct sljit_compiler * C;
};

// end namespace SymbolicMath
}

#endif // SYMBOLICMATHTYPESSLJIT_H
