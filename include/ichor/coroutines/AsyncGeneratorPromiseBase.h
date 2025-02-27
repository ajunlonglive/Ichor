///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

// Copied and modified from Lewis Baker's cppcoro

#pragma once

#include <coroutine>
#include <atomic>
#include <cassert>
#include <functional>
#include <ichor/Enums.h>

namespace Ichor {
    template<typename T>
    class AsyncGenerator;

    struct Empty;
}

namespace Ichor::Detail {
    thread_local extern DependencyManager *_local_dm;

    template<typename T>
    class AsyncGeneratorIterator;
    class AsyncGeneratorYieldOperation;
    class AsyncGeneratorAdvanceOperation;

    class AsyncGeneratorPromiseBase {
    public:
        AsyncGeneratorPromiseBase() noexcept
                : _state(state::value_ready_producer_suspended)
                , _exception(nullptr)
                , _id(_idCounter++)
        {
            // Other variables left intentionally uninitialised as they're
            // only referenced in certain states by which time they should
            // have been initialised.
            INTERNAL_DEBUG("Promise {}", _id);
        }
        virtual ~AsyncGeneratorPromiseBase() = default;

        AsyncGeneratorPromiseBase(const AsyncGeneratorPromiseBase& other) = delete;
        AsyncGeneratorPromiseBase& operator=(const AsyncGeneratorPromiseBase& other) = delete;

        std::suspend_always initial_suspend() const noexcept {
            return {};
        }

        AsyncGeneratorYieldOperation final_suspend() noexcept;

        void unhandled_exception() noexcept {
            // Don't bother capturing the exception if we have been cancelled
            // as there is no consumer that will see it.
            if (_state.load(std::memory_order_relaxed) != state::cancelled)
            {
                _exception = std::current_exception();
            }
        }

        /// Query if the generator has reached the end of the sequence.
        ///
        /// Only valid to call after resuming from an awaited advance operation.
        /// i.e. Either a begin() or iterator::operator++() operation.
        [[nodiscard]] virtual bool finished() const noexcept = 0;

        virtual void set_finished() noexcept = 0;

        void rethrow_if_unhandled_exception() {
            if (_exception)
            {
                std::rethrow_exception(std::move(_exception));
            }
        }

        /// Request that the generator cancel generation of new items.
        ///
        /// \return
        /// Returns true if the request was completed synchronously and the associated
        /// producer coroutine is now available to be destroyed. In which case the caller
        /// is expected to call destroy() on the coroutine_handle.
        /// Returns false if the producer coroutine was not at a suitable suspend-point.
        /// The coroutine will be destroyed when it next reaches a co_yield or co_return
        /// statement.
        bool request_cancellation() noexcept {
            INTERNAL_DEBUG("request_cancellation {}", _id);
            const auto previousState = _state.exchange(state::cancelled, std::memory_order_acq_rel);

            // Not valid to destroy async_generator<T> object if consumer coroutine still suspended
            // in a co_await for next item.
            assert(previousState != state::value_not_ready_consumer_suspended);

            // A coroutine should only ever be cancelled once, from the destructor of the
            // owning async_generator<T> object.
            assert(previousState != state::cancelled);

            return previousState == state::value_ready_producer_suspended;
        }

        [[nodiscard]] uint64_t get_id() const noexcept {
            return _id;
        }

    protected:
        AsyncGeneratorYieldOperation internal_yield_value() noexcept;

    public:
        friend class AsyncGeneratorYieldOperation;
        friend class AsyncGeneratorAdvanceOperation;

        std::atomic<state> _state;
        std::exception_ptr _exception;
        std::coroutine_handle<> _consumerCoroutine;
        uint64_t _id;
        static thread_local uint64_t _idCounter;
#ifdef ICHOR_USE_HARDENING
        DependencyManager *_dmAtTimeOfCreation{_local_dm};
#endif
    };


    class AsyncGeneratorYieldOperation final
    {
    public:

        AsyncGeneratorYieldOperation(AsyncGeneratorPromiseBase& promise, state initialState) noexcept
                : _promise(promise)
                , _initialState(initialState)
        {}

        bool await_ready() const noexcept {
            return _initialState == state::value_not_ready_consumer_suspended;
        }

        bool await_suspend(std::coroutine_handle<> producer) noexcept;

        void await_resume() noexcept {}

    private:
        AsyncGeneratorPromiseBase& _promise;
        state _initialState;
    };

    template<typename T>
    class AsyncGeneratorPromise final : public AsyncGeneratorPromiseBase
    {
        using value_type = std::remove_reference_t<T>;

    public:
        AsyncGeneratorPromise() noexcept : _destroyed(new bool(false)) {

        }
        ~AsyncGeneratorPromise() final {
            INTERNAL_DEBUG("destroyed promise {}", _id);
            *_destroyed = true;
        };

        AsyncGenerator<T> get_return_object() noexcept {
            return AsyncGenerator<T>{ *this };
        }

        AsyncGeneratorYieldOperation yield_value(value_type& value) noexcept(std::is_nothrow_constructible_v<T, T&&>);

        AsyncGeneratorYieldOperation yield_value(value_type&& value) noexcept(std::is_nothrow_constructible_v<T, T&&>);

        void return_value(value_type &value) noexcept(std::is_nothrow_constructible_v<T, T&&>);

        void return_value(value_type &&value) noexcept(std::is_nothrow_constructible_v<T, T&&>);

        T& value() noexcept {
            return _currentValue.value();
        }

        [[nodiscard]] bool finished() const noexcept final {
            return _finished;
        }

        std::shared_ptr<bool>& get_destroyed() noexcept  {
            return _destroyed;
        }

    private:
        void set_finished() noexcept final {
            INTERNAL_DEBUG("set_finished {}", _id);
            _finished = true;
        }

        std::optional<T> _currentValue;
        bool _finished{};
        std::shared_ptr<bool> _destroyed;
    };

    template<>
    class AsyncGeneratorPromise<void> final : public AsyncGeneratorPromiseBase
    {
    public:
        AsyncGeneratorPromise() noexcept : _destroyed(new bool(false)) {

        }
        ~AsyncGeneratorPromise() final {
            INTERNAL_DEBUG("destroyed promise {}", _id);
            *_destroyed = true;
        };

        AsyncGenerator<void> get_return_object() noexcept;

        AsyncGeneratorYieldOperation yield_value(Ichor::Empty) noexcept;

        void return_void() noexcept;

        [[nodiscard]] bool finished() const noexcept final {
            return _finished;
        }

        std::shared_ptr<bool>& get_destroyed() noexcept  {
            return _destroyed;
        }

    private:
        void set_finished() noexcept final {
            INTERNAL_DEBUG("set_finished {}", _id);
            _finished = true;
        }

        bool _finished{};
        std::shared_ptr<bool> _destroyed;
    };
}

#include <ichor/events/ContinuableEvent.h>
