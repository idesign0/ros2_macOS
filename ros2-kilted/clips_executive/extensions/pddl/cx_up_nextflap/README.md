# cx_up_nextflap

This wraps the temporal and numeric planner NEXTFLAP and it's integration into the unified planning framework for easy deployment in ROS ([source](https://github.com/aiplan4eu/up-nextflap)).

NEXTFLAP utilizes SMT solving via th z3 theorem prover. This wrapper is accompanied by the `cx_z3_vendor` package, which bundles a version that should work well with NEXTFLAP.

The both the original project and this one are licensed under Apache-2.0 license.

## Original Description

This is the NEXTFLAP UP integrator. NEXTFLAP is an expressive temporal and numeric planner supporting planning problems involving Boolean and numeric state variables, instantaneous and durative actions. A distinctive feature of NEXTFLAP is the ability to reason over problems with a prevalent numeric structure, which may involve linear and non-linear numeric conditions and effects. NEXTFLAP favors the capability of modeling expressive problems; indeed it handles negative and disjunctive preconditions as well as existential and universal expressions. NEXTFLAP can be used as a satisficing planner and as a partial-order plan validator. NEXTFLAP is written completely in C++.
NEXTFLAP is a numeric extension of [TFLAP planner](https://grps.webs.upv.es/downloadPaper.php?paperId=238), and uses the Z3 Theorem Prover to check the numeric constraints and ensure consistency of plans.

### Planning approaches of UP supported
Classical, Numeric and Temporal Planning

Partial-Order Plan Validator

### Operative modes of UP currently supported
One shot planning
Plan Validator
