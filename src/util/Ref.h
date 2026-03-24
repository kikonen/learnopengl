#pragma once

#include <atomic>
#include <cassert>

namespace util
{
    /////////////////////////////////////////////////////////
    // RefCounted
    /////////////////////////////////////////////////////////

    template<bool T_virtual = true>
    class RefCounted;

    template<>
    class RefCounted<true>
    {
    public:
        RefCounted() = default;
        virtual ~RefCounted() = default;

        void incRefCount() const { ++m_refCount; }
        uint32_t decRefCount() const { --m_refCount; return getRefCount(); }
        uint32_t getRefCount() const { return m_refCount.load(); }

    private:
        mutable std::atomic<uint32_t> m_refCount{ 0 };
    };

    template<>
    class RefCounted<false>
    {
    public:
        RefCounted() = default;
        ~RefCounted() = default;  // no virtual

        void incRefCount() const { ++m_refCount; }
        uint32_t decRefCount() const { --m_refCount; return getRefCount(); }
        uint32_t getRefCount() const { return m_refCount.load(); }

    private:
        mutable std::atomic<uint32_t> m_refCount{ 0 };
    };

    using RefCountedVirtual = RefCounted<true>;
    using RefCountedSimple = RefCounted<false>;


    /////////////////////////////////////////////////////////
    // Ref
    /////////////////////////////////////////////////////////

    template<typename T_type>
    class Ref
    {
        // NOTE KI have to disable asserts since it would require defining
        // also ref counted types in headers

        //static_assert(
        //    std::is_base_of_v<RefCounted<true>, T_type> ||
        //    std::is_base_of_v<RefCounted<false>, T_type>,
        //    "T_type must derive from RefCountedPolymorphic or RefCountedSimple");

        //static_assert(
        //    std::is_base_of_v<RefCounted<false>, T_type> && !std::is_polymorphic_v<T_type> ||
        //    std::is_base_of_v<RefCounted<true>, T_type>,
        //    "Class derived from RefCountedSimple must not have virtual functions"
        //    );

    public:
        Ref()
            : m_instance{ nullptr }
        {
        }

        ~Ref()
        {
            decRef();
        }

        Ref(std::nullptr_t)
            : m_instance{ nullptr }
        {
        }

        Ref(T_type* instance)
            : m_instance{ instance }
        {
            incRef();
        }

        Ref(const Ref<T_type>& o)
            : m_instance{ o.m_instance }
        {
            incRef();
        }

        template<typename T_type2>
        Ref(const Ref<T_type2>& o)
            : m_instance{ dynamic_cast<T_type*>(o.m_instance) }
        {
            incRef();
        }

        Ref(Ref<T_type>&& o)
            : m_instance{ o.m_instance }
        {
            o.m_instance = nullptr;
        }

        Ref& operator=(const Ref<T_type>& o)
        {
            if (this != &o) {
                decRef();
                m_instance = o.m_instance;
                incRef();
            }

            return *this;
        }

        Ref& operator=(Ref<T_type>&& o)
        {
            if (this != &o) {
                decRef();
                m_instance = o.m_instance;
                o.m_instance = nullptr;
            }

            return *this;
        }

        bool operator==(const Ref<T_type>& o) const noexcept
        {
            return m_instance == o.m_instance;
        }

        bool operator!=(const Ref<T_type>& o) const noexcept
        {
            return m_instance != o.m_instance;
        }

        T_type* operator->() const noexcept
        {
            //assert(m_instance != nullptr);
            return m_instance;
        }

        T_type& operator*() const noexcept
        {
            return *m_instance;
        }

        void reset() noexcept
        {
            decRef();
            m_instance = nullptr;
        }

        T_type* get() const noexcept
        {
            return m_instance;
        }

        explicit operator bool() const noexcept
        {
            return m_instance != nullptr;
        }

        template<typename T_type2>
        Ref<T_type2> as() const
        {
            return Ref<T_type2>{ dynamic_cast<T_type2*>(m_instance) };
        }

        template<typename... T_args>
        static Ref<T_type> create(T_args&& ... args)
        {
            return Ref<T_type>{ new T_type(std::forward<T_args>(args) ...) };
        }

    private:
        void incRef()
        {
            if (m_instance) {
                m_instance->incRefCount();
            }
        }

        void decRef()
        {
            if (m_instance) {
                if (m_instance->decRefCount() == 0) {
                    delete m_instance;
                    m_instance = nullptr;
                }
            }
        }

        template<class T_type2>
        friend class Ref;

    private:
        T_type* m_instance;
    };
}
