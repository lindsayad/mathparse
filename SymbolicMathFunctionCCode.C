#include "SymbolicMathFunctionCCode.h"
#include <stdio.h>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>

namespace SymbolicMath
{

Function::~Function()
{
  // destroy context only at function end of life
  // if (_jit_context)
  //   jit_context_destroy(_jit_context);
}

void
Function::compile()
{
  if (_jit_code)
    return;

  // build and lock context
  std::string ccode = "#include <cmath>\nextern \"C\" double F()\n{\n  return ";

  // return the value generated by the expression tree
  bool dummy;
  ccode += _root.jit(dummy) + ";\n}";

  // save to a temporary name and rename only when the file is fully written
  char ctmpname[] = "./tmp_adc_XXXXXX.C";
  int ctmpfile = mkstemps(ctmpname, 2);
  if (ctmpfile == -1)
    fatalError("Error creating tmp file " + std::string(ctmpname));
  write(ctmpfile, ccode.data(), ccode.length());
  close(ctmpfile);

  // compile code file
  char otmpname[] = "./tmp_adc_XXXXXX.so";
  int otmpfile = mkstemps(otmpname, 3);
  if (otmpfile == -1)
    fatalError("Error creating tmp file " + std::string(otmpname));
  close(otmpfile);
#if defined(__GNUC__) && defined(__APPLE__) && !defined(__INTEL_COMPILER)
  // gcc on OSX does neither need nor accept the  -rdynamic switch
  std::string command = CCODE_JIT_COMPILER " -std=c++11 -O2 -shared -fPIC ";
#else
  std::string command = CCODE_JIT_COMPILER " -std=c++11 -O2 -shared -rdynamic -fPIC ";
#endif
  command += std::string(ctmpname) + " -o " + std::string(otmpname);
  system(command.c_str());
  std::remove(ctmpname);

  // load object file in
  auto lib = dlopen(otmpname, RTLD_NOW);
  if (!lib)
  {
    std::cerr << "JIT object load failed.\n";
    std::remove(otmpname);
    return;
  }

  // fetch function pointer
  _jit_code = reinterpret_cast<JITFunctionPtr>(dlsym(lib, "F"));
  const char * error = dlerror();
  if (error)
  {
    std::cerr << "Error binding JIT compiled function\n" << error << '\n';
    _jit_code = NULL;
    std::remove(otmpname);
    return;
  }

  std::remove(otmpname);
}

// end namespace SymbolicMath
}
