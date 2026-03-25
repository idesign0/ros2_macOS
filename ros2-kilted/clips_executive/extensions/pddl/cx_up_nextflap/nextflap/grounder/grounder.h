#ifndef GROUNDER_H
#define GROUNDER_H

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* April 2022                                           */
/********************************************************/
/* Grounds all possible actions from the                */
/* PreprocessedTask.                                    */
/********************************************************/

#include "../preprocess/preprocessedTask.h"
#include "groundedTask.h"

// EPSILON for temporal scheduling
#define EPSILON 0.001f

// Class for assigments grounding
class GrounderAssignment {
public:
  unsigned int fncIndex;
  std::vector<Term> *params;
  bool grounded;
  Term *value;
  GrounderAssignment(OpFluent &f);
};

// Class for operators grounding
class GrounderOperator {
public:
  int index;
  Operator *op;
  unsigned int numParams;
  std::vector<unsigned int> *paramValues;
  std::vector<unsigned int> *compatibleObjectsWithParam;
  unsigned int newValueIndex;
  std::vector<GrounderAssignment> preconditions;

  void initialize(Operator &o);
  ~GrounderOperator();
};

// Class to program facts in the initial state
class ProgrammedValue {
public:
  unsigned int index;
  unsigned int varIndex;
  unsigned int valueIndex;
  ProgrammedValue(unsigned int index, unsigned int varIndex,
                  unsigned int valueIndex);
};

// Pairs of (variable, value)
class VariableValue {
public:
  bool valueIsNumeric;
  unsigned int value;
  float numericValue;
};

// Class for task grounding
class Grounder {
private:
  PreprocessedTask *prepTask;
  GroundedTask *gTask;
  bool **typesMatrix;
  unsigned int numOps;
  GrounderOperator *ops;
  std::vector<GrounderOperator *> *opRequireFunction;
  std::unordered_map<std::string, unsigned int> variableIndex;
  std::unordered_map<std::string, unsigned int> preferenceIndex;
  std::vector<ProgrammedValue> *newValues;
  std::vector<ProgrammedValue> *auxValues;
  std::vector<ProgrammedValue> *valuesByFunction;
  std::unordered_map<std::string, unsigned int> groundedActions;
  unsigned int numValues;
  unsigned int startNewValues;
  unsigned int currentLevel;

  std::string getVariableName(unsigned int function,
                              const std::vector<unsigned int> &parameters);
  std::string getVariableName(const Literal &l,
                              const std::vector<unsigned int> &opParameters);
  void initTypesMatrix();
  void clearMemory();
  void addTypeToMatrix(unsigned int typeIndex, unsigned int subtypeIndex);
  void initOperators();
  void addOpToRequireFunction(GrounderOperator *op, unsigned int f);
  void initInitialState();
  void createVariable(const Fact &f);
  unsigned int getVariableIndex(const Fact &f);
  unsigned int getVariableIndex(const Literal &l,
                                const std::vector<unsigned int> &opParameters);
  void groundRemainingParameters(GrounderOperator &op);
  void groundAction(GrounderOperator &op);
  bool objectIsCompatible(unsigned int objIndex,
                          std::vector<unsigned int> &types);
  void match(ProgrammedValue &pv);
  void swapLevels();
  int matches(GrounderOperator *op, unsigned int varIndex,
              unsigned int valueIndex, int startPrec);
  void stackParameters(GrounderOperator *op, int precIndex,
                       unsigned int varIndex, unsigned int valueIndex);
  void completeMatch(GrounderOperator *op, unsigned int precIndex);
  void unstackParameters(GrounderOperator *op, int precIndex);
  bool precMatches(GrounderOperator *op, GrounderAssignment &p,
                   unsigned int varIndex, unsigned int valueIndex);
  bool checkEqualityConditions(GrounderOperator &op, GroundedAction &a);
  bool groundPreconditions(GrounderOperator &op, GroundedAction &a);
  bool groundPreconditions(std::vector<OpFluent> &opCond,
                           std::vector<unsigned int> &parameters,
                           std::vector<GroundedCondition> &aCond);
  bool groundPreconditions(std::vector<OpNumericPrec> &opCond,
                           std::vector<unsigned int> &parameters,
                           std::vector<GroundedNumericCondition> &aCond);
  unsigned int createNewVariable(const Literal &l,
                                 const std::vector<unsigned int> &opParameters);
  GroundedNumericExpression
  groundNumericExpression(OpEffectExpression &exp,
                          std::vector<unsigned int> &parameters);
  bool groundEffects(GrounderOperator &op, GroundedAction &a);
  bool groundEffects(std::vector<OpFluent> &opEff,
                     std::vector<unsigned int> &parameters,
                     std::vector<GroundedCondition> &aEff);
  bool groundEffects(std::vector<OpEffect> &opEff, GroundedAction &a,
                     TimeSpecifier ts);
  bool groundEffects(std::vector<OpEffect> &opEff,
                     std::vector<unsigned int> &parameters,
                     std::vector<GroundedNumericEffect> &aEff);
  bool groundConditionalEffects(GrounderOperator &op, GroundedAction &a);
  void groundConditionalEffect(OpConditionalEffect &e, GroundedAction &a);
  bool groundPreferences(GrounderOperator &op, GroundedAction &a);
  bool groundDuration(GrounderOperator &op, GroundedAction &a);
  void programNewValue(GroundedCondition &eff);
  GroundedNumericExpression
  groundNumericExpression(NumericExpression &exp,
                          std::vector<unsigned int> &parameters);
  GroundedGoalDescription
  groundGoalDescription(GoalDescription &g,
                        std::vector<unsigned int> &parameters);
  bool canGroundVariable(Literal &literal, unsigned int numParameters);
  void addVariableComparison(GroundedGoalDescription &g, Literal &literal,
                             bool valueIsParam, unsigned int valueIndex,
                             std::vector<unsigned int> &parameters, bool equal);
  PartiallyGroundedNumericExpression
  partiallyGroundNumericExpression(NumericExpression &exp,
                                   std::vector<unsigned int> &parameters);
  void removeStaticVariables();
  void checkStaticVariables(GroundedAction &a, std::vector<bool> &staticVar);
  void getInitialValues(unsigned int varIndex, std::vector<Fact *> &initValues);
  void removeStaticVariables(std::vector<bool> &staticVar,
                             std::vector<unsigned int> &newIndex,
                             std::vector<VariableValue> &value);
  void groupVariables(std::vector<bool> &staticVar,
                      std::vector<unsigned int> &newIndex);
  bool removeStaticVariables(GroundedNumericExpression &e,
                             std::vector<bool> &staticVar,
                             std::vector<unsigned int> &newIndex,
                             std::vector<VariableValue> &value);
  float computeExpressionValue(GroundedNumericExpression &e);
  bool removeStaticVariables(std::vector<GroundedCondition> &cond,
                             std::vector<bool> &staticVar,
                             std::vector<unsigned int> &newIndex,
                             std::vector<VariableValue> &value);
  bool removeStaticVariables(std::vector<GroundedNumericCondition> &cond,
                             std::vector<bool> &staticVar,
                             std::vector<unsigned int> &newIndex,
                             std::vector<VariableValue> &value);
  bool numericComparisonHolds(GroundedNumericCondition &c);
  bool removeStaticVariables(std::vector<GroundedNumericEffect> &e,
                             std::vector<bool> &staticVar,
                             std::vector<unsigned int> &newIndex,
                             std::vector<VariableValue> &value);
  bool removeStaticVariables(std::vector<GroundedPreference> &p,
                             std::vector<bool> &staticVar,
                             std::vector<unsigned int> &newIndex,
                             std::vector<VariableValue> &value);
  bool removeStaticVariables(GroundedGoalDescription &g,
                             std::vector<bool> &staticVar,
                             std::vector<unsigned int> &newIndex,
                             std::vector<VariableValue> &value);
  bool removeStaticVariables(PartiallyGroundedNumericExpression &e,
                             std::vector<bool> &staticVar,
                             std::vector<unsigned int> &newIndex,
                             std::vector<VariableValue> &value);
  bool removeStaticVariables(GroundedConstraint &c,
                             std::vector<bool> &staticVar,
                             std::vector<unsigned int> &newIndex,
                             std::vector<VariableValue> &value);
  float computeExpressionValue(PartiallyGroundedNumericExpression &e);
  void computeInitialVariableValues();
  void removeADLFeaturesInPreferences();
  void removeADLFeaturesInPreferences(GroundedAction &a);
  void removeADLFeaturesInPreferences(GroundedGoalDescription *pref);
  void replaceADLPreference(
      GroundedGoalDescription *pref, unsigned int numParam,
      unsigned int prevParams,
      std::unordered_map<unsigned int, unsigned int> *parameters,
      GroundedGoalDescription *condition);
  GroundedGoalDescription
  groundPreference(GroundedGoalDescription *condition, unsigned int numParam,
                   std::unordered_map<unsigned int, unsigned int> *parameters);
  PartiallyGroundedNumericExpression groundPartiallyGroundedNumericExpression(
      PartiallyGroundedNumericExpression *exp,
      std::unordered_map<unsigned int, unsigned int> *parameters);
  void removeADLFeaturesInConstraints();
  GroundedConstraint groundConstraint(Constraint *c,
                                      std::vector<unsigned int> &parameters);
  void removeADLFeaturesInConstraint(GroundedConstraint *c);
  void replaceADLConstraint(
      GroundedConstraint *c, unsigned int numParam, unsigned int prevParams,
      std::unordered_map<unsigned int, unsigned int> *parameters,
      GroundedConstraint *condition);
  GroundedConstraint
  groundConstraint(GroundedConstraint *condition, unsigned int numParam,
                   std::unordered_map<unsigned int, unsigned int> *parameters);
  GroundedMetric groundMetric(Metric *m);
  void removeStaticVariables(GroundedMetric &m, std::vector<bool> &staticVar,
                             std::vector<unsigned int> &newIndex,
                             std::vector<VariableValue> &value);
  void checkNumericConditions();
  int checkNumericCondition(GroundedNumericCondition *c);
  void checkNumericEffectsNotRequired();
  void checkNumericEffectsNotRequired(GroundedNumericExpression &e,
                                      GroundedAction &a);
  void
  checkNumericEffectsNotRequired(GroundedNumericExpression &e,
                                 std::vector<GroundedNumericCondition> &aEff,
                                 GroundedAction &a);
  void checkConditionalNumericEffectsNotRequired(
      GroundedNumericExpression &e, std::vector<GroundedNumericCondition> &aEff,
      GroundedConditionalEffect &ce, GroundedAction &a);
  void addDummyNumericPrecondition(std::vector<GroundedNumericCondition> &cond,
                                   TVariable v);
  bool isBoolean(unsigned int value) {
    return value == gTask->task->CONSTANT_TRUE ||
           value == gTask->task->CONSTANT_FALSE;
  }

public:
  GroundedTask *groundTask(PreprocessedTask *prepTask, bool keepStaticData);
};

#endif
