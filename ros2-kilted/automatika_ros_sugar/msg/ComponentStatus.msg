uint8 STATUS_HEALTHY=0   # COMPONENT IS RUNNING AND HEALTHY

uint8 STATUS_FAILURE_ALGORITHM_LEVEL=1   # ERROR IN EXECUTING THE ALGORITHM/PLUGIN WITHIN THE COMPONENT

uint8 STATUS_FAILURE_COMPONENT_LEVEL=2      # ERROR OR FAILURE IN THE COMPONENT LEVEL

uint8 STATUS_FAILURE_SYSTEM_LEVEL=3     # ERROR IN COMPONENT EXECUTION DUE TO EXTERNAL FACTORS (SYSTEM LEVEL)

uint8 STATUS_GENERAL_FAILURE=4

uint8 status
string msg

string[] src_topics        # Registers names of the input topic provoking failure in case of system level error

string[] src_components    # Registers names of the component provoking failure in case of system level error and component level error (self)

string[] src_algorithms       # Registers names of the algorithm provoking failure in case of algorithmic level error
