//==-------------------- queue.hpp - SYCL queue ----------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <CL/sycl/detail/common.hpp>
#include <CL/sycl/detail/queue_impl.hpp>
#include <CL/sycl/device_selector.hpp>
#include <CL/sycl/exception_list.hpp>
#include <CL/sycl/info/info_desc.hpp>
#include <CL/sycl/property_list.hpp>

#include <memory>
#include <utility>

__SYCL_INLINE namespace cl {
namespace sycl {

// Forward declaration
class context;
class device;
class queue {
public:
  explicit queue(const property_list &propList = {})
      : queue(default_selector(), async_handler{}, propList) {}

  queue(const async_handler &asyncHandler, const property_list &propList = {})
      : queue(default_selector(), asyncHandler, propList) {}

  queue(const device_selector &deviceSelector,
        const property_list &propList = {})
      : queue(deviceSelector.select_device(), async_handler{}, propList) {}

  queue(const device_selector &deviceSelector,
        const async_handler &asyncHandler, const property_list &propList = {})
      : queue(deviceSelector.select_device(), asyncHandler, propList) {}

  queue(const device &syclDevice, const property_list &propList = {})
      : queue(syclDevice, async_handler{}, propList) {}

  queue(const device &syclDevice, const async_handler &asyncHandler,
        const property_list &propList = {});

  queue(const context &syclContext, const device_selector &deviceSelector,
        const property_list &propList = {})
      : queue(syclContext, deviceSelector,
              detail::getSyclObjImpl(syclContext)->get_async_handler(),
              propList) {}

  queue(const context &syclContext, const device_selector &deviceSelector,
        const async_handler &asyncHandler, const property_list &propList = {});

  queue(cl_command_queue clQueue, const context &syclContext,
        const async_handler &asyncHandler = {});

  queue(const queue &rhs) = default;

  queue(queue &&rhs) = default;

  queue &operator=(const queue &rhs) = default;

  queue &operator=(queue &&rhs) = default;

  bool operator==(const queue &rhs) const { return impl == rhs.impl; }

  bool operator!=(const queue &rhs) const { return !(*this == rhs); }

  cl_command_queue get() const { return impl->get(); }

  context get_context() const { return impl->get_context(); }

  device get_device() const { return impl->get_device(); }

  bool is_host() const { return impl->is_host(); }

  template <info::queue param>
  typename info::param_traits<info::queue, param>::return_type
  get_info() const {
    return impl->get_info<param>();
  }

  template <typename T> event submit(T cgf) { return impl->submit(cgf, impl); }

  template <typename T> event submit(T cgf, queue &secondaryQueue) {
    return impl->submit(cgf, impl, secondaryQueue.impl);
  }

  void wait() { impl->wait(); }

  void wait_and_throw() { impl->wait_and_throw(); }

  void throw_asynchronous() { impl->throw_asynchronous(); }

  template <typename propertyT> bool has_property() const {
    return impl->has_property<propertyT>();
  }

  template <typename propertyT> propertyT get_property() const {
    return impl->get_property<propertyT>();
  }

  event memset(void* Ptr, int Value, size_t Count) {
    return impl->memset(impl, Ptr, Value, Count);
  }

  event memcpy(void* Dest, const void* Src, size_t Count) {
    return impl->memcpy(impl, Dest, Src, Count);
  }

  event mem_advise(const void *Ptr, size_t Length, int Advice) {
    return impl->mem_advise(Ptr, Length, Advice);
  }

  event prefetch(const void* Ptr, size_t Count) {
    return submit([=](handler &CGH) {
        CGH.prefetch(Ptr, Count);
    });
  }

  /// single_task version with a kernel represented as a lambda.
  ///
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType>
  event single_task(KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.template single_task<KernelName, KernelType>(KernelFunc);
    });
  }

  /// single_task version with a kernel represented as a lambda.
  ///
  /// @param DepEvent is an event that specifies the kernel dependences 
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType>
  event single_task(event DepEvent, KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.depends_on(DepEvent);
      CGH.template single_task<KernelName, KernelType>(KernelFunc);
    });
  }

  /// single_task version with a kernel represented as a lambda.
  ///
  /// @param DepEvents is a vector of events that specify the kernel dependences
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType>
  event single_task(const vector_class<event> &DepEvents,
                    KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.depends_on(DepEvents);
      CGH.template single_task<KernelName, KernelType>(KernelFunc);
    });
  }

  /// parallel_for version with a kernel represented as a lambda + range that
  /// specifies global size only.
  ///
  /// @param NumWorkItems is a range that specifies the work space of the kernel
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType,
            int Dims>
  event parallel_for(range<Dims> NumWorkItems, KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.template parallel_for<KernelName, KernelType, Dims>(NumWorkItems,
                                                              KernelFunc);
    });
  }

  /// parallel_for version with a kernel represented as a lambda + range that
  /// specifies global size only.  
  ///
  /// @param NumWorkItems is a range that specifies the work space of the kernel
  /// @param DepEvent is an event that specifies the kernel dependences 
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType,
            int Dims>
  event parallel_for(range<Dims> NumWorkItems, event DepEvent,
                     KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.depends_on(DepEvent);
      CGH.template parallel_for<KernelName, KernelType, Dims>(NumWorkItems,
                                                              KernelFunc);
    });
  }

  /// parallel_for version with a kernel represented as a lambda + range that
  /// specifies global size only.
  ///
  /// @param NumWorkItems is a range that specifies the work space of the kernel
  /// @param DepEvents is a vector of events that specifies the kernel dependences 
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType,
            int Dims>
  event parallel_for(range<Dims> NumWorkItems,
                     const vector_class<event> &DepEvents,
                     KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.depends_on(DepEvents);
      CGH.template parallel_for<KernelName, KernelType, Dims>(NumWorkItems,
                                                              KernelFunc);
    });
  }

  /// parallel_for version with a kernel represented as a lambda + range and
  /// offset that specify global size and global offset correspondingly.
  ///
  /// @param NumWorkItems is a range that specifies the work space of the kernel
  /// @param WorkItemOffset specifies the offset for each work item id
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType,
            int Dims>
  event parallel_for(range<Dims> NumWorkItems, id<Dims> WorkItemOffset,
                    KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.template parallel_for<KernelName, KernelType, Dims>(
          NumWorkItems, WorkItemOffset, KernelFunc);
    });
  }

  /// parallel_for version with a kernel represented as a lambda + range and
  /// offset that specify global size and global offset correspondingly.
  ///
  /// @param NumWorkItems is a range that specifies the work space of the kernel
  /// @param WorkItemOffset specifies the offset for each work item id
  /// @param DepEvent is an event that specifies the kernel dependences 
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType,
            int Dims>
  event parallel_for(range<Dims> NumWorkItems, id<Dims> WorkItemOffset,
                     event DepEvent, KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.depends_on(DepEvent);
      CGH.template parallel_for<KernelName, KernelType, Dims>(
          NumWorkItems, WorkItemOffset, KernelFunc);
    });
  }

  /// parallel_for version with a kernel represented as a lambda + range and
  /// offset that specify global size and global offset correspondingly.
  ///
  /// @param NumWorkItems is a range that specifies the work space of the kernel
  /// @param WorkItemOffset specifies the offset for each work item id
  /// @param DepEvents is a vector of events that specifies the kernel dependences 
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType,
            int Dims>
  event parallel_for(range<Dims> NumWorkItems, id<Dims> WorkItemOffset,
                     const vector_class<event> &DepEvents,
                     KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.depends_on(DepEvents);
      CGH.template parallel_for<KernelName, KernelType, Dims>(
          NumWorkItems, WorkItemOffset, KernelFunc);
    });
  }

  /// parallel_for version with a kernel represented as a lambda + nd_range that
  /// specifies global, local sizes and offset.
  ///
  /// @param ExecutionRange is a range that specifies the work space of the kernel
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType,
            int Dims>
  event parallel_for(nd_range<Dims> ExecutionRange, KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.template parallel_for<KernelName, KernelType, Dims>(ExecutionRange,
                                                              KernelFunc);
    });
  }

  /// parallel_for version with a kernel represented as a lambda + nd_range that
  /// specifies global, local sizes and offset.
  ///
  /// @param ExecutionRange is a range that specifies the work space of the kernel
  /// @param DepEvent is an event that specifies the kernel dependences 
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType,
            int Dims>
  event parallel_for(nd_range<Dims> ExecutionRange,
                     event DepEvent, KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.depends_on(DepEvent);
      CGH.template parallel_for<KernelName, KernelType, Dims>(ExecutionRange,
                                                              KernelFunc);
    });
  }

  /// parallel_for version with a kernel represented as a lambda + nd_range that
  /// specifies global, local sizes and offset.
  ///
  /// @param ExecutionRange is a range that specifies the work space of the kernel
  /// @param DepEvents is a vector of events that specifies the kernel dependences 
  /// @param KernelFunc is the Kernel functor or lambda
  template <typename KernelName = detail::auto_name, typename KernelType,
            int Dims>
  event parallel_for(nd_range<Dims> ExecutionRange,
                     const vector_class<event> &DepEvents,
                     KernelType KernelFunc) {
    return submit([&](handler &CGH) {
      CGH.depends_on(DepEvents);
      CGH.template parallel_for<KernelName, KernelType, Dims>(ExecutionRange,
                                                              KernelFunc);
    });
  }

private:
  std::shared_ptr<detail::queue_impl> impl;
  template <class Obj>
  friend decltype(Obj::impl) detail::getSyclObjImpl(const Obj &SyclObject);
};

} // namespace sycl
} // namespace cl

namespace std {
template <> struct hash<cl::sycl::queue> {
  size_t operator()(const cl::sycl::queue &q) const {
    return std::hash<std::shared_ptr<cl::sycl::detail::queue_impl>>()(
        cl::sycl::detail::getSyclObjImpl(q));
  }
};
} // namespace std
