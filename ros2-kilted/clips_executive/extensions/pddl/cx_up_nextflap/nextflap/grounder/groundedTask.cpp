/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* April 2022                                           */
/********************************************************/
/* Grounded task obtained after the grounding process.  */
/********************************************************/

#include "groundedTask.h"
#include "../utils/utils.h"
using namespace std;

/********************************************************/
/* CLASS: GroundedValue                                 */
/********************************************************/

string GroundedValue::toString(ParsedTask *task, bool isNumeric) {
  string s = "(at " + to_string(time) + " (";
  if (isNumeric)
    s += to_string(numericValue);
  else
    s += task->objects[value].name;
  return s + ")";
}

/********************************************************/
/* CLASS: GroundedVar                                   */
/********************************************************/

string GroundedVar::toString(ParsedTask *task) { return toString(task, false); }

string GroundedVar::toString(ParsedTask *task, bool isGoal) {
  string s = "(" + task->functions[fncIndex].name;
  for (unsigned int i = 0; i < params.size(); i++) {
    s += " ";
#ifdef PDDL4J_FORMAT
    if (!task->objects[params[i]].isConstant && !isGoal)
      s += "?";
#endif
    s += task->objects[params[i]].name;
  }
  return s + ")"; // + std::to_string(index);
}

/********************************************************/
/* CLASS: GroundedCondition                             */
/********************************************************/

GroundedCondition::GroundedCondition(unsigned int variable,
                                     unsigned int value) {
  varIndex = variable;
  valueIndex = value;
}

string GroundedCondition::toString(ParsedTask *task,
                                   vector<GroundedVar> &variables) {
  return "(= " + std::to_string(varIndex) + " -> " +
         variables[varIndex].toString(task) + " " +
         task->objects[valueIndex].name + ")";
}

void GroundedCondition::writePDDLCondition(ofstream &f, ParsedTask *task,
                                           vector<GroundedVar> &variables,
                                           bool isGoal) {
  if (valueIndex == task->CONSTANT_TRUE)
    f << variables[varIndex].toString(task, isGoal);
  else if (valueIndex == task->CONSTANT_FALSE)
    f << "(not " << variables[varIndex].toString(task, isGoal) << ")";
  else
    f << "(= " + variables[varIndex].toString(task, isGoal) + " " +
             task->objects[valueIndex].name + ")";
}

/********************************************************/
/* CLASS: GroundedNumericExpression                     */
/********************************************************/

string
GroundedNumericExpression::toString(ParsedTask *task,
                                    vector<GroundedVar> &variables,
                                    vector<GroundedControlVar> &controlVars) {
  string s;
  switch (type) {
  case GE_NUMBER:
    s = to_string(value);
    break;
  case GE_VAR:
    s = variables[index].toString(task);
    break;
  case GE_SUM:
  case GE_SUB:
  case GE_DIV:
  case GE_MUL:
    if (type == GE_SUM)
      s = "*";
    else if (type == GE_SUB)
      s = "-";
    else if (type == GE_MUL)
      s = "*";
    else
      s = "/";
    for (unsigned int i = 0; i < terms.size(); i++)
      s += " (" + terms[i].toString(task, variables, controlVars) + ")";
    break;
  case GE_OBJECT:
    s = task->objects[index].name;
    break;
  case GE_DURATION:
    s = "duration";
    break;
  case GE_SHARP_T:
    s = "#t";
    break;
  case GE_CONTROL_VAR:
    s = controlVars[index].name;
    break;
  default:
    s = "?";
  }
  return s;
}

void GroundedNumericExpression::writePDDLNumericExpression(
    ofstream &f, ParsedTask *task, vector<GroundedVar> &variables,
    bool isGoal) {
  switch (type) {
  case GE_NUMBER:
    f << value;
    break;
  case GE_VAR:
    f << variables[index].toString(task, isGoal);
    break;
  case GE_SUM:
  case GE_SUB:
  case GE_DIV:
  case GE_MUL:
    f << "(";
    if (type == GE_SUM)
      f << "*";
    else if (type == GE_SUB)
      f << "-";
    else if (type == GE_MUL)
      f << "*";
    else
      f << "/";
    for (unsigned int i = 0; i < terms.size(); i++) {
      f << " ";
      terms[i].writePDDLNumericExpression(f, task, variables, isGoal);
    }
    f << ")";
    break;
  case GE_OBJECT:
    f << task->objects[index].name;
    break;
  case GE_DURATION:
    f << "duration";
    break;
  case GE_SHARP_T:
    f << "#t";
    break;
  default:;
  }
}

bool GroundedNumericExpression::requiresNumericVariable(TVariable v) {
  if (type == GE_VAR)
    return index == v;
  for (GroundedNumericExpression &t : terms)
    if (t.requiresNumericVariable(v))
      return true;
  return false;
}

/********************************************************/
/* CLASS: GroundedNumericCondition                     */
/********************************************************/

string
GroundedNumericCondition::toString(ParsedTask *task,
                                   vector<GroundedVar> &variables,
                                   vector<GroundedControlVar> &controlVars) {
  string s = "(" + ParsedTask::comparatorToString(comparator);
  for (unsigned int i = 0; i < terms.size(); i++)
    s += " " + terms[i].toString(task, variables, controlVars);
  return s + ")";
}

void GroundedNumericCondition::writePDDLCondition(
    ofstream &f, ParsedTask *task, vector<GroundedVar> &variables,
    bool isGoal) {
  f << "(" << ParsedTask::comparatorToString(comparator);
  for (unsigned int i = 0; i < terms.size(); i++) {
    f << " ";
    terms[i].writePDDLNumericExpression(f, task, variables, isGoal);
  }
  f << ")";
}

bool GroundedNumericCondition::requiresNumericVariable(TVariable v) {
  for (GroundedNumericExpression &t : terms)
    if (t.requiresNumericVariable(v))
      return true;
  return false;
}

/********************************************************/
/* CLASS: GroundedNumericEffect                         */
/********************************************************/

string
GroundedNumericEffect::toString(ParsedTask *task,
                                vector<GroundedVar> &variables,
                                vector<GroundedControlVar> &controlVars) {
  return "(" + ParsedTask::assignmentToString(assignment) + " " +
         variables[varIndex].toString(task) + " " +
         exp.toString(task, variables, controlVars) + ")";
}

void GroundedNumericEffect::writePDDLEffect(
    std::ofstream &f, ParsedTask *task, std::vector<GroundedVar> &variables) {
  f << "(" << ParsedTask::assignmentToString(assignment) << " "
    << variables[varIndex].toString(task) << " ";
  exp.writePDDLNumericExpression(f, task, variables, false);
  f << ")";
}

/********************************************************/
/* CLASS: PartiallyGroundedNumericExpression            */
/********************************************************/

void PartiallyGroundedNumericExpression::addTerm(
    Term &term, vector<unsigned int> &parameters) {
  if (term.type != TERM_CONSTANT) {
    if (term.index < parameters.size()) {
      isParameter.push_back(false);
      paramIndex.push_back(parameters[term.index]);
    } else {
      isParameter.push_back(true);
      paramIndex.push_back(term.index - parameters.size());
    }
  } else {
    isParameter.push_back(false);
    paramIndex.push_back(term.index);
  }
}

string
PartiallyGroundedNumericExpression::toString(ParsedTask *task,
                                             vector<GroundedVar> &variables) {
  string s = "(";
  switch (type) {
  case PGE_NUMBER:
    s += to_string(value);
    break;
  case PGE_VAR:
    s += variables[index].toString(task);
    break;
  case PGE_UNGROUNDED_VAR:
    s += task->functions[index].name;
    for (unsigned int i = 0; i < paramIndex.size(); i++)
      if (isParameter[i])
        s += " ?" + to_string(paramIndex[i]);
      else
        s += " " + task->objects[paramIndex[i]].name;
    break;
  case PGE_SUM:
  case PGE_SUB:
  case PGE_DIV:
  case PGE_MUL:
  case PGE_NOT:
    if (type == PGE_SUM)
      s += "+";
    else if (type == PGE_SUB)
      s += "-";
    else if (type == PGE_DIV)
      s += "/";
    else if (type == PGE_MUL)
      s += "*";
    else
      s += "not";
    for (unsigned int i = 0; i < terms.size(); i++)
      s += " " + terms[i].toString(task, variables);
    break;
  case PGE_TERM:
    if (isParameter[0])
      s += " ?" + to_string(paramIndex[0]);
    else
      s += " " + task->objects[paramIndex[0]].name;
    break;
  }
  return s + ")";
}

void PartiallyGroundedNumericExpression::writePDDLNumericExpression(
    ofstream &f, ParsedTask *task, vector<GroundedVar> &variables) {
  switch (type) {
  case PGE_NUMBER:
    f << value;
    break;
  case PGE_VAR:
    f << variables[index].toString(task);
    break;
  case PGE_UNGROUNDED_VAR:
    f << "(" << task->functions[index].name;
    for (unsigned int i = 0; i < paramIndex.size(); i++) {
      f << " ";
      if (isParameter[i])
        f << "?" << (char)(paramIndex[i] + 'a');
      else
        f << task->objects[paramIndex[i]].name;
    }
    f << ")";
    break;
  case PGE_SUM:
  case PGE_SUB:
  case PGE_DIV:
  case PGE_MUL:
  case PGE_NOT:
    if (type == PGE_SUM)
      f << "(+";
    else if (type == PGE_SUB)
      f << "(-";
    else if (type == PGE_DIV)
      f << "(/";
    else if (type == PGE_MUL)
      f << "(*";
    else
      f << "(not";
    for (unsigned int i = 0; i < terms.size(); i++) {
      f << " ";
      terms[i].writePDDLNumericExpression(f, task, variables);
    }
    f << ")";
    break;
  case PGE_TERM:
    if (isParameter[0])
      f << "?" << (char)(paramIndex[0] + 'a');
    else
      f << task->objects[paramIndex[0]].name;
    break;
  }
}

/********************************************************/
/* CLASS: GroundedGoalDescription                       */
/********************************************************/

void GroundedGoalDescription::addTerm(Term &term,
                                      vector<unsigned int> &parameters) {
  if (term.type != TERM_CONSTANT) {
    if (term.index < parameters.size()) {
      isParameter.push_back(false);
      paramIndex.push_back(parameters[term.index]);
    } else {
      isParameter.push_back(true);
      paramIndex.push_back(term.index - parameters.size());
    }
  } else {
    isParameter.push_back(false);
    paramIndex.push_back(term.index);
  }
}

string GroundedGoalDescription::toString(ParsedTask *task,
                                         vector<GroundedVar> &variables,
                                         unsigned int paramNumber) {
  string s = ParsedTask::timeToString(time) + "(";
  switch (type) {
  case GG_FLUENT:
    s += equal ? "= (" : "!= (";
    s += variables[index].toString(task) + ") ";
    if (valueIsParam)
      s += "?" + to_string(value);
    else
      s += task->objects[value].name;
    break;
  case GG_UNGROUNDED_FLUENT:
    s += equal ? "= (" : "!= (";
    s += task->functions[index].name;
    for (unsigned int i = 0; i < paramIndex.size(); i++)
      if (isParameter[i])
        s += " ?" + to_string(paramIndex[i]);
      else
        s += " " + task->objects[paramIndex[i]].name;
    s += ") ";
    if (valueIsParam)
      s += "?" + to_string(value);
    else
      s += task->objects[value].name;
    break;
  case GG_AND:
  case GG_OR:
  case GG_NOT:
  case GG_IMPLY:
    if (type == GG_AND)
      s += "and";
    else if (type == GG_OR)
      s += "or";
    else if (type == GG_NOT)
      s += "not";
    else
      s += "imply";
    for (unsigned int i = 0; i < terms.size(); i++)
      s += " " + terms[i].toString(task, variables, paramNumber);
    break;
  case GG_EXISTS:
  case GG_FORALL:
    if (type == GG_EXISTS)
      s += "exists (";
    else
      s += "forall (";
    for (unsigned int i = 0; i < paramTypes.size(); i++) {
      s += "?" + to_string(paramNumber) + " -";
      for (unsigned int j = 0; j < paramTypes[i].size(); j++)
        s += " " + to_string(paramTypes[i][j]);
      paramNumber++;
      if (i != paramTypes.size() - 1)
        s += " ";
    }
    s += ")";
    for (unsigned int i = 0; i < terms.size(); i++)
      s += " " + terms[i].toString(task, variables, paramNumber);
    break;
  case GG_EQUALITY:
  case GG_INEQUALITY:
    s += type == GG_EQUALITY ? "=" : "!=";
    for (unsigned int i = 0; i < paramIndex.size(); i++)
      if (isParameter[i])
        s += " ?" + to_string(paramIndex[i]);
      else
        s += " " + task->objects[paramIndex[i]].name;
    break;
  case GG_COMP:
    s += ParsedTask::comparatorToString(comparator);
    for (unsigned int i = 0; i < exp.size(); i++)
      s += " " + exp[i].toString(task, variables);
    break;
  }
  return s + ")";
}

string GroundedGoalDescription::toString(ParsedTask *task,
                                         vector<GroundedVar> &variables) {
  return toString(task, variables, 0);
}

void GroundedGoalDescription::writePDDLGoal(ofstream &f, ParsedTask *task,
                                            vector<GroundedVar> &variables) {
  writePDDLGoal(f, task, variables, 'a');
}

void GroundedGoalDescription::writePDDLGoal(ofstream &f, ParsedTask *task,
                                            vector<GroundedVar> &variables,
                                            char paramName) {
  if (time != NONE)
    f << "(" << ParsedTask::timeToString(time) + " ";
  switch (type) {
  case GG_FLUENT:
    if (value == task->CONSTANT_TRUE) {
      if (equal)
        f << variables[index].toString(task, true);
      else
        f << "(not " << variables[index].toString(task, true) << ")";
    } else if (value == task->CONSTANT_FALSE) {
      if (!equal)
        f << variables[index].toString(task, true);
      else
        f << "(not " << variables[index].toString(task, true) << ")";
    } else {
      if (!equal)
        f << "(not ";
      f << "(= " << variables[index].toString(task, true) << " ";
      if (valueIsParam)
        f << "?" << (char)(value + 'a');
      else
        f << task->objects[value].name;
      f << ")";
      if (!equal)
        f << ")";
    }
    break;
  case GG_UNGROUNDED_FLUENT: {
    bool isLiteral =
        value == task->CONSTANT_TRUE || value == task->CONSTANT_FALSE;
    bool neg = (value == task->CONSTANT_TRUE && !equal) ||
               (value == task->CONSTANT_FALSE && equal) ||
               (!isLiteral && !equal);
    if (neg)
      f << "(not ";
    if (!isLiteral)
      f << "(= ";
    f << "(" << task->functions[index].name;
    for (unsigned int i = 0; i < paramIndex.size(); i++) {
      f << " ";
      if (isParameter[i])
        f << "?" << (char)(paramIndex[i] + 'a');
      else
        f << task->objects[paramIndex[i]].name;
    }
    if (isLiteral)
      f << ")";
    else {
      if (valueIsParam)
        f << "?" << (char)(value + 'a');
      else
        f << task->objects[value].name;
      f << ")";
    }
    if (neg)
      f << ")";
  } break;
  case GG_AND:
  case GG_OR:
  case GG_NOT:
  case GG_IMPLY:
    if (type == GG_AND)
      f << "(and";
    else if (type == GG_OR)
      f << "(or";
    else if (type == GG_NOT)
      f << "(not";
    else
      f << "(imply";
    for (unsigned int i = 0; i < terms.size(); i++) {
      f << " ";
      terms[i].writePDDLGoal(f, task, variables, paramName);
    }
    f << ")";
    break;
  case GG_EXISTS:
  case GG_FORALL:
    if (type == GG_EXISTS)
      f << "(exists (";
    else
      f << "(forall (";
    for (unsigned int i = 0; i < paramTypes.size(); i++) {
      if (i > 0)
        f << " ";
      f << "?" << paramName;
      GroundedTask::writePDDLTypes(f, paramTypes[i], task);
      paramName++;
    }
    f << ")";
    for (unsigned int i = 0; i < terms.size(); i++) {
      f << " ";
      terms[i].writePDDLGoal(f, task, variables, paramName);
    }
    f << ")";
    break;
  case GG_EQUALITY:
  case GG_INEQUALITY:
    if (type == GG_EQUALITY)
      f << "(=";
    else
      f << "(not (=";
    for (unsigned int i = 0; i < paramIndex.size(); i++) {
      f << " ";
      if (isParameter[i])
        f << "?" << (char)(paramIndex[i] + 'a');
      else
        f << task->objects[paramIndex[i]].name;
    }
    if (type == GG_INEQUALITY)
      f << ")";
    f << ")";
    break;
  case GG_COMP:
    if (comparator == CMP_NEQ)
      f << "(not (=";
    else
      f << "(" << ParsedTask::comparatorToString(comparator);
    for (unsigned int i = 0; i < exp.size(); i++) {
      f << " ";
      exp[i].writePDDLNumericExpression(f, task, variables);
    }
    if (comparator == CMP_NEQ)
      f << ")";
    f << ")";
    break;
  }
  if (time != NONE)
    f << ")";
}

/********************************************************/
/* CLASS: GroundedPreference                            */
/********************************************************/

string GroundedPreference::toString(ParsedTask *task,
                                    vector<GroundedVar> &variables,
                                    vector<string> &preferenceNames) {
  return "[" + preferenceNames[nameIndex] + "] " +
         preference.toString(task, variables);
}

void GroundedPreference::writePDDLPreference(ofstream &f, ParsedTask *task,
                                             vector<GroundedVar> &variables,
                                             vector<string> &preferenceNames) {
  f << "(preference " << preferenceNames[nameIndex] << " ";
  preference.writePDDLGoal(f, task, variables);
  f << ")";
}

/********************************************************/
/* CLASS: GroundedDuration                              */
/********************************************************/

string GroundedDuration::toString(ParsedTask *task,
                                  vector<GroundedVar> &variables,
                                  vector<GroundedControlVar> &controlVars) {
  string s = "(";
  bool closePar = false;
  if (time == AT_END) {
    s += "at end (";
    closePar = true;
  }
  s += ParsedTask::comparatorToString(comp) + " duration " +
       exp.toString(task, variables, controlVars);
  if (closePar)
    s += ")";
  return s + ")";
}

void GroundedDuration::writePDDLDuration(ofstream &f, ParsedTask *task,
                                         vector<GroundedVar> &variables) {
  f << "(";
  if (time == AT_END)
    f << "at end (";
  f << ParsedTask::comparatorToString(comp) << " ?duration ";
  exp.writePDDLNumericExpression(f, task, variables, false);
  if (time == AT_END)
    f << ")";
  f << ")";
}

/********************************************************/
/* CLASS: Action                                        */
/********************************************************/

string GroundedAction::getName(ParsedTask *task) {
  string s = name;
  for (unsigned int i = 0; i < parameters.size(); i++)
    s += " " + task->objects[parameters[i]].name;
  return s;
}

string GroundedAction::toString(ParsedTask *task,
                                vector<GroundedVar> &variables,
                                vector<string> &preferenceNames) {
  string s = name;
  for (unsigned int i = 0; i < parameters.size(); i++)
    s += " " + task->objects[parameters[i]].name;
  for (unsigned int i = 0; i < controlVars.size(); i++)
    s += "\n\t" + controlVars[i].toString();
  for (unsigned int i = 0; i < duration.size(); i++)
    s += "\n\t" + duration[i].toString(task, variables, controlVars);
  for (unsigned int i = 0; i < startCond.size(); i++)
    s += "\n\tPREC: (at-start " + startCond[i].toString(task, variables) + ")";
  for (unsigned int i = 0; i < startNumCond.size(); i++)
    s += "\n\tPREN: (at-start " +
         startNumCond[i].toString(task, variables, controlVars) + ")";
  for (unsigned int i = 0; i < overCond.size(); i++)
    s += "\n\tPREC: (over-all " + overCond[i].toString(task, variables) + ")";
  for (unsigned int i = 0; i < overNumCond.size(); i++)
    s += "\n\tPREN: (over-all " +
         overNumCond[i].toString(task, variables, controlVars) + ")";
  for (unsigned int i = 0; i < endCond.size(); i++)
    s += "\n\tPREC: (at-end " + endCond[i].toString(task, variables) + ")";
  for (unsigned int i = 0; i < endNumCond.size(); i++)
    s += "\n\tPREN: (at-end " +
         endNumCond[i].toString(task, variables, controlVars) + ")";
  for (unsigned int i = 0; i < startEff.size(); i++)
    s += "\n\tEFF.: (at-start " + startEff[i].toString(task, variables) + ")";
  for (unsigned int i = 0; i < startNumEff.size(); i++)
    s += "\n\tEFFN: (at-start " +
         startNumEff[i].toString(task, variables, controlVars) + ")";
  for (unsigned int i = 0; i < endEff.size(); i++)
    s += "\n\tEFF.: (at-end " + endEff[i].toString(task, variables) + ")";
  for (unsigned int i = 0; i < endNumEff.size(); i++)
    s += "\n\tEFFN: (at-end " +
         endNumEff[i].toString(task, variables, controlVars) + ")";
  for (unsigned int i = 0; i < preferences.size(); i++)
    s += "\n\tPREF: " +
         preferences[i].toString(task, variables, preferenceNames);
  for (unsigned int i = 0; i < conditionalEffect.size(); i++) {
    s += "\n\tCOND.EFF:";
    GroundedConditionalEffect &e = conditionalEffect[i];
    for (GroundedCondition &c : e.startCond)
      s += "\n\t\tCOND: (at-start " + c.toString(task, variables) + ")";
    for (GroundedCondition &c : e.endCond)
      s += "\n\t\tCOND: (at-end " + c.toString(task, variables) + ")";
    for (GroundedNumericCondition &c : e.startNumCond)
      s += "\n\t\tCONDN: (at-start " +
           c.toString(task, variables, controlVars) + ")";
    for (GroundedNumericCondition &c : e.endNumCond)
      s += "\n\t\tCONDN: (at-end" + c.toString(task, variables, controlVars) +
           ")";
    for (GroundedCondition &c : e.startEff)
      s += "\n\t\tEFF: (at-start " + c.toString(task, variables) + ")";
    for (GroundedCondition &c : e.endEff)
      s += "\n\t\tEFF: (at-end" + c.toString(task, variables) + ")";
    for (GroundedNumericEffect &c : e.startNumEff)
      s += "\n\t\tEFFN: (at-start " + c.toString(task, variables, controlVars) +
           ")";
    for (GroundedNumericEffect &c : e.endNumEff)
      s += "\n\t\tEFFN: (at-end " + c.toString(task, variables, controlVars) +
           ")";
  }
  return s;
}

void GroundedAction::writePDDLAction(ofstream &f, ParsedTask *task,
                                     vector<GroundedVar> &variables,
                                     vector<string> &preferenceNames) {
  f << "(:durative-action " << name;
  for (unsigned int i = 0; i < parameters.size(); i++)
    f << "_" << task->objects[parameters[i]].name;
  f << endl << ":parameters ( )" << endl;
  writePDDLDuration(f, task, variables);
  writePDDLCondition(f, task, variables, preferenceNames);
  writePDDLEffect(f, task, variables);
  f << ")" << endl;
}

void GroundedAction::writePDDLDuration(ofstream &f, ParsedTask *task,
                                       vector<GroundedVar> &variables) {
  f << ":duration ";
  if (duration.size() == 0)
    f << "( )";
  else if (duration.size() == 1)
    duration[0].writePDDLDuration(f, task, variables);
  else {
    f << "(and";
    for (unsigned int i = 0; i < duration.size(); i++) {
      f << " ";
      duration[i].writePDDLDuration(f, task, variables);
    }
    f << ")";
  }
  f << endl;
}

void GroundedAction::writePDDLCondition(ofstream &f, ParsedTask *task,
                                        vector<GroundedVar> &variables,
                                        vector<string> &preferenceNames) {
  unsigned int numConditions = startCond.size() + overCond.size() +
                               endCond.size() + startNumCond.size() +
                               overNumCond.size() + endNumCond.size() +
                               preferences.size();
  f << ":condition";
  if (numConditions == 0)
    f << " ( )";
  else {
    if (numConditions > 1)
      f << " (and ";
    for (unsigned int i = 0; i < startCond.size(); i++) {
      f << endl << "  (at start ";
      startCond[i].writePDDLCondition(f, task, variables, false);
      f << ")";
    }
    for (unsigned int i = 0; i < startNumCond.size(); i++) {
      f << endl << "  (at start ";
      startNumCond[i].writePDDLCondition(f, task, variables, false);
      f << ")";
    }
    for (unsigned int i = 0; i < overCond.size(); i++) {
      f << endl << "  (over all ";
      overCond[i].writePDDLCondition(f, task, variables, false);
      f << ")";
    }
    for (unsigned int i = 0; i < overNumCond.size(); i++) {
      f << endl << "  (over all ";
      overNumCond[i].writePDDLCondition(f, task, variables, false);
      f << ")";
    }
    for (unsigned int i = 0; i < endCond.size(); i++) {
      f << endl << "  (at end ";
      endCond[i].writePDDLCondition(f, task, variables, false);
      f << ")";
    }
    for (unsigned int i = 0; i < endNumCond.size(); i++) {
      f << endl << "  (at end ";
      endNumCond[i].writePDDLCondition(f, task, variables, false);
      f << ")";
    }
    for (unsigned int i = 0; i < preferences.size(); i++) {
      f << endl << "  ";
      preferences[i].writePDDLPreference(f, task, variables, preferenceNames);
    }
    if (numConditions > 1)
      f << ")";
  }
  f << endl;
}

void GroundedAction::writePDDLEffect(ofstream &f, ParsedTask *task,
                                     vector<GroundedVar> &variables) {
  unsigned int numEffects =
      startEff.size() + endEff.size() + startNumEff.size() + endNumEff.size();
  f << ":effect";
  if (numEffects == 0)
    f << " ( )";
  else {
    if (numEffects > 1)
      f << " (and ";
    for (unsigned int i = 0; i < startEff.size(); i++) {
      f << endl << "  (at start ";
      startEff[i].writePDDLCondition(f, task, variables, false);
      f << ")";
    }
    for (unsigned int i = 0; i < startNumEff.size(); i++) {
      f << endl << "  (at start ";
      startNumEff[i].writePDDLEffect(f, task, variables);
      f << ")";
    }
    for (unsigned int i = 0; i < endEff.size(); i++) {
      f << endl << "  (at end ";
      endEff[i].writePDDLCondition(f, task, variables, false);
      f << ")";
    }
    for (unsigned int i = 0; i < endNumEff.size(); i++) {
      f << endl << "  (at end ";
      endNumEff[i].writePDDLEffect(f, task, variables);
      f << ")";
    }
    if (numEffects > 1)
      f << ")";
  }
  f << endl;
}

void GroundedAction::writePDDLGoal(ofstream &f, ParsedTask *task,
                                   std::vector<GroundedVar> &variables,
                                   vector<string> &preferenceNames) {
  unsigned int numConditions =
      startCond.size() + startNumCond.size() + preferences.size();
  if (numConditions == 0)
    f << " ( )";
  else {
    if (numConditions > 1)
      f << "(and ";
    for (unsigned int i = 0; i < startCond.size(); i++) {
      f << endl << "  ";
      startCond[i].writePDDLCondition(f, task, variables, true);
    }
    for (unsigned int i = 0; i < startNumCond.size(); i++) {
      f << endl << "  ";
      startNumCond[i].writePDDLCondition(f, task, variables, true);
    }
    for (unsigned int i = 0; i < preferences.size(); i++) {
      f << endl << "  ";
      preferences[i].writePDDLPreference(f, task, variables, preferenceNames);
    }
    if (numConditions > 1)
      f << ")";
  }
  f << endl;
}

bool GroundedAction::requiresNumericVariable(TVariable v) {
  for (GroundedNumericCondition &c : startNumCond)
    if (c.requiresNumericVariable(v))
      return true;
  for (GroundedNumericCondition &c : overNumCond)
    if (c.requiresNumericVariable(v))
      return true;
  for (GroundedNumericCondition &c : endNumCond)
    if (c.requiresNumericVariable(v))
      return true;
  return false;
}

/********************************************************/
/* CLASS: GroundedConstraint                            */
/********************************************************/

string GroundedConstraint::toString(ParsedTask *task,
                                    vector<GroundedVar> &variables,
                                    vector<std::string> &preferenceNames) {
  string s = "(";
  switch (type) {
  case RT_AND:
    s += "and";
    for (unsigned int i = 0; i < terms.size(); i++)
      s += " " + terms[i].toString(task, variables, preferenceNames);
    break;
  case RT_FORALL:
    s += "forall"; // Forall should have been removed
    break;
  case RT_PREFERENCE:
    s += "preference " + preferenceNames[preferenceIndex] + " " +
         terms[0].toString(task, variables, preferenceNames);
    break;
  case RT_AT_END:
    s += "at end " + goal[0].toString(task, variables);
    break;
  case RT_ALWAYS:
    s += "always " + goal[0].toString(task, variables);
    break;
  case RT_SOMETIME:
    s += "sometime " + goal[0].toString(task, variables);
    break;
  case RT_WITHIN:
    s += "within " + to_string(time[0]) + " " +
         goal[0].toString(task, variables);
    break;
  case RT_AT_MOST_ONCE:
    s += "at-most-once " + goal[0].toString(task, variables);
    break;
  case RT_SOMETIME_AFTER:
    s += "sometime-after " + goal[0].toString(task, variables) + " " +
         goal[1].toString(task, variables);
    break;
  case RT_SOMETIME_BEFORE:
    s += "sometime-before " + goal[0].toString(task, variables) + " " +
         goal[1].toString(task, variables);
    break;
  case RT_ALWAYS_WITHIN:
    s += "always-within " + to_string(time[0]) + " " +
         goal[0].toString(task, variables) + " " +
         goal[1].toString(task, variables);
    break;
  case RT_HOLD_DURING:
    s += "hold-during " + to_string(time[0]) + " " + to_string(time[1]) + " " +
         goal[0].toString(task, variables);
    break;
  case RT_HOLD_AFTER:
    s += "hold-after " + to_string(time[0]) + " " +
         goal[0].toString(task, variables);
    break;
  case RT_GOAL_PREFERENCE:
    s += "preference " + preferenceNames[preferenceIndex] + " " +
         goal[0].toString(task, variables);
    break;
  }
  return s + ")";
}

/********************************************************/
/* CLASS: GroundedTask                                  */
/********************************************************/

GroundedTask::GroundedTask(ParsedTask *parsedTask) { task = parsedTask; }

string GroundedTask::toString() {
  string s = "VARIABLES: " + to_string(variables.size()) + ":";
  for (unsigned int i = 0; i < variables.size(); i++) {
    s += "\n* Var. " + to_string(i) + ": " + variables[i].toString(task);
    if (!variables[i].isNumeric) {
      s += "\n  Values:";
      for (unsigned int j = 0; j < task->objects.size(); j++)
        if (reachedValues[i][j] != MAX_UNSIGNED_INT)
          s += " (" + task->objects[j].name + ")" +
               to_string(reachedValues[i][j]);
    }
  }
  s += "\nACTIONS: " + to_string(actions.size()) + ":";
  for (unsigned int i = 0; i < actions.size(); i++)
    s += "\n* Act. " + to_string(i) + ": " +
         actions[i].toString(task, variables, preferenceNames);
  return s;
}

void GroundedTask::writePDDLDomain() {
  ofstream f;
  f.open("GroundedDomain.pddl");
  f << "(define (domain " << task->domainName << ")" << endl;
  writePDDLRequirements(f);
  writePDDLTypes(f);
  writePDDLConstants(f);
  writePDDLPredicates(f);
  writePDDLFunctions(f);
  writePDDLActions(f);
  writePDDLDerivedPredicates(f);
  f << ")" << endl;
  f.close();
}

void GroundedTask::writePDDLRequirements(ofstream &f) {
  if (task->requirements.size() == 0)
    return;
  f << "(:requirements";
  for (unsigned int i = 0; i < task->requirements.size(); i++)
    f << " :" << task->requirements[i];
  f << ")" << endl;
}

void GroundedTask::writePDDLTypes(std::ofstream &f) {
  unsigned int startType = 0;
  while (startType < task->types.size() &&
         (task->types[startType].name.compare("#boolean") == 0 ||
          task->types[startType].name.compare("number") == 0 ||
          task->types[startType].name.compare("object") == 0))
    startType++;
  if (startType >= task->types.size())
    return;
  f << "(:types" << endl;
  for (unsigned int i = startType; i < task->types.size(); i++) {
    Type &t = task->types[i];
    f << "  " << t.name;
    writePDDLTypes(f, t.parentTypes, task);
    f << endl;
  }
  f << ")" << endl;
}

void GroundedTask::writePDDLTypes(ofstream &f, vector<unsigned int> &types,
                                  ParsedTask *task) {
  if (types.size() > 0) {
    f << " - ";
    if (types.size() == 1)
      f << task->types[types[0]].name;
    else {
      f << "(either";
      for (unsigned int j = 0; j < types.size(); j++)
        f << " " << task->types[types[j]].name;
      f << ")";
    }
  }
}

void GroundedTask::writePDDLConstants(std::ofstream &f) {
  unsigned int numConstants = 0;
  for (unsigned int i = 0; i < task->objects.size(); i++)
    if (task->objects[i].name.compare("#true") != 0 &&
        task->objects[i].name.compare("#false") != 0)
      numConstants++;
  if (numConstants == 0)
    return;
  f << "(:constants" << endl;
  for (unsigned int i = 0; i < task->objects.size(); i++) {
    Object &o = task->objects[i];
    if (o.name.compare("#true") == 0 || o.name.compare("#false") == 0)
      continue;
    f << "  " << o.name;
    writePDDLTypes(f, o.types, task);
    f << endl;
  }
  f << ")" << endl;
}

void GroundedTask::writePDDLPredicates(std::ofstream &f) {
  unsigned int numPredicates = 0;
  for (unsigned int i = 0; i < task->functions.size(); i++)
    if (task->isBooleanFunction(i))
      numPredicates++;
  if (numPredicates == 0)
    return;
  f << "(:predicates" << endl;
  for (unsigned int i = 0; i < task->functions.size(); i++)
    if (task->isBooleanFunction(i)) {
      Function &fnc = task->functions[i];
      f << "  (" << fnc.name;
      writePDDLParameters(f, fnc.parameters);
      f << ")" << endl;
    }
  f << ")" << endl;
}

void GroundedTask::writePDDLParameters(ofstream &f,
                                       vector<Variable> parameters) {
  for (unsigned int i = 0; i < parameters.size(); i++) {
    f << " " << parameters[i].name;
    writePDDLTypes(f, parameters[i].types, task);
  }
}

void GroundedTask::writePDDLFunctions(ofstream &f) {
  unsigned int numFunctions = 0;
  for (unsigned int i = 0; i < task->functions.size(); i++)
    if (!task->isBooleanFunction(i))
      numFunctions++;
  if (numFunctions == 0)
    return;
  f << "(:functions" << endl;
  for (unsigned int i = 0; i < task->functions.size(); i++)
    if (!task->isBooleanFunction(i)) {
      Function &fnc = task->functions[i];
      f << "  (" << fnc.name;
      writePDDLParameters(f, fnc.parameters);
      f << ")";
      writePDDLTypes(f, fnc.valueTypes, task);
      f << endl;
    }
  f << ")" << endl;
}

void GroundedTask::writePDDLActions(ofstream &f) {
  for (unsigned int i = 0; i < actions.size(); i++)
    actions[i].writePDDLAction(f, task, variables, preferenceNames);
}

void GroundedTask::writePDDLDerivedPredicates(std::ofstream &f) {
  for (unsigned int i = 0; i < task->derivedPredicates.size(); i++) {
    DerivedPredicate &p = task->derivedPredicates[i];
    f << "(:derived (" << p.function.name << " ";
    writePDDLParameters(f, p.function.parameters);
    f << ")" << endl;
    writePDDLGoalDescription(f, p.goal, p.function.parameters);
    f << ")" << endl;
  }
}

void GroundedTask::writePDDLGoalDescription(std::ofstream &f,
                                            GoalDescription &g,
                                            vector<Variable> parameters) {
  if (g.time != NONE)
    f << "(" << ParsedTask::timeToString(g.time);
  switch (g.type) {
  case GD_LITERAL:
  case GD_NEG_LITERAL:
    if (g.type == GD_NEG_LITERAL)
      f << "(not ";
    writePDDLLiteral(f, g.literal, parameters);
    if (g.type == GD_NEG_LITERAL)
      f << ")";
    break;
  case GD_AND:
  case GD_NOT:
  case GD_OR:
  case GD_IMPLY:
    if (g.type == GD_AND)
      f << "(and";
    else if (g.type == GD_OR)
      f << "(or";
    else if (g.type == GD_NOT)
      f << "(not";
    else
      f << "(imply";
    for (unsigned int i = 0; i < g.terms.size(); i++) {
      f << " ";
      writePDDLGoalDescription(f, g.terms[i], parameters);
    }
    f << ")";
    break;
  case GD_EXISTS:
  case GD_FORALL:
    if (g.type == GD_EXISTS)
      f << "(exists (";
    else
      f << "(forall (";
    for (unsigned int i = 0; i < g.parameters.size(); i++) {
      if (i > 0)
        f << " ";
      f << g.parameters[i].name;
      writePDDLTypes(f, g.parameters[i].types, task);
      parameters.push_back(g.parameters[i]);
    }
    f << ") ";
    writePDDLGoalDescription(f, g.terms[0], parameters);
    f << ")";
    break;
  case GD_F_CMP:
    if (g.comparator == CMP_NEQ)
      f << "(not (= ";
    else
      f << "(" << ParsedTask::comparatorToString(g.comparator) << " ";
    writePDDLNumericExpression(f, g.exp[0], parameters);
    f << " ";
    writePDDLNumericExpression(f, g.exp[1], parameters);
    f << ")";
    if (g.comparator == CMP_NEQ)
      f << ")";
    break;
  case GD_EQUALITY:
  case GD_INEQUALITY:
    if (g.type == GD_INEQUALITY)
      f << "(not ";
    f << "(= ";
    writePDDLTerm(f, g.eqTerms[0], parameters);
    f << " ";
    writePDDLTerm(f, g.eqTerms[1], parameters);
    f << ")";
    if (g.type == GD_INEQUALITY)
      f << ")";
    break;
  }
  if (g.time != NONE)
    f << ")";
}

void GroundedTask::writePDDLLiteral(ofstream &f, Literal &l,
                                    vector<Variable> &parameters) {
  f << "(" << task->functions[l.fncIndex].name;
  for (unsigned int i = 0; i < l.params.size(); i++) {
    f << " ";
    writePDDLTerm(f, l.params[i], parameters);
  }
  f << ")";
}

void GroundedTask::writePDDLTerm(ofstream &f, Term &t,
                                 vector<Variable> &parameters) {
  if (t.type != TERM_CONSTANT)
    f << parameters[t.index].name;
  else
    f << task->objects[t.index].name;
}

void GroundedTask::writePDDLNumericExpression(
    std::ofstream &f, NumericExpression &e, std::vector<Variable> &parameters) {
  if (e.type == NET_NUMBER)
    f << e.value;
  else if (e.type == NET_FUNCTION)
    writePDDLLiteral(f, e.function, parameters);
  else if (e.type == NET_TERM)
    writePDDLTerm(f, e.term, parameters);
  else {
    switch (e.type) {
    case NET_NEGATION:
    case NET_SUB:
      f << "(-";
      break;
    case NET_SUM:
      f << "(+";
      break;
    case NET_MUL:
      f << "(*";
      break;
    case NET_DIV:
      f << "(/";
      break;
    default:;
    }
    for (unsigned int i = 0; i < e.operands.size(); i++) {
      f << " ";
      writePDDLNumericExpression(f, e.operands[i], parameters);
    }
    f << ")";
  }
}

void GroundedTask::writePDDLProblem() {
  ofstream f;
  f.open("GroundedProblem.pddl");
  f << "(define (problem " << task->problemName << ")" << endl;
  f << "(:domain " << task->domainName << ")" << endl;
  f << "(:objects" << endl;
  for (unsigned int i = 0; i < task->objects.size(); i++) {
    Object &o = task->objects[i];
    if (o.name.compare("#true") == 0 || o.name.compare("#false") == 0)
      continue;
    f << "  " << o.name;
    writePDDLTypes(f, o.types, task);
    f << endl;
  }
  f << ")" << endl;
  writePDDLInitialState(f);
  writePDDLGoal(f);
  writePDDLConstraints(f);
  writePDDLMetric(f);
  if (task->serialLength != -1 || task->parallelLength != -1) {
    f << "(:length";
    if (task->serialLength != -1)
      f << " (:serial " << task->serialLength << ")";
    if (task->parallelLength != -1)
      f << " (:parallel " << task->parallelLength << ")";
    f << ")" << endl;
  }
  f << ")" << endl;
  f.close();
}

void GroundedTask::writePDDLInitialState(ofstream &f) {
  f << "(:init" << endl;
  for (unsigned int i = 0; i < task->init.size(); i++) {
    f << "  ";
    writePDDLFact(f, task->init[i]);
    f << endl;
  }
  f << ")" << endl;
}

void GroundedTask::writePDDLFact(ofstream &f, Fact &fact) {
  if (fact.time > 0)
    f << "(at " << fact.time << " (";
  else
    f << "(";
  if (task->isBooleanFunction(fact.function)) {
    if (fact.value == task->CONSTANT_FALSE)
      f << "not (";
    f << task->functions[fact.function].name;
    for (unsigned int i = 0; i < fact.parameters.size(); i++)
      f << " " << task->objects[fact.parameters[i]].name;
    if (fact.value == task->CONSTANT_FALSE)
      f << ")";
  } else {
    f << "= (" << task->functions[fact.function].name;
    for (unsigned int i = 0; i < fact.parameters.size(); i++)
      f << " " << task->objects[fact.parameters[i]].name;
    f << ") ";
    if (fact.valueIsNumeric)
      f << fact.numericValue;
    else
      f << task->objects[fact.value].name;
  }
  f << ")";
  if (fact.time > 0)
    f << ")";
}

void GroundedTask::writePDDLGoal(ofstream &f) {
  f << "(:goal ";
  if (goals.size() > 0) {
    if (goals.size() == 1)
      goals[0].writePDDLGoal(f, task, variables, preferenceNames);
    else {
      f << "(or";
      for (unsigned int i = 0; i < goals.size(); i++) {
        f << endl << " ";
        goals[i].writePDDLGoal(f, task, variables, preferenceNames);
      }
      f << ")";
    }
  }
  f << ")" << endl;
}

void GroundedTask::writePDDLPrecondition(ofstream &f, Precondition &p,
                                         vector<Variable> parameters) {
  switch (p.type) {
  case PT_LITERAL:
  case PT_NEG_LITERAL:
    if (p.type == PT_NEG_LITERAL)
      f << "(not ";
    writePDDLLiteral(f, p.literal, parameters);
    if (p.type == PT_NEG_LITERAL)
      f << ")";
    break;
  case PT_AND:
  case PT_NOT:
  case PT_OR:
  case PT_IMPLY:
    if (p.type == PT_AND)
      f << "(and";
    else if (p.type == PT_OR)
      f << "(or";
    else if (p.type == PT_NOT)
      f << "(not";
    else
      f << "(imply";
    for (unsigned int i = 0; i < p.terms.size(); i++) {
      f << " ";
      writePDDLPrecondition(f, p.terms[i], parameters);
      f << endl;
    }
    f << ")";
    break;
  case PT_EXISTS:
  case PT_FORALL:
    if (p.type == PT_EXISTS)
      f << "(exists (";
    else
      f << "(forall (";
    for (unsigned int i = 0; i < p.parameters.size(); i++) {
      if (i > 0)
        f << " ";
      f << p.parameters[i].name;
      writePDDLTypes(f, p.parameters[i].types, task);
      parameters.push_back(p.parameters[i]);
    }
    f << ") ";
    writePDDLPrecondition(f, p.terms[0], parameters);
    f << ")";
    break;
  case PT_F_CMP:
  case PT_EQUALITY:
  case PT_PREFERENCE:
  case PT_GOAL:
    if (p.type == PT_PREFERENCE)
      f << "(preference " << p.preferenceName << " ";
    writePDDLGoalDescription(f, p.goal, parameters);
    if (p.type == PT_PREFERENCE)
      f << ")";
    break;
  }
}

void GroundedTask::writePDDLConstraints(ofstream &f) {
  if (task->constraints.size() == 0)
    return;
  f << "(:constraints" << endl;
  for (unsigned int i = 0; i < task->constraints.size(); i++) {
    f << " ";
    vector<Variable> parameters;
    writePDDLConstraint(f, task->constraints[i], parameters);
    f << endl;
  }
  f << ")" << endl;
}

void GroundedTask::writePDDLConstraint(std::ofstream &f, Constraint &c,
                                       vector<Variable> parameters) {
  f << "(";
  switch (c.type) {
  case RT_AND:
    f << "and";
    for (unsigned int i = 0; i < c.terms.size(); i++) {
      f << endl << " ";
      writePDDLConstraint(f, c.terms[i], parameters);
    }
    break;
  case RT_FORALL:
    f << "forall (";
    for (unsigned int i = 0; i < c.parameters.size(); i++) {
      if (i > 0)
        f << " ";
      f << c.parameters[i].name;
      writePDDLTypes(f, c.parameters[i].types, task);
      parameters.push_back(c.parameters[i]);
    }
    f << ") ";
    writePDDLConstraint(f, c.terms[0], parameters);
    break;
  case RT_PREFERENCE:
    f << "preference " << c.preferenceName << " ";
    writePDDLConstraint(f, c.terms[0], parameters);
    break;
  case RT_AT_END:
    f << "at end ";
    writePDDLGoalDescription(f, c.goal[0], parameters);
    break;
  case RT_ALWAYS:
    f << "always ";
    writePDDLGoalDescription(f, c.goal[0], parameters);
    break;
  case RT_SOMETIME:
    f << "sometime ";
    writePDDLGoalDescription(f, c.goal[0], parameters);
    break;
  case RT_WITHIN:
    f << "within " << c.time[0] << " ";
    writePDDLGoalDescription(f, c.goal[0], parameters);
    break;
  case RT_AT_MOST_ONCE:
    f << "at-most-once ";
    writePDDLGoalDescription(f, c.goal[0], parameters);
    break;
  case RT_SOMETIME_AFTER:
    f << "sometime-after ";
    writePDDLGoalDescription(f, c.goal[0], parameters);
    f << " ";
    writePDDLGoalDescription(f, c.goal[1], parameters);
    break;
  case RT_SOMETIME_BEFORE:
    f << "sometime-before ";
    writePDDLGoalDescription(f, c.goal[0], parameters);
    f << " ";
    writePDDLGoalDescription(f, c.goal[1], parameters);
    break;
  case RT_ALWAYS_WITHIN:
    f << "always-within " << c.time[0] << " ";
    writePDDLGoalDescription(f, c.goal[0], parameters);
    f << " ";
    writePDDLGoalDescription(f, c.goal[1], parameters);
    break;
  case RT_HOLD_DURING:
    f << "hold-during " << c.time[0] << " " << c.time[1] << " ";
    writePDDLGoalDescription(f, c.goal[0], parameters);
    break;
  case RT_HOLD_AFTER:
    f << "hold-after " << c.time[0] << " ";
    writePDDLGoalDescription(f, c.goal[0], parameters);
    break;
  case RT_GOAL_PREFERENCE:
    f << "preference " << c.preferenceName << " ";
    writePDDLGoalDescription(f, c.goal[0], parameters);
    break;
  }
  f << ")";
}

void GroundedTask::writePDDLMetric(ofstream &f) {
  if (task->metricType == MT_NONE)
    return;
  f << "(:metric ";
  if (task->metricType == MT_MINIMIZE)
    f << "minimize ";
  else
    f << "maximize ";
  writePDDLMetric(f, task->metric);
  f << ")" << endl;
}

void GroundedTask::writePDDLMetric(ofstream &f, Metric &m) {
  switch (m.type) {
  case MT_TOTAL_TIME:
    f << "total-time";
    break;
  case MT_NUMBER:
    f << m.value;
    break;
  case MT_PLUS:
  case MT_MINUS:
  case MT_PROD:
  case MT_DIV:
    f << "(";
    if (m.type == MT_PLUS)
      f << "+";
    else if (m.type == MT_MINUS)
      f << "-";
    else if (m.type == MT_PROD)
      f << "*";
    else
      f << "/";
    for (unsigned int i = 0; i < m.terms.size(); i++) {
      f << " ";
      writePDDLMetric(f, m.terms[i]);
    }
    f << ")";
    break;
  case MT_IS_VIOLATED:
    f << "(is-violated " << m.preferenceName << ")";
    break;
  case MT_FLUENT:
    f << "(" << task->functions[m.function].name;
    for (unsigned int i = 0; i < m.parameters.size(); i++)
      f << " " << task->objects[m.parameters[i]].name;
    f << ")";
  }
}

bool GroundedConditionalEffect::requiresNumericVariable(TVariable v) {
  for (GroundedNumericCondition &c : startNumCond)
    if (c.requiresNumericVariable(v))
      return true;
  for (GroundedNumericCondition &c : endNumCond)
    if (c.requiresNumericVariable(v))
      return true;
  return false;
}
