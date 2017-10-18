#ifndef DD_TRACE_H
#define DD_TRACE_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Option/Arg.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/ToolOutputFile.h"
#include "oclint/DDMacroTracker.h"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace dd
{

class DDItem
{
public:
	std::string filePath;
	std::string dependency;
}; 

class DDTraceConsumer : public clang::ASTConsumer {
public:
  DDTraceConsumer(llvm::SmallSet<std::string, 4> &Ignore,
                  std::vector<CallbackCall> &CallbackCalls, clang::Preprocessor &PP, clang::CompilerInstance &Instance) {
    // PP takes ownership.
    PP.addPPCallbacks(llvm::make_unique<DDMacroTracker>(Ignore, CallbackCalls, PP, Instance));
  }
};

class DDTraceAction : public clang::SyntaxOnlyAction {
public:
  DDTraceAction() {};

  void OutputDDItems(std::vector<DDItem> &items) {
  	for (std::vector<CallbackCall>::const_iterator I = CallbackCalls.begin(),
                                                   E = CallbackCalls.end();
         I != E; ++I) 
  	{
  		const CallbackCall &Callback = *I;

  		std::string filePath ;
      std::string dependency;

      for (std::vector<Argument>::const_iterator AI = Callback.Arguments.begin(),
                                                 AE = Callback.Arguments.end();
           AI != AE; ++AI) 
      {
      	const Argument &Arg = *AI;
      	if (Arg.Name == "File")
      	{
          dependency = Arg.Value;
      	}

        if (Arg.Name == "MainFile")
        {
          filePath = Arg.Value;
        }
      }

      DDItem item;
      item.filePath = filePath;
      item.dependency = dependency;
      items.push_back(item);
  	}
  };

protected:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI, StringRef InFile) override {
    return llvm::make_unique<DDTraceConsumer>(Ignore, CallbackCalls,
                                              CI.getPreprocessor(),
                                              CI);
  }

private:
  llvm::SmallSet<std::string, 4> Ignore;
  std::vector<CallbackCall> CallbackCalls;
};

}// end namespace dd

#endif // DD_TRACE_H