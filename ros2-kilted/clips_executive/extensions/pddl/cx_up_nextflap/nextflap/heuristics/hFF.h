#ifndef FF_RPG_H
#define FF_RPG_H

#include "../planner/state.h"
#include "../sas/sasTask.h"
#include "../utils/priorityQueue.h"
#include "../utils/utils.h"

class FF_RPGCondition : public PriorityQueueItem {
public:
  TVariable var;
  TValue value;
  int level;
  FF_RPGCondition(TVariable v, TValue val, int l) {
    var = v;
    value = val;
    level = l;
  }
  inline int compare(PriorityQueueItem *other) {
    return ((FF_RPGCondition *)other)->level - level;
  }
  virtual ~FF_RPGCondition() {}
};

class FF_RPGVarValue {
public:
  TVariable var;
  TValue value;
  FF_RPGVarValue(TVariable var, TValue value);
};

class FF_RPG {
private:
  SASTask *task;
  std::vector<std::vector<int>> literalLevels;
  std::vector<int> actionLevels;
  unsigned int numLevels;
  std::vector<FF_RPGVarValue> *lastLevel;
  std::vector<FF_RPGVarValue> *newLevel;
  std::vector<TVarValue> reachedValues;

  void initialize();
  void addEffects(SASAction *a);
  void addEffect(TVariable var, TValue value);
  void expand();
  void addSubgoals(std::vector<TVarValue> *goals,
                   PriorityQueue *openConditions);
  void addSubgoal(TVariable var, TValue value, PriorityQueue *openConditions);
  void addSubgoals(SASAction *a, PriorityQueue *openConditions);
  uint16_t getDifficulty(SASAction *a);
  uint16_t getDifficulty(SASCondition *c);
  void addTILactions(std::vector<SASAction *> *tilActions);
  uint16_t computeHeuristic(PriorityQueue *openConditions);
  void resetReachedValues();
  bool isExecutable(SASAction *a);

public:
  std::vector<SASAction *> relaxedPlan;

  FF_RPG(TState *fs, std::vector<SASAction *> *tilActions, SASTask *task);
  uint16_t evaluate();
};

#endif
