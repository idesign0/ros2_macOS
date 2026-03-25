# aws_sdk_cpp_vendor
vendor package for [AWS SDK for C++](https://github.com/aws/aws-sdk-cpp)

## Usage

```
# package.xml
<depend>aws_sdk_cpp_vendor<depend>

# CMakeLists.txt
find_package(aws_sdk_cpp_vendor REQUIRED)
find_package(AWSSDK REQUIRED COMPONENTS core)

target_link_libraries(your_target
  aws-cpp-sdk-core
)
```
