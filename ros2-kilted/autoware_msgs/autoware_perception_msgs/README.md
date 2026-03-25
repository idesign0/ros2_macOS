# autoware_perception_msgs

## TrafficSignalElement.msg

This is the element of traffic signals such as red, amber, green, and turn arrow.
The elements are based on Vienna Convention on Road Signs and Signals.

## TrafficSignal.msg

For each direction of an intersection, there is one state of traffic signal regardless of the number of equipment.
This message represents the traffic signal as a concept and is used by components such as planning.

## TrafficSignalArray.msg

This is a plural type of TrafficSignal.msg.
