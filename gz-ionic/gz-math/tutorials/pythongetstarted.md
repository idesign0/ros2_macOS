\page pythongetstarted Python Get Started

Previous Tutorial: \ref cppgetstarted

## Overview

This tutorial describes how to get started using Gazebo Math with Python.

**NOTE**: If you have compiled Gazebo Math from source, you should export
your `PYTHONPATH`.

```bash
export PYTHONPATH=$PYTHONPATH:<path to your workspace>/install/lib/python
```

We will run through an example that determines the distance between two
points in 3D space. Start by creating a bare-bones main file using the
editor of your choice.

```python
def main():
  pass

if __name__ == "__main__":
  main()
```

The easiest way to include Gazebo Math is through `import gz.math8`.

At this point your main file should look like

```python
import gz.math8

def main():
  pass

if __name__ == "__main__":
  main()
```

Now let's create two 3D points with arbitrary values. We will use the
`gz.math.Vector3` class to represent these points. Gazebo Math provides
some `Vector3` types which are: `Vector3d` (Vector3 using doubles), `Vector3f` (Vector3 using floats)
and `Vector3i` (Vector3 using integers). The result of this addition will be a
main file similar to the following.

```python
from gz.math8 import Vector3d

def main():
  point1 = Vector3d(1, 3, 5)
  point2 = Vector3d(2, 4, 6)

if __name__ == "__main__":
  main()
```

Finally, we can compute the distance between `point1` and `point2` using the
`gz.math.Vector3.distance()` function and output the distance value.

```python
from gz.math8 import Vector3d

def main():
  point1 = Vector3d(1, 3, 5)
  point2 = Vector3d(2, 4, 6)

  distance = point1.distance(point2);

  print("Distance from {} to {} is {}".format(point1, point2, distance))

if __name__ == "__main__":
  main()
```

Running this program should result in the following output:

```{.bash}
Distance from 1 3 5 to 2 4 6 is 1.7320508075688772
```
