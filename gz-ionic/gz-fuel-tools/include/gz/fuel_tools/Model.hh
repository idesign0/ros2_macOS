/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#ifndef GZ_FUEL_TOOLS_MODEL_HH_
#define GZ_FUEL_TOOLS_MODEL_HH_

#include <memory>
#include <string>

#include "gz/fuel_tools/Helpers.hh"
#include "gz/fuel_tools/Result.hh"
#include "gz/fuel_tools/ModelIdentifier.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::shared_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief Forward Declaration
  class ModelPrivate;

  /// \brief Forward declaration
  class ModelIter;

  /// \brief Forward declaration
  class ModelIterPrivate;

  /// \brief forward declaration
  class LocalCache;

  /// \brief forward declaration
  class LocalCachePrivate;

  /// \brief Forward declaration
  class IterIds;

  /// \brief Forward declaration
  class IterRESTIds;

  /// \brief Forward declaration
  class IterRestIds;

  /// \brief Forward declaration
  class ModelIterTest;

  /// \brief Defines how to identify a model.
  class GZ_FUEL_TOOLS_VISIBLE Model
  {
    friend IterIds;
    friend IterRESTIds;
    friend IterRestIds;
    friend ModelIter;
    friend ModelIterPrivate;
    friend ModelIterTest;
    friend LocalCache;
    friend LocalCachePrivate;

    /// \brief Default constructor
    public: Model();

    /// \brief Copy constructor
    /// \param[in] _orig Model to copy.
    public: Model(const Model &_orig) = default;

    /// \brief Protected constructor
    /// \param[in] _dptr Model private data to copy.
    protected: explicit Model(std::shared_ptr<ModelPrivate> _dptr);

    /// \brief Returns false if model was constructed via Model()
    public: operator bool() const;

    /// \brief Returns false if model was constructed via Model().
    /// \return False if model was constructed via Model().
    public: operator bool();

    /// \brief Returns information identifying the model.
    /// \return Information that can identify the model.
    public: ModelIdentifier Identification() const;

    /// \brief Make sure this model is in the local cache
    /// \remarks this downloads the model and saves it locally if necessary
    /// \returns The result of fetching
    public: Result Fetch() const;

    /// \brief Returns a path to the model on disk if it is already
    /// cached.
    /// \returns path, or empty string if the model is not cached.
    public: std::string PathToModel() const;

    /// \brief PIMPL
    private: std::shared_ptr<ModelPrivate> dataPtr;
  };
}  // namespace gz::fuel_tools

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif  // GZ_FUEL_TOOLS_MODEL_HH_
