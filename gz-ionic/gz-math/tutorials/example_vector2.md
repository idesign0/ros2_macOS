\page example_vector2 Vector example

This tutorial explains how to use the `Vector2` class from Gazebo Math library.

## C++ example

### Compile the code

To compile the code, go to `gz-math/examples` and use `cmake`:

```bash
git clone https://github.com/gazebosim/gz-math/ -b gz-math8
cd gz-math/examples
mkdir build
cd build
cmake ..
# Linux and macOS
make
# Windows
cd ..
cmake --build build --config Release
```

When the code is compiled, run:

```bash
# Linux and macOS
./vector2_example
# Windows
.\build\Release\vector2_example.exe
```

The output of the program:

```bash
Vec2: 2 4
Vec2a: 1 2
Vec2b: 1.2 3.4
Vec2: x=2 y=4
Vec2a: x=1 y=2
Vec2b: x=1.2 y=3.4
4
2 8
3 6
1 2
2 2
2.23607
```

### Code

Create a `Vector2` called `vec2` of doubles using the typedef `Vector2d`. **The initial x and y values are zero**. The x and y components of `vec2` can be set at anytime.

\snippet examples/vector2_example.cc constructor


The `Vector2` class is a template, so you can also create a `Vector2` using `gz::math::Vector2<double>`:

\snippet examples/vector2_example.cc constructor2

It's also possible to set initial values. Here we are using a `Vector2` of floats:

\snippet examples/vector2_example.cc constructor3

We can output the contents of each vector using `std::cout`.

\snippet examples/vector2_example.cc stdout

You can also get access to each component in the vector using the `X()`, `Y()` accessors or the `[]` operator, The operator is clamped to the range `[0, 1]`.

\snippet examples/vector2_example.cc access

The `Vector2` class overloads many common operators, such as:

\snippet examples/vector2_example.cc operators

There are also many useful functions, such as finding the distance between two vectors.

\snippet examples/vector2_example.cc distance

**There are more functions in Vector2. Take a look at the [API](https://gazebosim.org/libs/math)**

## Ruby examples

This example will only work if the Ruby interface library was compiled and installed. For example,
on Ubuntu:

```bash
sudo apt install ruby-gz-math<#>
```

Be sure to replace <#> with a number value, such as 8 or 7, depending on which version you need.<#>.

Modify the `RUBYLIB` environment variable to include the Gazebo Math library install path. For example, if you install to `/usr`:

```bash
export RUBYLIB=/usr/lib/ruby:$RUBYLIB
```

Move to the examples folder:

```bash
cd examples
```

Execute the example:

```bash
ruby vector2_example.rb
```

The output of the program:

```bash
va = 1.000000 2.000000
vb = 3.000000 4.000000
vc = 3.000000 4.000000
vb += va: 4.000000 6.000000
vb.Normalize = 0.554700 0.832050
vb.Distance(va) = 1.249959
```

Execute the example:

```bash
ruby vector3_example.rb
```

The output of the program:

```bash
v1 = 0.000000 0.000000 0.000000
v2 = 1.000000 0.000000 0.000000
v1 + v2 = 1.000000 0.000000 0.000000
v1.Distance(v2) = 1.000000
```

### Code

Create a `Vector2` of doubles using the typedef `Vector2d`. It's possible to set initial values or use another object to create an identical copy.

```bash
va = Gz::Math::Vector2d.new(1, 2)
```

You can get access to each component in the vector using the `X()`, `Y()` accessors.

```bash
printf("va = %f %f\n", va.X(), va.Y())
printf("vb = %f %f\n", vb.X(), vb.Y())
printf("vc = %f %f\n", vc.X(), vc.Y())
```

The `Vector2` class overloads many common operators, such as:

```bash
vb += va
printf("vb += va: %f %f\n", vb.X(), vb.Y())
```

There are also many useful functions, such as finding the distance between two vectors or normalizing a vector.

```bash
vb.Normalize
printf("vb.Normalize = %f %f\n", vb.X(), vb.Y())
printf("vb.Distance(va) = %f\n", vb.Distance(va))
```

You can create vectors with 3 dimensions using the typedef `Vector3d`:

```bash
v1 = Gz::Math::Vector3d.new(0, 0, 0)
```

You can also get access to each component in the vector using the `X()`, `Y()` and `Z()` accessors:

```bash
printf("v1 =: %f %f %f\n", v1.X(), v1.Y(), v1.Z())
```
