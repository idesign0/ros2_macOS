#include <gz/msgs/Factory.hh>

#include <google/protobuf/text_format.h>

// A simple example that demonstrates the use of the message factory
//
// Usage:
// With no arguments, it will list all known messages
//   ./using_gz_msgs
// With an argument, it will display that message, if found
//   ./using_gz_msgs gz.msgs.Vector3d
int main(int argc, char** argv)
{
  if (argc == 1)
  {
    std::vector<std::string> known_types;
    gz::msgs::Factory::Types(known_types);
    std::cout << "Known types: " << std::endl;
    for (const auto &t: known_types)
    {
      std::cout << t << std::endl;
    }
  }
  else
  {
    gz::msgs::Factory::MessagePtr msg = gz::msgs::Factory::New(argv[1]);
    if (msg)
    {
      auto descriptor = msg->GetDescriptor();
      auto fileDescriptor = descriptor->file();
      std::cout << "Name: " << descriptor->full_name() << std::endl;
      std::cout << "File: " << fileDescriptor->name() << std::endl << std::endl;
      std::cout << descriptor->DebugString() << std::endl;
    }
    else
    {
      std::cout << "Couldn't find msg: " << argv[1] << std::endl;;
    }
  }
}
