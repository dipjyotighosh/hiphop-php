/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <compiler/construct.h>
#include <compiler/parser/parser.h>
#include <util/util.h>
#include <compiler/analysis/file_scope.h>
#include <compiler/analysis/function_scope.h>
#include <compiler/analysis/class_scope.h>
#include <compiler/analysis/analysis_result.h>

#include <compiler/expression/simple_function_call.h>
#include <compiler/expression/simple_variable.h>
#include <iomanip>

using namespace HPHP;
using namespace std;

///////////////////////////////////////////////////////////////////////////////

int Construct::s_effectsTag = 1;

Construct::Construct(BlockScopePtr scope, LocationPtr loc)
    : m_blockScope(scope), m_loc(loc),
      m_fileLevel(false), m_topLevel(false),
      m_containedEffects(0), m_effectsTag(0) {
}

void Construct::resetScope(BlockScopePtr scope) {
  setBlockScope(scope);
  for (int i = 0, n = getKidCount(); i < n; i++) {
    if (ConstructPtr kid = getNthKid(i)) {
      if (StatementPtr s = boost::dynamic_pointer_cast<Statement>(kid)) {
        switch (s->getKindOf()) {
          case Statement::KindOfClassStatement:
          case Statement::KindOfInterfaceStatement:
          case Statement::KindOfMethodStatement:
          case Statement::KindOfFunctionStatement:
            continue;
          default:
            break;
        }
      }
      kid->resetScope(scope);
    }
  }
}

int Construct::getChildrenEffects() const {
  int childrenEffects = NoEffect;
  for (int i = getKidCount(); i--; ) {
    ConstructPtr child = getNthKid(i);
    if (child) {
      childrenEffects |= child->getContainedEffects();
      if ((childrenEffects & UnknownEffect) == UnknownEffect) {
        break;
      }
    }
  }
  return childrenEffects;
}

int Construct::getContainedEffects() const {
  if (m_effectsTag != s_effectsTag) {
    m_effectsTag = s_effectsTag;
    m_containedEffects = getLocalEffects() | getChildrenEffects();
  }
  return m_containedEffects;
}

ExpressionPtr Construct::makeConstant(AnalysisResultPtr ar,
                                      const std::string &value) const {
  return Expression::MakeConstant(ar, getScope(), getLocation(), value);
}

ExpressionPtr Construct::makeScalarExpression(AnalysisResultPtr ar,
                                               const Variant &value) const {
  return Expression::MakeScalarExpression(ar, getScope(), getLocation(), value);
}

std::string Construct::getText(bool useCache /* = false */,
                               bool translate /* = false */,
                               AnalysisResultPtr ar
                               /* = AnalysisResultPtr() */) {
  std::string &text = m_text;
  if (useCache && !text.empty()) return text;
  ostringstream o;
  CodeGenerator cg(&o, CodeGenerator::PickledPHP);
  cg.translatePredefined(translate);
  outputPHP(cg, ar);
  text = o.str();
  return text;
}

void Construct::serialize(JSON::OutputStream &out) const {
  JSON::ListStream ls(out);
  ls << m_loc->file << m_loc->line0 << m_loc->char0 <<
                       m_loc->line1 << m_loc->char1;
  ls.done();
}

void Construct::addUserFunction(AnalysisResultPtr ar,
                                const std::string &name) {
  if (!name.empty()) {
    if (ar->getPhase() == AnalysisResult::AnalyzeAll ||
        ar->getPhase() == AnalysisResult::AnalyzeFinal) {
      FunctionScopePtr func = getFunctionScope();
      getFileScope()->addFunctionDependency(ar, name, func &&
                                            func->isInlined());
    }
  }
}

void Construct::addUserClass(AnalysisResultPtr ar,
                             const std::string &name) {
  if (!name.empty()) {
    if (ar->getPhase() == AnalysisResult::AnalyzeAll ||
        ar->getPhase() == AnalysisResult::AnalyzeFinal) {
      getFileScope()->addClassDependency(ar, name);
    }
  }
}

void Construct::printSource(CodeGenerator &cg) {
  if (m_loc) {
    cg_printf("/* SRC: %s line %d */\n", m_loc->file, m_loc->line0);
  }
}

void Construct::dump(int spc, AnalysisResultPtr ar) {
  int nkid = getKidCount();
  std::string name;
  int type = 0;
  std::string scontext = "";
  std::string value = "";
  std::string type_info = "";
  unsigned id = 0;
  ExpressionPtr idPtr = ExpressionPtr();

  if (Statement *s = dynamic_cast<Statement*>(this)) {
    Statement::KindOf stype = s->getKindOf();
    switch (stype) {
    case Statement::KindOfFunctionStatement:
      name="FunctionStatement";
      break;
    case Statement::KindOfClassStatement:
      name="ClassStatement";
      break;
    case Statement::KindOfInterfaceStatement:
      name="InterfaceStatement";
      break;
    case Statement::KindOfClassVariable:
      name="ClassVariable";
      break;
    case Statement::KindOfClassConstant:
      name="ClassConstant";
      break;
    case Statement::KindOfMethodStatement:
      name="MethodStatement";
      break;
    case Statement::KindOfStatementList:
      name="StatementList";
      break;
    case Statement::KindOfBlockStatement:
      name="BlockStatement";
      break;
    case Statement::KindOfIfBranchStatement:
      name="IfBranchStatement";
      break;
    case Statement::KindOfIfStatement:
      name="IfStatement";
      break;
    case Statement::KindOfWhileStatement:
      name="WhileStatement";
      break;
    case Statement::KindOfDoStatement:
      name="DoStatement";
      break;
    case Statement::KindOfForStatement:
      name="ForStatement";
      break;
    case Statement::KindOfSwitchStatement:
      name="SwitchStatement";
      break;
    case Statement::KindOfCaseStatement:
      name="CaseStatement";
      break;
    case Statement::KindOfBreakStatement:
      name="BreakStatement";
      break;
    case Statement::KindOfContinueStatement:
      name="ContinueStatement";
      break;
    case Statement::KindOfReturnStatement:
      name="ReturnStatement";
      break;
    case Statement::KindOfGlobalStatement:
      name="GlobalStatement";
      break;
    case Statement::KindOfStaticStatement:
      name="StaticStatement";
      break;
    case Statement::KindOfEchoStatement:
      name="EchoStatement";
      break;
    case Statement::KindOfUnsetStatement:
      name="UnsetStatement";
      break;
    case Statement::KindOfExpStatement:
      name="ExpStatement";
      break;
    case Statement::KindOfForEachStatement:
      name="ForEachStatement";
      break;
    case Statement::KindOfCatchStatement:
      name="CatchStatement";
      break;
    case Statement::KindOfTryStatement:
      name="TryStatement";
      break;
    case Statement::KindOfThrowStatement:
      name="ThrowStatement";
      break;
    }
    type = (int)stype;
  } else if (Expression *e = dynamic_cast<Expression*>(this)) {
    id = e->getCanonID();
    idPtr = e->getCanonPtr();

    Expression::KindOf etype = e->getKindOf();
    switch (etype) {
    case Expression::KindOfSimpleFunctionCall:
      name="SimpleFunctionCall";
      value = static_cast<SimpleFunctionCall*>(e)->getName();
      break;
    case Expression::KindOfSimpleVariable:
      name="SimpleVariable";
      value = static_cast<SimpleVariable*>(e)->getName();
      break;
    case Expression::KindOfConstantExpression:
      name="ConstantExpression";
      value = e->getText();
      break;
    case Expression::KindOfScalarExpression:
      name="ScalarExpression";
      value = e->getText();
      break;

    case Expression::KindOfExpressionList:
      name="ExpressionList";
      break;
    case Expression::KindOfAssignmentExpression:
      name="AssignmentExpression";
      break;
    case Expression::KindOfDynamicVariable:
      name="DynamicVariable";
      break;
    case Expression::KindOfStaticMemberExpression:
      name="StaticMemberExpression";
      break;
    case Expression::KindOfArrayElementExpression:
      name="ArrayElementExpression";
      break;
    case Expression::KindOfDynamicFunctionCall:
      name="DynamicFunctionCall";
      break;
    case Expression::KindOfObjectPropertyExpression:
      name="ObjectPropertyExpression";
      break;
    case Expression::KindOfObjectMethodExpression:
      name="ObjectMethodExpression";
      break;
    case Expression::KindOfListAssignment:
      name="ListAssignment";
      break;
    case Expression::KindOfNewObjectExpression:
      name="NewObjectExpression";
      break;
    case Expression::KindOfUnaryOpExpression:
      name="UnaryOpExpression";
      break;
    case Expression::KindOfIncludeExpression:
      name="IncludeExpression";
      break;
    case Expression::KindOfBinaryOpExpression:
      name="BinaryOpExpression";
      break;
    case Expression::KindOfQOpExpression:
      name="QOpExpression";
      break;
    case Expression::KindOfArrayPairExpression:
      name="ArrayPairExpression";
      break;
    case Expression::KindOfClassConstantExpression:
      name="ClassConstantExpression";
      break;
    case Expression::KindOfParameterExpression:
      name="ParameterExpression";
      break;
    case Expression::KindOfModifierExpression:
      name="ModifierExpression";
      break;
    case Expression::KindOfEncapsListExpression:
      name="EncapsListExpression";
      break;
    }

    int c = e->getContext();
    if ((c & Expression::Declaration) == Expression::Declaration) {
      scontext += "|Declaration";
    } else if (c & Expression::LValue) {
      scontext += "|LValue";
    }
    if (c & Expression::NoLValueWrapper) {
      scontext += "|NoLValueWrapper";
    }
    if (c & Expression::RefValue) {
      scontext += "|RefValue";
    }
    if (c & Expression::RefParameter) {
      scontext += "|RefParameter";
    }
    if (c & Expression::DeepReference) {
      scontext += "|DeepReference";
    }
    if (c & Expression::NoRefWrapper) {
      scontext += "|NoRefWrapper";
    }
    if (c & Expression::ObjectContext) {
      scontext += "|ObjectContext";
    }
    if (c & Expression::InParameterExpression) {
      scontext += "|InParameterExpression";
    }
    if (c & Expression::ExistContext) {
      scontext += "|ExistContext";
    }
    if (c & Expression::UnsetContext) {
      scontext += "|UnsetContext";
    }
    if (c & Expression::AssignmentLHS) {
      scontext += "|AssignmentLHS";
    }
    if (c & Expression::DeepAssignmentLHS) {
      scontext += "|DeepAssignmentLHS";
    }
    if (c & Expression::InvokeArgument) {
      scontext += "|InvokeArgument";
    }
    if (c & Expression::OprLValue) {
      scontext += "|OprLValue";
    }
    if (c & Expression::DeepOprLValue) {
      scontext += "|DeepOprLValue";
    }

    if (scontext != "") {
      scontext = " (" + scontext.substr(1) + ")";
    }
    type = (int)etype;

    if (e->getActualType()) {
      type_info = e->getActualType()->toString();
      if (e->getExpectedType()) {
        type_info += ":" + e->getExpectedType()->toString();
      }
      if (e->getImplementedType()) {
        type_info += ";" + e->getImplementedType()->toString();
      }
      type_info = "{" + type_info + "} ";
    }
  } else {
    ASSERT(FALSE);
  }

  int s = spc;
  while (s > 0) {
    int n = s > 10 ? 10 : s;
    std::cout << ("          "+10-n);
    s -= n;
  }

  std::cout << "-> 0x" << hex << setfill('0') << setw(10) << (int64)this << dec;

  std::cout << " " << name << "(" << type << ") ";
  if (id) {
    std::cout << "id=" << id << " ";
  }
  if (idPtr) {
    std::cout << "idp=0x" <<
      hex << setfill('0') << setw(10) << (int64)idPtr.get() << " ";
  }

  if (value != "") {
    std::cout << "[" << value << "] ";
  }
  std::cout << type_info << nkid << scontext;
  if (m_loc) {
    std::cout << " " << m_loc->file << ":" <<
      m_loc->line1 << "@" << m_loc->char1;
  }
  std::cout << "\n";

  for (int i = 0; i < nkid; i++) {
    ConstructPtr kid = getNthKid(i);
    if (kid) {
      kid->dump(spc+2, ar);
    } else {
      int s = spc+2;
      while (s > 0) {
        int n = s > 10 ? 10 : s;
        std::cout << ("          "+10-n);
        s -= n;
      }
      std::cout << "-> (nokid)\n";
    }
  }
}
