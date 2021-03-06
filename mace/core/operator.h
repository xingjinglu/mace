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

#ifndef MACE_CORE_OPERATOR_H_
#define MACE_CORE_OPERATOR_H_

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "mace/core/arg_helper.h"
#include "mace/core/op_context.h"
#include "mace/core/registry.h"
#include "mace/core/tensor.h"
#include "mace/core/workspace.h"
#include "mace/proto/mace.pb.h"

namespace mace {

// memory_optimizer, device
class OpConstructContext {
 public:
  explicit OpConstructContext(Workspace *ws);
  OpConstructContext(OperatorDef *operator_def, Workspace *ws, Device *device);
  ~OpConstructContext() = default;

  inline void set_operator_def(OperatorDef *operator_def) {
    operator_def_ = operator_def;
  }

  inline OperatorDef *operator_def() const {
    return operator_def_;
  }

  inline Workspace *workspace() const {
    return ws_;
  }

  inline void set_device(Device* device) {
    device_ = device;
  }

  inline Device *device() const {
    return device_;
  }

 private:
  OperatorDef *operator_def_;
  Workspace *ws_;
  Device *device_;
};

// memory_optimizer, device
class OpInitContext {
 public:
  explicit OpInitContext(Workspace *ws, Device *device = nullptr);
  ~OpInitContext() = default;

  inline Workspace *workspace() const {
    return ws_;
  }

  inline void set_device(Device *device) {
    device_ = device;
  }

  inline Device *device() const {
    return device_;
  }

 private:
  Workspace *ws_;
  Device *device_;
};

class Operation {
 public:
  explicit Operation(OpConstructContext *context);
  virtual ~Operation() = default;

  template <typename T>
  inline T GetOptionalArg(const std::string &name,
                          const T &default_value) const {
    MACE_CHECK(operator_def_, "operator_def was null!");
    return ProtoArgHelper::GetOptionalArg<OperatorDef, T>(
        *operator_def_, name, default_value);
  }
  template <typename T>
  inline std::vector<T> GetRepeatedArgs(
      const std::string &name, const std::vector<T> &default_value = {}) const {
    MACE_CHECK(operator_def_, "operator_def was null!");
    return ProtoArgHelper::GetRepeatedArgs<OperatorDef, T>(
        *operator_def_, name, default_value);
  }

  inline DeviceType device_type() const {
    return static_cast<DeviceType>(operator_def_->device_type());
  }

  inline const Tensor *Input(unsigned int idx) {
    MACE_CHECK(idx < inputs_.size());
    return inputs_[idx];
  }

  inline Tensor *Output(int idx) { return outputs_[idx]; }

  inline int InputSize() { return inputs_.size(); }
  inline int OutputSize() { return outputs_.size(); }
  inline const std::vector<const Tensor *> &Inputs() const { return inputs_; }
  inline const std::vector<Tensor *> &Outputs() { return outputs_; }

  // Run Op asynchronously (depends on device), return a future if not nullptr.
  virtual MaceStatus Init(OpInitContext *);
  virtual MaceStatus Run(OpContext *) = 0;

  inline const OperatorDef &debug_def() const {
    MACE_CHECK(has_debug_def(), "operator_def was null!");
    return *operator_def_;
  }

  inline void set_debug_def(
      const std::shared_ptr<const OperatorDef> &operator_def) {
    operator_def_ = operator_def;
  }

  inline bool has_debug_def() const { return operator_def_ != nullptr; }

 protected:
  std::shared_ptr<const OperatorDef> operator_def_;
  std::vector<const Tensor *> inputs_;
  std::vector<Tensor *> outputs_;

  MACE_DISABLE_COPY_AND_ASSIGN(Operation);
};

// MACE_OP_INPUT_TAGS and MACE_OP_OUTPUT_TAGS are optional features to name the
// indices of the operator's inputs and outputs, in order to avoid confusion.
// For example, for a fully convolution layer that has input, weight and bias,
// you can define its input tags as:
//     MACE_OP_INPUT_TAGS(INPUT, WEIGHT, BIAS);
// And in the code, instead of doing
//     auto& weight = Input(1);
// you can now do
//     auto& weight = Input(WEIGHT);
// to make it more clear.
#define MACE_OP_INPUT_TAGS(first_input, ...) \
  enum _InputTags { first_input = 0, __VA_ARGS__ }
#define MACE_OP_OUTPUT_TAGS(first_input, ...) \
  enum _OutputTags { first_input = 0, __VA_ARGS__ }

class OpKeyBuilder {
 public:
  explicit OpKeyBuilder(const char *op_name);

  OpKeyBuilder &Device(DeviceType device);

  OpKeyBuilder &TypeConstraint(const char *attr_name,
                               DataType allowed);

  template <typename T>
  OpKeyBuilder &TypeConstraint(const char *attr_name);

  const std::string Build();

 private:
  std::string op_name_;
  DeviceType device_type_;
  std::map<std::string, DataType> type_constraint_;
};

template <typename T>
OpKeyBuilder &OpKeyBuilder::TypeConstraint(const char *attr_name) {
  return this->TypeConstraint(attr_name, DataTypeToEnum<T>::value);
}

class OpRegistryBase {
 public:
  typedef Registry<std::string,
                   Operation,
                   OpConstructContext *>
      RegistryType;
  OpRegistryBase() = default;
  virtual ~OpRegistryBase();
  RegistryType *registry() { return &registry_; }
  std::unique_ptr<Operation> CreateOperation(
      OpConstructContext *context,
      DeviceType device_type,
      const NetMode mode) const;

 private:
  RegistryType registry_;
  MACE_DISABLE_COPY_AND_ASSIGN(OpRegistryBase);
};

MACE_DECLARE_REGISTRY(OpRegistry,
                      Operation,
                      OpConstructContext *);

#define MACE_REGISTER_OP(op_registry, op_type, class_name, device, dt) \
  MACE_REGISTER_CLASS(OpRegistry,                                      \
                      op_registry->registry(),                         \
                      OpKeyBuilder(op_type)                            \
                        .Device(device)                                \
                        .TypeConstraint<dt>("T")                       \
                        .Build(),                                      \
                      class_name<device, dt>)

}  // namespace mace

#endif  // MACE_CORE_OPERATOR_H_
