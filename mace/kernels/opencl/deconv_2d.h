// Copyright 2018 Xiaomi, Inc.  All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MACE_KERNELS_OPENCL_DECONV_2D_H_
#define MACE_KERNELS_OPENCL_DECONV_2D_H_

#include <vector>

#include "mace/kernels/activation.h"

namespace mace {

class OpContext;
class Tensor;

namespace kernels {
class OpenCLDeconv2dKernel {
 public:
  virtual MaceStatus Compute(
      OpContext *context,
      const Tensor *input,
      const Tensor *filter,
      const Tensor *bias,
      const int *strides,
      const int *padding_data,
      const ActivationType activation,
      const float relux_max_limit,
      const std::vector<index_t> &output_shape,
      Tensor *output) = 0;
  MACE_EMPTY_VIRTUAL_DESTRUCTOR(OpenCLDeconv2dKernel);
};
}  // namespace kernels
}  // namespace mace

#endif  // MACE_KERNELS_OPENCL_DECONV_2D_H_
