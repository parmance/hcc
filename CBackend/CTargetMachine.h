//===-- CTargetMachine.h - TargetMachine for the C backend ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the TargetMachine that is used by the C backend.
//
//===----------------------------------------------------------------------===//

#ifndef CTARGETMACHINE_H
#define CTARGETMACHINE_H

#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/DataLayout.h"

namespace llvm {

class StructType;

struct CTargetMachine : public TargetMachine {
  CTargetMachine(const Target &T, StringRef TT,
                 StringRef CPU, StringRef FS, const TargetOptions &Options,
                 Reloc::Model RM, CodeModel::Model CM,
                 CodeGenOpt::Level OL)
    : TargetMachine(T, TT, CPU, FS, Options) { }

  virtual bool addPassesToEmitFile(PassManagerBase &PM,
                                   formatted_raw_ostream &Out,
                                   CodeGenFileType FileType,
                                   bool DisableVerify,
                                   AnalysisID StartAfter,
                                   AnalysisID StopAfter);
  
  virtual const DataLayout *getDataLayout() const { return 0; }
};

extern Target TheCBackendTarget;

} // End llvm namespace


#endif
