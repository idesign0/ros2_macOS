#include "grounder/grounder.h"
#include "parser/parsedTask.h"
#include "planner/plannerSetting.h"
#include "planner/printPlan.h"
#include "planner/z3Checker.h"
#include "preprocess/preprocess.h"
#include "sas/sasTranslator.h"
#include "utils/utils.h"
#include <Python.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

/*********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                     */
/* May 2023                                              */
/*********************************************************/
/* NextFLAP interface with the Unified Planning Platform */
/*********************************************************/

// Planning task to store the planning problem
ParsedTask *parsedTask = nullptr;

// Preprocesses the parsed task
PreprocessedTask *_preprocessStage(ParsedTask *parsedTask) {
  Preprocess preprocess;
  PreprocessedTask *prepTask = preprocess.preprocessTask(parsedTask);
  return prepTask;
}

// Grounder stage of the preprocessed task
GroundedTask *_groundingStage(PreprocessedTask *prepTask) {
  Grounder grounder;
  GroundedTask *gTask = grounder.groundTask(prepTask, false);
  if (gTask != nullptr && debugFile != nullptr)
    *debugFile << gTask->toString() << endl;
  return gTask;
}

// SAS translation stage
SASTask *_sasTranslationStage(GroundedTask *gTask) {
  SASTranslator translator;
  SASTask *sasTask = translator.translate(gTask, false, false, false);
  return sasTask;
}

// Planning process to search a solution plan
std::string _startPlanning(SASTask *sTask, bool durativePlan) {
  PlannerSetting planner(sTask);
  Plan *solution;
  float bestMakespan = FLOAT_INFINITY;
  int bestNumSteps = MAX_UINT16;
  do {
    solution = planner.plan(bestMakespan, parsedTask);
    if (solution != nullptr) {
      Z3Checker checker;
      TControVarValues cvarValues;
      float solutionMakespan;
      if (checker.checkPlan(solution, true, &cvarValues)) {
        solutionMakespan = PrintPlan::getMakespan(solution);
        if (solutionMakespan < bestMakespan ||
            (abs(solutionMakespan - bestMakespan) < EPSILON &&
             solution->g < bestNumSteps)) {
          bestMakespan = solutionMakespan;
          bestNumSteps = solution->g;
          return PrintPlan::print(solution, &cvarValues, durativePlan);
        }
      }
    }
  } while (solution != nullptr);
  return "No plan";
}

// Preprocesses and solves the planning task
std::string _solve(bool durativePlan) {
  parsedTask->startTime = clock();

  PreprocessedTask *prepTask = nullptr;
  GroundedTask *gTask = nullptr;
  SASTask *sTask = nullptr;
  std::string res = "";
  try {
    parsedTask->error = "";
    prepTask = _preprocessStage(parsedTask);
    if (prepTask != nullptr) {
      gTask = _groundingStage(prepTask);
      if (gTask != nullptr) {
        sTask = _sasTranslationStage(gTask);
        if (sTask != nullptr) {
          res = _startPlanning(sTask, durativePlan);
        }
      }
    }
  } catch (const PlannerException &e) {
    parsedTask->error = std::string(e.what());
    res = "Error: " + parsedTask->error;
  }
  try {
    if (sTask != nullptr)
      delete sTask;
    if (gTask != nullptr)
      delete gTask;
    if (prepTask != nullptr)
      delete prepTask;
  } catch (...) {
  }
  return res;
}

// Frees the memory, so another planning task can be defined
void end_task() {
  if (parsedTask != nullptr) {
    delete parsedTask;
  }
  parsedTask = nullptr;
}

// Creates a new planning task
void start_task(py::float_ timeout) {
  if (parsedTask != nullptr) {
    end_task();
  }
  parsedTask = new ParsedTask();
  parsedTask->timeout = timeout;
  parsedTask->setDomainName("UPF");
  // createDebugFile();
}

// Adds a new type to the planning task. Returns false if an error occurred
py::bool_ add_type(py::str typeName, py::list ancestors) {
  try {
    SyntaxAnalyzer syn;
    unsigned int index;
    std::vector<unsigned int> parentTypes;
    for (auto it : ancestors) {
      std::string parentTypeName = std::string(py::str(it));
      index = parsedTask->getTypeIndex(parentTypeName);
      if (index == MAX_UNSIGNED_INT) {
        index = parsedTask->getTypeIndex("#object");
        if (parentTypeName.compare("object") != 0) {
          std::vector<unsigned int> granParentTypes;
          granParentTypes.push_back(index);
          index = parsedTask->addType(parentTypeName, granParentTypes, &syn);
        }
      }
      parentTypes.push_back(index);
    }
    std::string name = typeName;
    if (parsedTask->addType(name, parentTypes, &syn) != MAX_UNSIGNED_INT)
      return true;
    parsedTask->error = "Type " + name + " redefined";
    return false;
  } catch (const std::exception &e) {
    parsedTask->error = e.what();
    return false;
  }
}

// Adds a new object to the planning task. Returns false if an error occurred
py::bool_ add_object(py::str objName, py::str typeName) {
  try {
    SyntaxAnalyzer syn;
    unsigned int typeIndex = parsedTask->getTypeIndex(typeName);
    if (typeIndex == MAX_UNSIGNED_INT)
      return false;
    std::vector<unsigned int> type(1, typeIndex);
    if (parsedTask->addObject(objName, type, &syn) != MAX_UNSIGNED_INT)
      return true;
    parsedTask->error = "Object " + std::string(objName) + " redefined";
    return false;
  } catch (const std::exception &e) {
    parsedTask->error = e.what();
    return false;
  }
}

// Adds a new fluent to the planning task. Returns false if an error occurred
py::bool_ add_fluent(py::str type, py::str name, py::list parameters) {
  try {
    SyntaxAnalyzer syn;
    Function f;
    f.name = name;
    for (auto it : parameters) {
      std::vector<unsigned int> paramTypes;
      std::string paramType = std::string(py::str(it));
      unsigned int typeIndex = parsedTask->getTypeIndex(paramType);
      if (typeIndex == MAX_UNSIGNED_INT) {
        parsedTask->error = "Type " + paramType + " undefined";
        return false;
      }
      paramTypes.push_back(typeIndex);
      f.parameters.emplace_back("", paramTypes);
    }
    std::string stype = type;
    unsigned int index = stype.compare("bool") == 0
                             ? parsedTask->addPredicate(f, &syn)
                             : parsedTask->addFunction(f, &syn);
    if (index != MAX_UNSIGNED_INT)
      return true;
    parsedTask->error = "Function/predicate " + f.name + " error";
    return false;
  } catch (const std::exception &e) {
    parsedTask->error = e.what();
    return false;
  }
}

// Checks if an action is already defined. Returns false if the action is not
// found
bool _find_action(std::string name) {
  for (DurativeAction &a : parsedTask->durativeActions)
    if (a.name.compare(name) == 0) {
      parsedTask->error = "Action " + name + " redefined";
      return true; // Action redefined
    }
  for (Action &a : parsedTask->actions)
    if (a.name.compare(name) == 0) {
      parsedTask->error = "Action " + name + " redefined";
      return true; // Action redefined
    }
  return false;
}

// Adds a new variable to the given list. Returns false if an error occurred
bool _add_variable(std::string name, std::string type,
                   std::vector<Variable> &list) {
  unsigned int typeIndex = parsedTask->getTypeIndex(type);
  if (typeIndex == MAX_UNSIGNED_INT) {
    parsedTask->error = "Type " + type + " undefined";
    return false;
  }
  std::vector<unsigned int> types(1, typeIndex);
  list.emplace_back(name, types);
  return true;
}

// Converts a term and stores it in the parameter "t". Returns false if an error
// occurred
bool _to_term(py::list term, Term &t,
              std::vector<std::vector<Variable> *> *variables) {
  std::string token = std::string(py::str(term[0]));
  if (token.compare("*param*") == 0) {
    t.type = TERM_PARAMETER;
    std::string token = std::string(py::str(term[1]));
    for (unsigned int i = 0; i < variables->at(0)->size(); i++) {
      if (variables->at(0)->at(i).name.compare(token) == 0) {
        t.index = i;
        return true;
      }
    }
    parsedTask->error = "Parameter " + token + " not defined";
    return false;
  }
  if (token.compare("*obj*") == 0) {
    t.type = TERM_CONSTANT;
    std::string token = std::string(py::str(term[1]));
    t.index = parsedTask->getObjectIndex(token);
    if (t.index != MAX_UNSIGNED_INT)
      return true;
    parsedTask->error = "Object " + token + " undefined";
    return false;
  }
  if (token.compare("*var*") == 0) {
    std::string token = std::string(py::str(term[1]));
    t.type = TERM_PARAMETER;
    t.index = (unsigned int)variables->at(0)->size();
    for (unsigned int i = 1; i < variables->size(); i++) {
      for (unsigned int j = 0; j < variables->at(i)->size(); j++) {
        if (variables->at(i)->at(j).name.compare(token) == 0) {
          return true;
        }
        t.index++;
      }
    }
    parsedTask->error = "Variable " + token + " undefined";
    return false;
  }
  return false;
}

// Converts a literal and stores it in the parameter "l". Returns false if an
// error occurred
bool _to_literal(py::list exp, Literal &l,
                 std::vector<std::vector<Variable> *> *variables) {
  std::string token = std::string(py::str(exp[1]));
  l.fncIndex = parsedTask->getFunctionIndex(token);
  if (l.fncIndex == MAX_UNSIGNED_INT) {
    parsedTask->error = "Function " + token + " undefined";
    return false;
  }
  for (int i = 2; i < exp.size(); i++) { // Parameters
    py::list param = py::cast<py::list>(exp[i]);
    Term t;
    if (!_to_term(param, t, variables)) {
      return false;
    }
    l.params.push_back(t);
  }
  return true;
}

// Converts a numeric expression and stores it in the parameter "nexp". Returns
// false if an error occurred
bool _to_numeric_expression(py::list exp, NumericExpression &nexp,
                            std::vector<std::vector<Variable> *> *variables,
                            std::vector<Variable> *controlVars) {
  std::string token = std::string(py::str(exp[0]));
  if (token.compare("*int*") == 0 ||
      token.compare("*real*") == 0) { // Integer or real number
    token = std::string(py::str(exp[1]));
    nexp.type = NET_NUMBER;
    nexp.value = std::stof(token);
    return true;
  }
  if (token.compare("*+*") == 0 || token.compare("*-*") == 0 ||
      token.compare("***") == 0 || token.compare("*/*") == 0) {
    char op = token.at(1);
    switch (op) {
    case '+':
      nexp.type = NET_SUM;
      break;
    case '-':
      nexp.type = NET_SUB;
      break;
    case '*':
      nexp.type = NET_MUL;
      break;
    case '/':
      nexp.type = NET_DIV;
      break;
    default:
      return false;
    }
    for (int i = 1; i < exp.size(); i++) {
      NumericExpression operand;
      if (!_to_numeric_expression(py::cast<py::list>(exp[i]), operand,
                                  variables, controlVars))
        return false;
      nexp.operands.push_back(operand);
    }
    if (nexp.type == NET_SUB && nexp.operands.size() == 1)
      nexp.type = NET_NEGATION;
    return true;
  }
  if (token.compare("*fluent*") == 0) {
    nexp.type = NET_FUNCTION;
    if (_to_literal(exp, nexp.function, variables))
      return true;
  }
  if (token.compare("*param*") == 0 && controlVars != nullptr) {
    std::string varName = std::string(py::str(exp[1]));
    nexp.type = NET_TERM;
    unsigned int paramIndex = MAX_UNSIGNED_INT;
    for (unsigned int i = 0; i < controlVars->size(); i++)
      if (varName.compare(controlVars->at(i).name) == 0) {
        paramIndex = i;
        break;
      }
    if (paramIndex == MAX_UNSIGNED_INT) {
      parsedTask->error = "Numeric variable " + varName + " not defined";
      return false;
    }
    nexp.term.type = TERM_CONTROL_VAR;
    nexp.term.index = paramIndex;
    return true;
  }
  parsedTask->error = token + " not implemented";
  return false;
}

// Adds the duration to a durative action. Returns false if an error occurred
py::bool_ _add_duration(py::list duration, DurativeAction &a) {
  std::vector<std::vector<Variable> *> variables;
  variables.push_back(&a.parameters);
  if (duration.size() == 1) {
    py::list exp = py::cast<py::list>(duration[0]);
    NumericExpression nexp;
    if (!_to_numeric_expression(exp, nexp, &variables, &a.controlVars))
      return false;
    a.duration.emplace_back(Symbol::EQUAL, nexp);
    return true;
  } else { // Duration inequalities
    py::list lower = py::cast<py::list>(duration[0]),
             upper = py::cast<py::list>(duration[1]);
    py::bool_ open = py::cast<py::bool_>(lower[0]);
    NumericExpression lower_exp;
    if (!_to_numeric_expression(py::cast<py::list>(lower[1]), lower_exp,
                                &variables, &a.controlVars))
      return false;
    a.duration.emplace_back(open ? Symbol::GREATER : Symbol::GREATER_EQ,
                            lower_exp);
    open = py::cast<py::bool_>(upper[0]);
    NumericExpression upper_exp;
    if (!_to_numeric_expression(py::cast<py::list>(upper[1]), upper_exp,
                                &variables, &a.controlVars))
      return false;
    a.duration.emplace_back(open ? Symbol::LESS : Symbol::LESS_EQ, upper_exp);
    return true;
  }
  return false;
}

// Converts a goal description and stores it in the parameter "goal". Returns
// false if an error occurred
bool _to_goal_description(py::list cond, GoalDescription &goal,
                          std::vector<std::vector<Variable> *> *variables,
                          TimeSpecifier time,
                          std::vector<Variable> *controlVars) {
  goal.time = time;
  std::string token = std::string(py::str(cond[0]));
  if (token.compare("*and*") == 0 || token.compare("*not*") == 0 ||
      token.compare("*imply*") == 0 || token.compare("*exists*") == 0 ||
      token.compare("*forall*") == 0) {
    char t = token.at(1);
    switch (t) {
    case 'a':
      goal.type = GD_AND;
      break;
    case 'n':
      goal.type = GD_NOT;
      break;
    case 'i':
      goal.type = GD_IMPLY;
      break;
    case 'e':
      goal.type = GD_EXISTS;
      break;
    case 'f':
      goal.type = GD_FORALL;
      break;
    default:
      return false;
    }
    int start = 1;
    if (goal.type == GD_EXISTS || goal.type == GD_FORALL) {
      start++;
      py::list vars = py::cast<py::list>(cond[1]);
      for (int i = 0; i < vars.size(); i++) {
        py::list var = py::cast<py::list>(vars[i]);
        if (!_add_variable(std::string(py::str(var[0])),
                           std::string(py::str(var[1])), goal.parameters))
          return false;
      }
    }
    if (goal.parameters.size() > 0)
      variables->push_back(&goal.parameters);
    for (int i = start; i < cond.size(); i++) {
      GoalDescription term;
      if (!_to_goal_description(py::cast<py::list>(cond[i]), term, variables,
                                NONE, controlVars))
        return false;
      goal.terms.push_back(term);
    }
    if (goal.parameters.size() > 0)
      variables->pop_back();
    return true;
  }
  if (token.compare("*fluent*") == 0) {
    goal.type = GD_LITERAL;
    return _to_literal(cond, goal.literal, variables);
  }
  if (token.compare("*<*") == 0 || token.compare("*<=*") == 0 ||
      token.compare("*>=*") == 0 || token.compare("*>*") == 0 ||
      token.compare("*=*") == 0) {
    goal.type = GD_F_CMP;
    char c1 = token.at(1), c2 = token.at(2);
    switch (c1) {
    case '<':
      goal.comparator = c2 == '=' ? CMP_LESS_EQ : CMP_LESS;
      break;
    case '>':
      goal.comparator = c2 == '=' ? CMP_GREATER_EQ : CMP_GREATER;
      break;
    case '=':
      goal.comparator = CMP_EQ;
      break;
    default:
      return false;
    }
    if (goal.comparator == CMP_EQ) { // Equality?
      for (int i = 1; i < cond.size(); i++) {
        Term term;
        if (_to_term(py::cast<py::list>(cond[i]), term, variables)) {
          goal.type = GD_EQUALITY;
          goal.eqTerms.push_back(term);
        } else {
          if (goal.type == GD_EQUALITY)
            return false;
          break;
        }
      }
    }
    if (goal.type != GD_EQUALITY) {
      for (int i = 1; i < cond.size(); i++) {
        NumericExpression nexp;
        if (!_to_numeric_expression(py::cast<py::list>(cond[i]), nexp,
                                    variables, controlVars))
          return false;
        goal.exp.push_back(nexp);
      }
    }
    return true;
  }
  parsedTask->error = token + " not implemented";
  return false;
}

// Converts a durative condition and stores it in the parameter "c". Returns
// false if an error occurred
bool _to_durative_condition(py::list cond, DurativeCondition &c,
                            DurativeAction *a, TimeSpecifier time) {
  c.type = CT_GOAL;
  std::vector<std::vector<Variable> *> variables;
  variables.push_back(&a->parameters);
  return _to_goal_description(cond, c.goal, &variables, time, &a->controlVars);
}

// Converts an effect expression and stores it in the parameter "e". Returns
// false if an error occurred
bool to_effect_expression(py::list exp, EffectExpression &e,
                          std::vector<std::vector<Variable> *> *variables,
                          std::vector<Variable> *controlVars) {
  std::string token = std::string(py::str(exp[0]));
  if (token.compare("*int*") == 0 || token.compare("*real*") == 0) {
    e.type = EE_NUMBER;
    std::string value = std::string(py::str(exp[1]));
    e.value = std::stof(value);
    return true;
  }
  if (token.compare("*fluent*") == 0) {
    e.type = EE_FLUENT;
    return _to_literal(exp, e.fluent, variables);
  }
  if (token.compare("*+*") == 0 || token.compare("*-*") == 0 ||
      token.compare("***") == 0 || token.compare("*/*") == 0) {
    e.type = EE_OPERATION;
    char op = token.at(1);
    switch (op) {
    case '+':
      e.operation = OT_SUM;
      break;
    case '-':
      e.operation = OT_SUB;
      break;
    case '*':
      e.operation = OT_MUL;
      break;
    case '/':
      e.operation = OT_DIV;
      break;
    default:
      return false;
    }
    for (int i = 1; i < exp.size(); i++) {
      EffectExpression operand;
      if (!to_effect_expression(py::cast<py::list>(exp[i]), operand, variables,
                                controlVars))
        return false;
      e.operands.push_back(operand);
    }
    return true;
  }
  if (token.compare("*duration*") == 0) {
    e.type = EE_DURATION;
    return true;
  }
  if (token.compare("*param*") == 0 && controlVars != nullptr) {
    std::string varName = std::string(py::str(exp[1]));
    e.type = EE_TERM;
    unsigned int paramIndex = MAX_UNSIGNED_INT;
    for (unsigned int i = 0; i < controlVars->size(); i++)
      if (varName.compare(controlVars->at(i).name) == 0) {
        paramIndex = i;
        break;
      }
    if (paramIndex == MAX_UNSIGNED_INT) {
      parsedTask->error = "Numeric variable " + varName + " not defined";
      return false;
    }
    e.term.type = TERM_CONTROL_VAR;
    e.term.index = paramIndex;
    return true;
  }
  parsedTask->error = token + " effect not implemented";
  return false;
}

// Converts an effect and stores it in the parameter "e". Returns false if an
// error occurred
bool _to_effect_single(py::list eff, Effect &e,
                       std::vector<std::vector<Variable> *> *variables,
                       std::vector<Variable> *controlVars) {
  std::string token = std::string(py::str(eff[0]));
  if (token.compare("*=*") == 0 || token.compare("*+=*") == 0 ||
      token.compare("*-=*") == 0 || token.compare("**=*") == 0 ||
      token.compare("*/=*") == 0) {
    char op = token.at(1);
    if (op == '=') { // Fluent or numeric assign?
      py::list lvalue = py::cast<py::list>(eff[2]);
      std::string token = std::string(py::str(lvalue[0]));
      if (token.compare("*true*") == 0 || token.compare("*false*") == 0) {
        if (token.compare("*true*") == 0) {
          e.type = ET_LITERAL;
          return _to_literal(py::cast<py::list>(eff[1]), e.literal, variables);
        } else {
          e.type = ET_NOT;
          Effect term;
          term.type = ET_LITERAL;
          if (!_to_literal(py::cast<py::list>(eff[1]), term.literal, variables))
            return false;
          e.terms.push_back(term);
          return true;
        }
      }
    }
    e.type = ET_ASSIGNMENT;
    switch (op) {
    case '=':
      e.assignment.type = AS_ASSIGN;
      break;
    case '+':
      e.assignment.type = AS_INCREASE;
      break;
    case '-':
      e.assignment.type = AS_DECREASE;
      break;
    case '*':
      e.assignment.type = AS_SCALE_UP;
      break;
    case '/':
      e.assignment.type = AS_SCALE_DOWN;
      break;
    default:
      return false;
    }
    if (!_to_literal(py::cast<py::list>(eff[1]), e.assignment.fluent,
                     variables))
      return false;
    return to_effect_expression(py::cast<py::list>(eff[2]), e.assignment.exp,
                                variables, controlVars);
  }
  if (token.compare("*when*") == 0) {
    e.type = ET_WHEN;
    if (!_to_goal_description(py::cast<py::list>(eff[1]), e.goal, variables,
                              NONE, controlVars))
      return false;
    for (int i = 2; i < eff.size(); i++) {
      Effect term;
      if (!_to_effect_single(py::cast<py::list>(eff[i]), term, variables,
                             controlVars))
        return false;
      e.terms.push_back(term);
    }
    return true;
  }
  if (token.compare("*forall*") == 0) {
    py::list vars = py::cast<py::list>(eff[1]);
    for (int i = 0; i < vars.size(); i++) {
      py::list var = py::cast<py::list>(vars[i]);
      if (!_add_variable(std::string(py::str(var[0])),
                         std::string(py::str(var[1])), e.parameters))
        return false;
    }
    if (e.parameters.size() > 0)
      variables->push_back(&e.parameters);
    for (int i = 2; i < eff.size(); i++) {
      Effect term;
      if (!_to_effect_single(py::cast<py::list>(eff[i]), term, variables,
                             controlVars))
        return false;
      e.terms.push_back(term);
    }
    if (e.parameters.size() > 0)
      variables->pop_back();
    return true;
  }
  parsedTask->error = token + " effect not implemented";
  return false;
}

// Converts an effect and stores it in the parameter "e". Returns false if an
// error occurred
bool _to_effect(py::list eff, Effect &e,
                std::vector<std::vector<Variable> *> *variables,
                std::vector<Variable> *controlVars) {
  if (eff.size() == 0)
    return true;
  if (eff.size() == 1) {
    return _to_effect_single(py::cast<py::list>(eff[0]), e, variables,
                             controlVars);
  }
  e.type = ET_AND;
  for (int i = 0; i < eff.size(); i++) {
    Effect term;
    if (!_to_effect_single(py::cast<py::list>(eff[i]), term, variables,
                           controlVars))
      return false;
    e.terms.push_back(term);
  }
  return true;
}

// Converts a precondition and stores it in the parameter "prec". Returns false
// if an error occurred
bool _to_precondition(py::list cond, Precondition &prec,
                      std::vector<std::vector<Variable> *> *variables,
                      std::vector<Variable> *controlVars) {
  if (cond.size() == 0)
    return true;
  if (cond.size() == 1) {
    return _to_precondition(py::cast<py::list>(cond[0]), prec, variables,
                            controlVars);
  }
  std::string token = std::string(py::str(cond[0]));
  if (token.compare("*or*") == 0 || token.compare("*and*") == 0 ||
      token.compare("*not*") == 0 || token.compare("*imply*") == 0 ||
      token.compare("*exists*") == 0 || token.compare("*forall*") == 0) {
    char t = token.at(1);
    switch (t) {
    case 'o':
      prec.type = PT_OR;
      break;
    case 'a':
      prec.type = PT_AND;
      break;
    case 'n':
      prec.type = PT_NOT;
      break;
    case 'i':
      prec.type = PT_IMPLY;
      break;
    case 'e':
      prec.type = PT_EXISTS;
      break;
    case 'f':
      prec.type = PT_FORALL;
      break;
    default:
      return false;
    }
    int start = 1;
    if (prec.type == PT_EXISTS || prec.type == PT_FORALL) {
      start++;
      py::list vars = py::cast<py::list>(cond[1]);
      for (int i = 0; i < vars.size(); i++) {
        py::list var = py::cast<py::list>(vars[i]);
        if (!_add_variable(std::string(py::str(var[0])),
                           std::string(py::str(var[1])), prec.parameters))
          return false;
      }
    }
    if (prec.parameters.size() > 0)
      variables->push_back(&prec.parameters);
    for (int i = start; i < cond.size(); i++) {
      Precondition term;
      if (!_to_precondition(py::cast<py::list>(cond[i]), term, variables,
                            controlVars))
        return false;
      prec.terms.push_back(term);
    }
    if (prec.parameters.size() > 0)
      variables->pop_back();
    return true;
  }
  if (token.compare("*fluent*") == 0) {
    prec.type = PT_LITERAL;
    return _to_literal(cond, prec.literal, variables);
  }
  if (token.compare("*<*") == 0 || token.compare("*<=*") == 0 ||
      token.compare("*>=*") == 0 || token.compare("*>*") == 0 ||
      token.compare("*=*") == 0) {
    prec.type = PT_F_CMP;
    if (token.compare("*=*") == 0) {
      Term term;
      if (_to_term(py::cast<py::list>(cond[1]), term, variables)) {
        prec.type = PT_EQUALITY;
      }
    }
    return _to_goal_description(cond, prec.goal, variables, NONE, controlVars);
  }
  parsedTask->error = token + " not implemented";
  return false;
}

// Converts a timed effect and stores it in the parameter "e". Returns false if
// an error occurred
bool _to_timed_effect(py::list eff, TimedEffect &e,
                      std::vector<std::vector<Variable> *> *variables,
                      TimeSpecifier time, std::vector<Variable> *controlVars) {
  std::string token = std::string(py::str(eff[0]));
  e.time = time;
  if (token.compare("*and*") == 0 || token.compare("*not*") == 0 ||
      token.compare("*or*") == 0) {
    char op = token.at(1);
    switch (op) {
    case 'a':
      e.type = TE_AND;
      break;
    case 'n':
      e.type = TE_NOT;
      break;
    case 'o':
      e.type = TE_OR;
      break;
    default:
      return false;
    }
    for (int i = 1; i < eff.size(); i++) {
      TimedEffect term;
      if (!_to_timed_effect(py::cast<py::list>(eff[i]), term, variables, time,
                            controlVars))
        return false;
      e.terms.push_back(term);
    }
    return true;
  }
  if (token.compare("*=*") == 0 || token.compare("*+=*") == 0 ||
      token.compare("*-=*") == 0 || token.compare("**=*") == 0 ||
      token.compare("*/=*") == 0) {
    char op = token.at(1);
    if (op == '=') { // Fluent or numeric assign?
      py::list lvalue = py::cast<py::list>(eff[2]);
      std::string token = std::string(py::str(lvalue[0]));
      if (token.compare("*true*") == 0 || token.compare("*false*") == 0) {
        if (token.compare("*true*") == 0) {
          e.type = TE_LITERAL;
          return _to_literal(py::cast<py::list>(eff[1]), e.literal, variables);
        } else {
          e.type = TE_NOT;
          TimedEffect term;
          term.time = time;
          term.type = TE_LITERAL;
          if (!_to_literal(py::cast<py::list>(eff[1]), term.literal, variables))
            return false;
          e.terms.push_back(term);
          return true;
        }
      }
    }
    e.type = TE_ASSIGNMENT;
    switch (op) {
    case '=':
      e.assignment.type = AS_ASSIGN;
      break;
    case '+':
      e.assignment.type = AS_INCREASE;
      break;
    case '-':
      e.assignment.type = AS_DECREASE;
      break;
    case '*':
      e.assignment.type = AS_SCALE_UP;
      break;
    case '/':
      e.assignment.type = AS_SCALE_DOWN;
      break;
    default:
      return false;
    }
    if (!_to_literal(py::cast<py::list>(eff[1]), e.assignment.fluent,
                     variables))
      return false;
    return to_effect_expression(py::cast<py::list>(eff[2]), e.assignment.exp,
                                variables, controlVars);
  }
  parsedTask->error = token + " effect not implemented";
  return false;
}

// Converts a single durative effect and stores it in the parameter "e". Returns
// false if an error occurred
bool _to_durative_effect_single(py::list eff, DurativeEffect &e,
                                std::vector<std::vector<Variable> *> *variables,
                                TimeSpecifier time,
                                std::vector<Variable> *controlVars) {
  std::string token = std::string(py::str(eff[0]));
  if (token.compare("*and*") == 0 || token.compare("*forall*") == 0) {
    char t = token.at(1);
    switch (t) {
    case 'a':
      e.type = DET_AND;
      break;
    case 'f':
      e.type = DET_FORALL;
      break;
    default:
      return false;
    }
    int start = 1;
    if (e.type == DET_FORALL) {
      start++;
      py::list vars = py::cast<py::list>(eff[1]);
      for (int i = 0; i < vars.size(); i++) {
        py::list var = py::cast<py::list>(vars[i]);
        if (!_add_variable(std::string(py::str(var[0])),
                           std::string(py::str(var[1])), e.parameters))
          return false;
      }
    }
    if (e.parameters.size() > 0)
      variables->push_back(&e.parameters);
    for (int i = start; i < eff.size(); i++) {
      DurativeEffect term;
      if (!_to_durative_effect_single(py::cast<py::list>(eff[i]), term,
                                      variables, time, controlVars))
        return false;
      e.terms.push_back(term);
    }
    if (e.parameters.size() > 0)
      variables->pop_back();
    return true;
  }
  if (token.compare("*when*") == 0) {
    e.type = DET_WHEN;
    e.condition.type = CT_GOAL;
    if (!_to_goal_description(py::cast<py::list>(eff[1]), e.condition.goal,
                              variables, time, controlVars))
      return false;
    return _to_timed_effect(py::cast<py::list>(eff[2]), e.timedEffect,
                            variables, time, controlVars);
  }
  e.type = DET_TIMED_EFFECT;
  return _to_timed_effect(eff, e.timedEffect, variables, time, controlVars);
}

// Converts a durative effect and stores it in the parameter "e". Returns false
// if an error occurred
bool _to_durative_effect(py::list eff, DurativeEffect &e, DurativeAction *a,
                         TimeSpecifier time) {
  std::vector<std::vector<Variable> *> variables;
  variables.push_back(&a->parameters);
  return _to_durative_effect_single(eff, e, &variables, time, &a->controlVars);
}

// Adds a control parameter
bool _add_control_parameter(std::string param, std::string typeName,
                            std::vector<Variable> &controlVars) {
  std::vector<unsigned int> types;
  if (typeName.compare("#int") == 0)
    types.push_back(parsedTask->INTEGER_TYPE);
  else
    types.push_back(parsedTask->NUMBER_TYPE);
  controlVars.emplace_back(param, types);
  return true;
}

// Adds a durative action to the planning task. Returns false if an error
// occurred
bool _add_durative_action(py::str name, py::list parameters, py::list duration,
                          py::list startCond, py::list overAllCond,
                          py::list endCond, py::list startEff,
                          py::list endEff) {
  if (_find_action(name))
    return false;
  DurativeAction a;
  a.index = (int)parsedTask->durativeActions.size();
  a.name = name;
  bool hasControlParameters = false;
  for (py::handle it : parameters) {
    py::list param = py::cast<py::list>(it);
    std::string typeName = std::string(py::str(param[1]));
    if (typeName.compare("#real") == 0 || typeName.compare("#int") == 0) {
      hasControlParameters = true;
      if (!_add_control_parameter(std::string(py::str(param[0])), typeName,
                                  a.controlVars))
        return false;
    } else {
      if (hasControlParameters) {
        parsedTask->error = "Numeric parameters must be defined at the end of "
                            "the parameter list";
        return false;
      }
      if (!_add_variable(std::string(py::str(param[0])), typeName,
                         a.parameters))
        return false;
    }
  }
  parsedTask->error = "Error in duration";
  if (!_add_duration(duration, a))
    return false;

  parsedTask->error = "Error in cond";
  a.condition.type = CT_AND;
  for (py::handle it : startCond) {
    DurativeCondition c;
    if (!_to_durative_condition(py::cast<py::list>(it), c, &a, AT_START))
      return false;
    a.condition.conditions.push_back(c);
  }
  for (py::handle it : overAllCond) {
    DurativeCondition c;
    if (!_to_durative_condition(py::cast<py::list>(it), c, &a, OVER_ALL))
      return false;
    a.condition.conditions.push_back(c);
  }
  for (py::handle it : endCond) {
    DurativeCondition c;
    if (!_to_durative_condition(py::cast<py::list>(it), c, &a, AT_END))
      return false;
    a.condition.conditions.push_back(c);
  }
  parsedTask->error = "Error in eff";
  a.effect.type = DET_AND;
  for (py::handle it : startEff) {
    DurativeEffect e;
    if (!_to_durative_effect(py::cast<py::list>(it), e, &a, AT_START))
      return false;
    a.effect.terms.push_back(e);
  }
  for (py::handle it : endEff) {
    DurativeEffect e;
    if (!_to_durative_effect(py::cast<py::list>(it), e, &a, AT_END))
      return false;
    a.effect.terms.push_back(e);
  }
  parsedTask->durativeActions.push_back(a);
  parsedTask->error =
      a.toString(parsedTask->functions, parsedTask->objects, parsedTask->types);
  return true;
}

// Adds an instantaneous action to the planning task. Returns false if an error
// occurred
bool _add_instantaneous_action(py::str name, py::list parameters, py::list cond,
                               py::list eff) {
  if (_find_action(name))
    return false;
  Action a;
  a.index = (int)parsedTask->actions.size();
  a.name = name;
  for (py::handle it : parameters) {
    py::list param = py::cast<py::list>(it);
    std::string typeName = std::string(py::str(param[1]));
    if (typeName.compare("#real") == 0 || typeName.compare("#int") == 0) {
      parsedTask->error =
          "Numeric parameters are only allowed in durative actions";
      return false;
    }
    if (!_add_variable(std::string(py::str(param[0])), typeName, a.parameters))
      return false;
  }
  std::vector<std::vector<Variable> *> variables;
  variables.push_back(&a.parameters);
  if (!_to_precondition(cond, a.precondition, &variables, nullptr))
    return false;
  if (!_to_effect(eff, a.effect, &variables, nullptr))
    return false;
  parsedTask->actions.push_back(a);
  parsedTask->error =
      a.toString(parsedTask->functions, parsedTask->objects, parsedTask->types);
  return true;
}

// Adds an action to the planning task. Returns false if an error occurred
py::bool_ add_action(py::str name, py::bool_ durative, py::list parameters,
                     py::list duration, py::list startCond,
                     py::list overAllCond, py::list endCond, py::list startEff,
                     py::list endEff) {
  try {
    bool ok =
        durative
            ? _add_durative_action(name, parameters, duration, startCond,
                                   overAllCond, endCond, startEff, endEff)
            : _add_instantaneous_action(name, parameters, startCond, startEff);
    return ok;
  } catch (const std::exception &e) {
    parsedTask->error = e.what();
    return false;
  }
}

// Returns the last error message occurred
py::str get_error() {
  return parsedTask != nullptr ? parsedTask->error : "Task not started";
}

// Converts a fact and stores it in the parameter "f". Returns false if an error
// occurred
bool _to_fact(py::list fluent, Fact &f, float time) {
  std::string function = std::string(py::str(fluent[1]));
  f.function = parsedTask->getFunctionIndex(function);
  if (f.function == MAX_UNSIGNED_INT) {
    parsedTask->error = "Function " + function + " undefined";
    return false;
  }
  f.valueIsNumeric = false;
  for (unsigned int type : parsedTask->functions[f.function].valueTypes) {
    if (type == parsedTask->NUMBER_TYPE || type == parsedTask->INTEGER_TYPE) {
      f.valueIsNumeric = true;
      break;
    }
  }
  for (int i = 2; i < fluent.size(); i++) {
    py::list param = py::cast<py::list>(fluent[i]);
    std::string obj = std::string(py::str(param[1]));
    unsigned int objIndex = parsedTask->getObjectIndex(obj);
    if (objIndex == MAX_UNSIGNED_INT) {
      parsedTask->error = "Object " + obj + " undefined";
      return false;
    }
    f.parameters.push_back(objIndex);
  }
  f.time = time;
  return true;
}

// Assigns a value to a fact. Returns false if an error occurred
bool _add_value(Fact &f, py::list value) {
  if (f.valueIsNumeric) {
    std::string v = std::string(py::str(value[1]));
    f.numericValue = std::stof(v);
    return true;
  } else {
    std::string v = std::string(py::str(value[0]));
    if (v.compare("*true*") == 0)
      f.value = parsedTask->CONSTANT_TRUE;
    else if (v.compare("*false*") == 0)
      f.value = parsedTask->CONSTANT_FALSE;
    else {
      parsedTask->error = v + " is not a boolean value";
      return false;
    }
  }
  return true;
}

// Adds an initial value to the planning task. Returns false if an error
// occurred
py::bool_ add_initial_value(py::list fluent, py::list value, py::float_ time) {
  try {
    Fact fact;
    if (!_to_fact(fluent, fact, time))
      return false;
    if (!_add_value(fact, value))
      return false;
    parsedTask->init.push_back(fact);
    return true;
  } catch (const std::exception &e) {
    parsedTask->error = e.what();
    return false;
  }
}

// Adds a goal to the planning task. Returns false if an error occurred
py::bool_ add_goal(py::list cond) {
  std::vector<std::vector<Variable> *> variables;
  return _to_precondition(cond, parsedTask->goal, &variables, nullptr);
}

// Solves the planning task and returns the plan as a string
py::str solve(py::bool_ durativePlan) { return py::str(_solve(durativePlan)); }

PYBIND11_MODULE(nextflap, m) {
  m.doc() = "pybind11 nextflap plugin"; // optional module docstring

  m.def("start_task", &start_task, "A function that creates the PDDL task");
  m.def("end_task", &end_task, "A function that finishes the PDDL task");
  m.def("get_error", &get_error,
        "A function that gets information about the last error");
  m.def("add_type", &add_type, "A function that adds a PDDL type to the task");
  m.def("add_object", &add_object,
        "A function that adds a PDDL object to the task");
  m.def("add_fluent", &add_fluent,
        "A function that adds a PDDL fluent to the task");
  m.def("add_action", &add_action,
        "A function that adds a PDDL action to the task");
  m.def("add_initial_value", &add_initial_value,
        "A function that adds the initial value of a fluent to the task");
  m.def("add_goal", &add_goal, "A function that adds the goal to the task");
  m.def("solve", &solve, "Solve the planning task");
}
