#include <gz/common.hh>

int main(int argc, char **argv)
{
  // Example of using assert with a message
  GZ_ASSERT(1 == 2, "1 must equal 2");
  return 0;
}
