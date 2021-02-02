// This file is part of the Acts project.
//
// Copyright (C) 2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <vector>
#include <memory>

namespace cub {
  class CachingDeviceAllocator;
}

namespace Acts::Cuda {
  class NaiveMemoryManager {
  public:
    NaiveMemoryManager(const NaiveMemoryManager&) = delete;
    NaiveMemoryManager(NaiveMemoryManager&&) = delete;

    NaiveMemoryManager& operator=(const NaiveMemoryManager&) = delete;
    NaiveMemoryManager& operator=(NaiveMemoryManager&&) = delete;

    static NaiveMemoryManager& instance();

    void setMemorySize(std::size_t sizeInBytes, int device = -1);

    std::size_t availableMemory(int device = -1) const;

    void* allocate(std::size_t sizeInBytes, int device = -1);

    void reset(int device = -1);
  private:
    NaiveMemoryManager() = default;

    cub::CachingDeviceAllocator * allocator;

    std::vector<void *> allocations;

    size_t max_capacity = 1024L * 1024L * 1024L;
    size_t total_size = 0;
  };
}