#ifndef GROUNDED_TASK_H
#define GROUNDED_TASK_H

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* April 2022                                           */
/********************************************************/
/* Grounded task obtained after the grounding process.  */
/********************************************************/

#include "../parser/parsedTask.h"
#include "../utils/utils.h"
#include <cstdint>
#include <fstream>

// Types of the control parameters (real or integer)
enum GroundedControlVarType { GCVT_NUMBER = 0, GCVT_INTEGER = 1 };

// Grounded control parameter
class GroundedControlVar {
public:
  unsigned int index;
  std::string name;
  GroundedControlVarType type;

  GroundedControlVar(Variable &v, unsigned int i, ParsedTask *task) {
    name = v.name;
    index = i;
    if (v.types[0] == task->INTEGER_TYPE)
      type = GCVT_INTEGER;
    else
      type = GCVT_NUMBER;
  }

  std::string toString() {
    return name + " - " + (type == GCVT_INTEGER ? "integer" : "number");
  }
};

// Grounded value
class GroundedValue {
public:
  float time; // 0 if it is not a time-initial literal
  unsigned int value;
  float numericValue;
  std::string toString(ParsedTask *task, bool isNumeric);
};

// Grounded variable
class GroundedVar {
public:
  unsigned int index;
  unsigned int fncIndex;
  std::vector<unsigned int> params;
  bool isNumeric;
  std::vector<GroundedValue> initialValues;
  std::string toString(ParsedTask *task);
  std::string toString(ParsedTask *task, bool isGoal);
};

// Grounded condition
class GroundedCondition {
public:
  unsigned int varIndex;
  unsigned int valueIndex;
  GroundedCondition(unsigned int variable, unsigned int value);
  std::string toString(ParsedTask *task, std::vector<GroundedVar> &variables);
  void writePDDLCondition(std::ofstream &f, ParsedTask *task,
                          std::vector<GroundedVar> &variables, bool isGoal);
};

// Types of numeric expressions
enum GroundedNumericExpressionType {
  GE_NUMBER = 0,
  GE_VAR = 1,
  GE_SUM = 2,
  GE_SUB = 3,
  GE_DIV = 4,
  GE_MUL = 5,
  GE_OBJECT = 6,
  GE_DURATION = 7,
  GE_SHARP_T = 8,
  GE_CONTROL_VAR = 9,
  GE_UNDEFINED = 10
};

// Grounded numeric expression
class GroundedNumericExpression {
public:
  GroundedNumericExpressionType type;
  float value;        // if type == GE_NUMBER
  unsigned int index; // if type == GE_VAR | GE_OBJECT
  std::vector<GroundedNumericExpression>
      terms; // if type == GE_SUM | GE_SUB | GE_DIV | GE_MUL | GE_SHARP_T
  std::string toString(ParsedTask *task, std::vector<GroundedVar> &variables,
                       std::vector<GroundedControlVar> &controlVars);
  void writePDDLNumericExpression(std::ofstream &f, ParsedTask *task,
                                  std::vector<GroundedVar> &variables,
                                  bool isGoal);
  bool requiresNumericVariable(TVariable v);
};

// Types of partially-gorunded numeric expressions
enum PartiallyGroundedNumericExpressionType {
  PGE_NUMBER = 0,
  PGE_VAR = 1,
  PGE_UNGROUNDED_VAR = 2,
  PGE_SUM = 3,
  PGE_SUB = 4,
  PGE_DIV = 5,
  PGE_MUL = 6,
  PGE_TERM = 7,
  PGE_NOT = 8
};

// Partially-grounded numeric expression
class PartiallyGroundedNumericExpression {
public:
  PartiallyGroundedNumericExpressionType type;
  float value;        // if type == PGE_NUMBER
  unsigned int index; // if type == PGE_VAR (var. index) | PGE_UNGROUNDED_VAR
                      // (function index)
  std::vector<bool> isParameter; // if type == PGE_UNGROUNDED_VAR | PGE_TERM
  std::vector<unsigned int>
      paramIndex; // if type == PGE_UNGROUNDED_VAR | PGE_TERM
  std::vector<PartiallyGroundedNumericExpression>
      terms; // if type == PGE_SUM | PGE_SUB | PGE_DIV | PGE_MUL | PGE_NOT
  void addTerm(Term &term, std::vector<unsigned int> &parameters);
  std::string toString(ParsedTask *task, std::vector<GroundedVar> &variables);
  void writePDDLNumericExpression(std::ofstream &f, ParsedTask *task,
                                  std::vector<GroundedVar> &variables);
};

// Grounded numeric condition
class GroundedNumericCondition {
public:
  Comparator comparator;
  std::vector<GroundedNumericExpression> terms;
  std::string toString(ParsedTask *task, std::vector<GroundedVar> &variables,
                       std::vector<GroundedControlVar> &controlVars);
  void writePDDLCondition(std::ofstream &f, ParsedTask *task,
                          std::vector<GroundedVar> &variables, bool isGoal);
  bool requiresNumericVariable(TVariable v);
};

// Grounded numeric effect
class GroundedNumericEffect {
public:
  Assignment assignment;
  unsigned int varIndex;
  GroundedNumericExpression exp;
  std::string toString(ParsedTask *task, std::vector<GroundedVar> &variables,
                       std::vector<GroundedControlVar> &controlVars);
  void writePDDLEffect(std::ofstream &f, ParsedTask *task,
                       std::vector<GroundedVar> &variables);
};

// Types of grounded goal descriptions
enum GroundedGoalDescriptionType {
  GG_FLUENT = 0,
  GG_UNGROUNDED_FLUENT = 1,
  GG_AND = 2,
  GG_OR = 3,
  GG_NOT = 4,
  GG_EXISTS = 5,
  GG_FORALL = 6,
  GG_IMPLY = 7,
  GG_EQUALITY = 8,
  GG_INEQUALITY = 9,
  GG_COMP = 10
};

// Grounded goal description
class GroundedGoalDescription {
private:
  std::string toString(ParsedTask *task, std::vector<GroundedVar> &variables,
                       unsigned int paramNumber);
  void writePDDLGoal(std::ofstream &f, ParsedTask *task,
                     std::vector<GroundedVar> &variables, char paramName);

public:
  TimeSpecifier time;
  GroundedGoalDescriptionType type;
  unsigned int index;            // if type == GG_FLUENT (variable index) |
                                 // GG_UNGROUNDED_FLUENT (function index)
  bool valueIsParam;             // if type == GG_FLUENT | GG_UNGROUNDED_FLUENT
  unsigned int value;            // if type == GG_FLUENT | GG_UNGROUNDED_FLUENT
  bool equal;                    // if type == GG_FLUENT | GG_UNGROUNDED_FLUENT
  std::vector<bool> isParameter; // if type == GG_UNGROUNDED_FLUENT |
                                 // GG_EQUALITY | GG_INEQUALITY
  std::vector<unsigned int> paramIndex; // if type == GG_UNGROUNDED_FLUENT |
                                        // GG_EQUALITY | GG_INEQUALITY
  std::vector<GroundedGoalDescription>
      terms; // if type == GG_AND | GG_OR | GG_NOT | GG_IMPLY | GG_EXISTS |
             // GG_FORALL
  std::vector<std::vector<unsigned int>>
      paramTypes;        // if type == GG_EXISTS | GG_FORALL
  Comparator comparator; // if type == GG_COMP
  std::vector<PartiallyGroundedNumericExpression> exp; // if type == GG_COMP
  void addTerm(Term &term, std::vector<unsigned int> &parameters);
  std::string toString(ParsedTask *task, std::vector<GroundedVar> &variables);
  void writePDDLGoal(std::ofstream &f, ParsedTask *task,
                     std::vector<GroundedVar> &variables);
};

// Grounded preference
class GroundedPreference {
public:
  unsigned int nameIndex;
  GroundedGoalDescription preference;
  std::string toString(ParsedTask *task, std::vector<GroundedVar> &variables,
                       std::vector<std::string> &preferenceNames);
  void writePDDLPreference(std::ofstream &f, ParsedTask *task,
                           std::vector<GroundedVar> &variables,
                           std::vector<std::string> &preferenceNames);
};

// Grounded duration
class GroundedDuration {
public:
  TimeSpecifier time;
  Comparator comp;
  GroundedNumericExpression exp;
  std::string toString(ParsedTask *task, std::vector<GroundedVar> &variables,
                       std::vector<GroundedControlVar> &controlVars);
  void writePDDLDuration(std::ofstream &f, ParsedTask *task,
                         std::vector<GroundedVar> &variables);
};

class GroundedConditionalEffect {
public:
  std::vector<GroundedCondition> startCond;
  std::vector<GroundedCondition> endCond;
  std::vector<GroundedNumericCondition> startNumCond;
  std::vector<GroundedNumericCondition> endNumCond;
  std::vector<GroundedCondition> startEff;
  std::vector<GroundedCondition> endEff;
  std::vector<GroundedNumericEffect> startNumEff;
  std::vector<GroundedNumericEffect> endNumEff;

  bool requiresNumericVariable(TVariable v);
};

// Grounded action
class GroundedAction {
private:
  void writePDDLDuration(std::ofstream &f, ParsedTask *task,
                         std::vector<GroundedVar> &variables);
  void writePDDLEffect(std::ofstream &f, ParsedTask *task,
                       std::vector<GroundedVar> &variables);

public:
  unsigned int index;
  std::string name;
  std::vector<unsigned int> parameters;
  std::vector<GroundedControlVar> controlVars;
  std::vector<GroundedDuration> duration;
  std::vector<GroundedCondition> startCond;
  std::vector<GroundedCondition> overCond;
  std::vector<GroundedCondition> endCond;
  std::vector<GroundedNumericCondition> startNumCond;
  std::vector<GroundedNumericCondition> overNumCond;
  std::vector<GroundedNumericCondition> endNumCond;
  std::vector<GroundedCondition> startEff;
  std::vector<GroundedCondition> endEff;
  std::vector<GroundedNumericEffect> startNumEff;
  std::vector<GroundedNumericEffect> endNumEff;
  std::vector<GroundedPreference> preferences;
  std::vector<GroundedConditionalEffect> conditionalEffect;
  bool instantaneous;
  bool isTIL;
  bool isGoal;

  GroundedAction(bool instantaneous, bool isTIL, bool isGoal) {
    this->instantaneous = instantaneous;
    this->isTIL = isTIL;
    this->isGoal = isGoal;
  }
  std::string getName(ParsedTask *task);
  std::string toString(ParsedTask *task, std::vector<GroundedVar> &variables,
                       std::vector<std::string> &preferenceNames);
  void writePDDLAction(std::ofstream &f, ParsedTask *task,
                       std::vector<GroundedVar> &variables,
                       std::vector<std::string> &preferenceNames);
  void writePDDLCondition(std::ofstream &f, ParsedTask *task,
                          std::vector<GroundedVar> &variables,
                          std::vector<std::string> &preferenceNames);
  void writePDDLGoal(std::ofstream &f, ParsedTask *task,
                     std::vector<GroundedVar> &variables,
                     std::vector<std::string> &preferenceNames);
  bool requiresNumericVariable(TVariable v);
};

// Grounded constraint
class GroundedConstraint {
public:
  ConstraintType type;
  std::vector<GroundedConstraint>
      terms;                    // if type==RT_AND | RT_PREFERENCE | RT_FORALL
  unsigned int preferenceIndex; // if type==RT_PREFERENCE
  std::vector<GroundedGoalDescription>
      goal;                // if type!=RT_AND & RT_PREFERENCE & RT_FORALL
  std::vector<float> time; // if type==RT_WITHIN | RT_ALWAYS_WITHIN |
                           // RT_HOLD_DURING | RT_HOLD_AFTER
  std::vector<std::vector<unsigned int>> paramTypes; // if type == RT_FORALL
  std::string toString(ParsedTask *task, std::vector<GroundedVar> &variables,
                       std::vector<std::string> &preferenceNames);
};

// Grounded metric
class GroundedMetric {
public:
  MetricExpressionType type;
  float value;
  unsigned int index; // Var index or preference index
  std::vector<GroundedMetric> terms;
};

// Grounded task
class GroundedTask {
private:
  void writePDDLRequirements(std::ofstream &f);
  void writePDDLTypes(std::ofstream &f);
  void writePDDLConstants(std::ofstream &f);
  void writePDDLPredicates(std::ofstream &f);
  void writePDDLParameters(std::ofstream &f, std::vector<Variable> parameters);
  void writePDDLFunctions(std::ofstream &f);
  void writePDDLActions(std::ofstream &f);
  void writePDDLDerivedPredicates(std::ofstream &f);
  void writePDDLGoalDescription(std::ofstream &f, GoalDescription &g,
                                std::vector<Variable> parameters);
  void writePDDLLiteral(std::ofstream &f, Literal &l,
                        std::vector<Variable> &parameters);
  void writePDDLTerm(std::ofstream &f, Term &t,
                     std::vector<Variable> &parameters);
  void writePDDLNumericExpression(std::ofstream &f, NumericExpression &e,
                                  std::vector<Variable> &parameters);
  void writePDDLInitialState(std::ofstream &f);
  void writePDDLFact(std::ofstream &f, Fact &fact);
  void writePDDLGoal(std::ofstream &f);
  void writePDDLPrecondition(std::ofstream &f, Precondition &p,
                             std::vector<Variable> parameters);
  void writePDDLConstraints(std::ofstream &f);
  void writePDDLConstraint(std::ofstream &f, Constraint &c,
                           std::vector<Variable> parameters);
  void writePDDLMetric(std::ofstream &f);
  void writePDDLMetric(std::ofstream &f, Metric &m);

public:
  ParsedTask *task;
  GroundedTask(ParsedTask *parsedTask);
  std::vector<GroundedVar> variables;
  std::vector<GroundedAction> actions;
  std::vector<GroundedAction> goals;
  std::vector<std::string> preferenceNames;
  std::vector<std::vector<unsigned int>> reachedValues;
  std::vector<GroundedConstraint> constraints;
  GroundedMetric metric;
  char metricType; // '>' = Maximize, '<' = Minimize , 'X' = no metric specified

  std::string toString();
  void writePDDLDomain();
  static void writePDDLTypes(std::ofstream &f, std::vector<unsigned int> &types,
                             ParsedTask *task);
  void writePDDLProblem();
};

#endif
