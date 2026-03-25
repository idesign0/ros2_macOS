#include <gz/custom_msgs/foo.pb.h>
#include <gz/custom_msgs/bar.pb.h>
#include <gz/custom_msgs/baz.pb.h>

#include <google/protobuf/text_format.h>

// A simple example that demonstrates the use of the message factory
//
//   ./generating_custom_messages
int main()
{
  gz::custom_msgs::BazStamped msg;

  // msg has header and baz field
  auto *header = msg.mutable_header();
  auto *baz = msg.mutable_baz();

  {
    // Populate the header with something
    header->mutable_stamp()->set_sec(100);
    header->mutable_stamp()->set_nsec(100);
  }

  {
    // Add a frame_id to the header
    auto map_entry = header->add_data();
    map_entry->set_key("frame_id");
    map_entry->add_value("gz_custom_msgs");
  }

  {
    // Add an arbitrary array value to the header
    auto map_entry = header->add_data();
    map_entry->set_key("array");
    map_entry->add_value("a");
    map_entry->add_value("b");
    map_entry->add_value("c");
  }

  // baz has foo and bar field;
  auto *foo = baz->mutable_foo();
  auto *bar = baz->mutable_bar();

  {
    // Set the values of our custom sub-messages
    foo->set_value(1.0);
    bar->set_value(1.0);
  }

  {
    // Print the text descriptor of a message
    auto descriptor = msg.GetDescriptor();
    auto fileDescriptor = descriptor->file();
    std::cout << "Message definition: " << std::endl;
    std::cout << "Name: " << descriptor->full_name() << std::endl;
    std::cout << "File: " << fileDescriptor->name() << std::endl << std::endl;
    std::cout << descriptor->DebugString() << std::endl;
  }

  {
    // Print the populated values of a message
    std::cout << "===============================" << std::endl;
    std::cout << "Populated Message: \n" << msg.DebugString() << std::endl;
  }
}
