#ifndef EVALUATOR_H
#define EVALUATOR_H

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* April 2022                                           */
/********************************************************/
/* Plan heuristic evaluator                             */
/********************************************************/

/********************************************************/
/* CLASS: Evaluator                                     */
/********************************************************/

#include "../planner/linearizer.h"
#include "../planner/plan.h"
#include "../planner/planComponents.h"
#include "../planner/state.h"
#include "../sas/sasTask.h"
#include "../utils/utils.h"
#include "hLand.h"
#include <deque>

// Entry of a priority queue to sort the plan timepoints
class ScheduledPoint : public PriorityQueueItem {
public:
  TTimePoint p;
  float time;
  Plan *plan;

  ScheduledPoint(TTimePoint tp, float t, Plan *pl) {
    p = tp;
    time = t;
    plan = pl;
  }
  virtual inline int compare(PriorityQueueItem *other) {
    double otherTime = ((ScheduledPoint *)other)->time;
    if (time < otherTime)
      return -1;
    else if (time > otherTime)
      return 1;
    else
      return 0;
  }
};

// Heuristic evaluator
class Evaluator {
private:
  SASTask *task;
  std::vector<SASAction *> *tilActions;
  PlanComponents planComponents;
  PriorityQueue pq;
  // bool* usefulActions;
  LandmarkHeuristic *landmarks;
  std::vector<LandmarkCheck *> openNodes; // For hLand calculation
  bool numericConditionsOrConditionalEffects;

  void calculateFrontierState(TState *fs, Plan *currentPlan);
  bool findOpenNode(LandmarkCheck *l);

public:
  Evaluator();
  ~Evaluator();
  void initialize(TState *state, SASTask *task, std::vector<SASAction *> *a,
                  bool forceAtEndConditions);
  void calculateFrontierState(Plan *p);
  void evaluate(Plan *p);
  void evaluateInitialPlan(Plan *p);
  std::vector<SASAction *> *getTILActions() { return tilActions; }
  bool informativeLandmarks();
};

#endif
