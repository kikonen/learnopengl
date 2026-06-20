#pragma once

#include <memory>
#include <utility>
#include <type_traits>

namespace event {
    // Minimal move-only, type-erased void() callable.
    //
    // std::function cannot hold a move-only lambda (e.g. one that move-captures a
    // std::unique_ptr) because it requires the target to be copy-constructible.
    // Deferred work commonly transfers ownership of a resource into the task, so the
    // callable must be move-only. C++23's std::move_only_function would replace this
    // verbatim, but the project targets C++20.
    class Task {
        struct Base {
            virtual ~Base() = default;
            virtual void call() = 0;
        };

        template<typename F>
        struct Impl final : Base {
            F m_fn;
            explicit Impl(F fn) : m_fn(std::move(fn)) {}
            void call() override { m_fn(); }
        };

        std::unique_ptr<Base> m_impl;

    public:
        Task() noexcept = default;

        template<
            typename F,
            typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, Task>>>
        Task(F&& fn)
            : m_impl(std::make_unique<Impl<std::decay_t<F>>>(std::forward<F>(fn)))
        {}

        Task(Task&&) noexcept = default;
        Task& operator=(Task&&) noexcept = default;

        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        explicit operator bool() const noexcept { return static_cast<bool>(m_impl); }

        void operator()() const { m_impl->call(); }
    };
}
