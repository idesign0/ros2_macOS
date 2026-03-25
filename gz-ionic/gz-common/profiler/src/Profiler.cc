/*
 * Copyright (C) 2018 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "gz/common/Profiler.hh" // NOLINT(*)
#include "gz/common/ProfilerImpl.hh"
#include "gz/common/Console.hh"

#if GZ_PROFILER_REMOTERY
#include "RemoteryProfilerImpl.hh"
#endif  // GZ_PROFILER_REMOTERY

using namespace gz;
using namespace common;

//////////////////////////////////////////////////
Profiler::Profiler():
  impl(nullptr)
{
#if GZ_PROFILER_REMOTERY
  impl = new RemoteryProfilerImpl();
#endif  // GZ_PROFILER_REMOTERY

  if (this->impl == nullptr)
  {
    gzwarn << "No profiler implementation detected, profiling is disabled"
            << std::endl;
  }
  else
  {
    gzdbg << "Gazebo profiling with: " << impl->Name() << std::endl;
  }
}

//////////////////////////////////////////////////
Profiler::~Profiler()
{
  if (this->impl)
    delete this->impl;
  this->impl = nullptr;
}

//////////////////////////////////////////////////
void Profiler::SetThreadName(const char * _name)
{
  if (this->impl)
    this->impl->SetThreadName(_name);
}

//////////////////////////////////////////////////
void Profiler::LogText(const char * _text)
{
  if (this->impl)
    this->impl->LogText(_text);
}

//////////////////////////////////////////////////
void Profiler::BeginSample(const char * _name, uint32_t* _hash)
{
  if (this->impl)
    this->impl->BeginSample(_name, _hash);
}

//////////////////////////////////////////////////
void Profiler::EndSample()
{
  if (this->impl)
    this->impl->EndSample();
}

//////////////////////////////////////////////////
std::string Profiler::ImplementationName() const
{
  if (this->impl)
    return this->impl->Name();
  else
    return "disabled";
}


//////////////////////////////////////////////////
bool Profiler::Valid() const
{
  return this->impl != nullptr;
}

//////////////////////////////////////////////////
bool Profiler::SetImplementation(std::unique_ptr<ProfilerImpl> _impl)
{
  if (_impl == nullptr)
  {
    gzwarn << "Setting an empty profiler implementation is not supported"
           << std::endl;
    return false;
  }
  if (this->impl != nullptr)
  {
    gzwarn << "A profiler implementation named '" << this->impl->Name()
           << "' is already in use. Cannot set a new one." << std::endl;
    return false;
  }
  this->impl = _impl.release();
  gzdbg << "Gazebo profiling with: " << this->impl->Name() << std::endl;
  return true;
}
