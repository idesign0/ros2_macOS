#!/usr/bin/ruby

# Copyright (C) 2018 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

require 'open3'

testfile = 'gz_log_record_force.tlog'
File.write(testfile, 'not empty file')

_, stdout, stderr, wait_thr =
  Open3.popen3("gz log record --force --file #{testfile}")

sleep(2)

cmd_running = true
if wait_thr.alive?
  Process.kill('KILL', wait_thr.pid)
else
  cmd_running = false
end

stdout = stdout.read
stderr = stderr.read

puts '-----stdout-----'
puts stdout
puts '-----stderr-----'
puts stderr
puts '-----status-----'
puts wait_thr.value

File.delete(testfile)

exit 1 unless cmd_running
exit 2 unless stderr.empty?
exit 0
