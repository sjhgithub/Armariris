//===- Loads.h - Local load analysis --------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares simple local analyses for load instructions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_LOADS_H
#define LLVM_ANALYSIS_LOADS_H

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/CommandLine.h"

namespace llvm {

class DataLayout;
class MDNode;

/// isDereferenceablePointer - Return true if this is always a dereferenceable
/// pointer. If the context instruction is specified perform context-sensitive
/// analysis and return true if the pointer is dereferenceable at the
/// specified instruction.
bool isDereferenceablePointer(const Value *V, const DataLayout &DL,
                              const Instruction *CtxI = nullptr,
                              const DominatorTree *DT = nullptr);

/// Returns true if V is always a dereferenceable pointer with alignment
/// greater or equal than requested. If the context instruction is specified
/// performs context-sensitive analysis and returns true if the pointer is
/// dereferenceable at the specified instruction.
bool isDereferenceableAndAlignedPointer(const Value *V, unsigned Align,
                                        const DataLayout &DL,
                                        const Instruction *CtxI = nullptr,
                                        const DominatorTree *DT = nullptr);

/// isSafeToLoadUnconditionally - Return true if we know that executing a load
/// from this value cannot trap.
///
/// If DT and ScanFrom are specified this method performs context-sensitive
/// analysis and returns true if it is safe to load immediately before ScanFrom.
///
/// If it is not obviously safe to load from the specified pointer, we do a
/// quick local scan of the basic block containing ScanFrom, to determine if
/// the address is already accessed.
bool isSafeToLoadUnconditionally(Value *V, unsigned Align,
                                 const DataLayout &DL,
                                 Instruction *ScanFrom = nullptr,
                                 const DominatorTree *DT = nullptr);

/// DefMaxInstsToScan - the default number of maximum instructions
/// to scan in the block, used by FindAvailableLoadedValue().
extern cl::opt<unsigned> DefMaxInstsToScan;

/// \brief Scan backwards to see if we have the value of the given load
/// available locally within a small number of instructions.
///
/// You can use this function to scan across multiple blocks: after you call
/// this function, if ScanFrom points at the beginning of the block, it's safe
/// to continue scanning the predecessors.
///
/// Note that performing load CSE requires special care to make sure the
/// metadata is set appropriately.  In particular, aliasing metadata needs
/// to be merged.  (This doesn't matter for store-to-load forwarding because
/// the only relevant load gets deleted.)
///
/// \param Load The load we want to replace.
/// \param ScanBB The basic block to scan. FIXME: This is redundant.
/// \param [in,out] ScanFrom The location to start scanning from. When this
/// function returns, it points at the last instruction scanned.
/// \param MaxInstsToScan The maximum number of instructions to scan. If this
/// is zero, the whole block will be scanned.
/// \param AA Optional pointer to alias analysis, to make the scan more
/// precise.
/// \param [out] AATags The aliasing metadata for the operation which produced
/// the value. FIXME: This is basically useless.
/// \param [out] IsLoadCSE Whether the returned value is a load from the same
/// location in memory, as opposed to the value operand of a store.
///
/// \returns The found value, or nullptr if no value is found.
Value *FindAvailableLoadedValue(LoadInst *Load,
                                BasicBlock *ScanBB,
                                BasicBlock::iterator &ScanFrom,
                                unsigned MaxInstsToScan = DefMaxInstsToScan,
                                AliasAnalysis *AA = nullptr,
                                AAMDNodes *AATags = nullptr,
                                bool *IsLoadCSE = nullptr);

}

#endif
