/*
    SPDX-License-Identifier: Apache-2.0

    Copyright 2024 Yingwei Zheng
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
        http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "CClangPass.hpp"
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/LLVMContext.h>

class CClangPass : public PassInfoMixin<CClangPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
    for (auto &F : M) {
      if (!F.isIntrinsic()) {
        F.removeFnAttr(Attribute::AllocSize);
        F.removeFnAttr(Attribute::AllocKind);
        F.removeFnAttr("alloc-family");
        F.removeFnAttr(Attribute::NoFree);
        F.removeFnAttr(Attribute::NoRecurse);
        F.removeFnAttr(Attribute::NoSync);
        F.removeFnAttr(Attribute::NoUnwind);
        F.removeFnAttr(Attribute::WillReturn);
        F.removeFnAttr(Attribute::MustProgress);
        F.setMemoryEffects(MemoryEffects::unknown());
        F.removeFnAttr(Attribute::NoReturn);
        F.addFnAttr(Attribute::NullPointerIsValid);
        // if (F.isDeclaration())
        //   F.addFnAttr(Attribute::NoBuiltin);
        F.addFnAttr(Attribute::NoImplicitFloat);
        F.addFnAttr(Attribute::NoMerge);
        // F.addFnAttr(Attribute::SpeculativeLoadHardening);
      }
      const Attribute::AttrKind Attrs[] = {Attribute::NoUndef,
                                           Attribute::NonNull,
                                           Attribute::Dereferenceable,
                                           Attribute::DereferenceableOrNull,
                                           Attribute::NoAlias,
                                           Attribute::NoCapture,
                                           Attribute::Range,
                                           Attribute::Alignment,
                                           Attribute::NoFree,
                                           Attribute::Returned,
                                           Attribute::NoFPClass,
                                           Attribute::AllocAlign,
                                           Attribute::AllocatedPointer,
                                           Attribute::ReadNone,
                                           Attribute::ReadOnly,
                                           Attribute::WriteOnly,
                                           Attribute::Writable,
                                           Attribute::Initializes,
                                           Attribute::DeadOnUnwind};
      for (auto Attr : Attrs) {
        F.removeRetAttr(Attr);
        for (auto &Arg : F.args())
          Arg.removeAttr(Attr);
      }

      for (auto &BB : F) {
        for (auto &I : make_early_inc_range(BB)) {
          if (auto *II = dyn_cast<IntrinsicInst>(&I)) {
            switch (II->getIntrinsicID()) {
            case Intrinsic::assume:
            case Intrinsic::lifetime_end: // workaround for stack coloring bugs
            case Intrinsic::invariant_start:
            case Intrinsic::invariant_end:
            case Intrinsic::experimental_noalias_scope_decl:
              II->eraseFromParent();
              continue;
            default:
              break;
            }
          }
          if (auto *CB = dyn_cast<CallBase>(&I)) {
            const uint32_t ArgSize = CB->arg_size();
            for (uint32_t I = 0; I != ArgSize; ++I)
              CB->removeParamAttr(I, Attribute::NonNull);
          }

          I.dropPoisonGeneratingAnnotations();
          I.dropUBImplyingAttrsAndMetadata();
          I.eraseMetadataIf([](unsigned MDKind, MDNode *Node) {
            switch (MDKind) {
            case LLVMContext::MD_tbaa:
            case LLVMContext::MD_fpmath:
            case LLVMContext::MD_range:
            case LLVMContext::MD_tbaa_struct:
            case LLVMContext::MD_invariant_load:
            case LLVMContext::MD_alias_scope:
            case LLVMContext::MD_noalias:
            case LLVMContext::MD_mem_parallel_loop_access:
            case LLVMContext::MD_nonnull:
            case LLVMContext::MD_dereferenceable:
            case LLVMContext::MD_dereferenceable_or_null:
            case LLVMContext::MD_align:
            case LLVMContext::MD_loop:
            case LLVMContext::MD_type:
            case LLVMContext::MD_irr_loop:
            case LLVMContext::MD_access_group:
            case LLVMContext::MD_noundef:
              return true;
            default:
              return false;
            }
          });
        }
      }
    }
    return PreservedAnalyses::none();
  }
};

void addCClangPasses(ModulePassManager &PM) { PM.addPass(CClangPass()); }
