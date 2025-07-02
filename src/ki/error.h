#pragma once

#include <string>
#include <system_error>
#include <typeinfo>

// https://stackoverflow.com/questions/41718912/is-there-any-exception-with-error-code-in-stl
namespace ki
{
    struct ErrorCode
    {
        enum Enum
        {
            // TODO KI error codes
            // FORMAT:
            // [subsystem]_[topic]_[code]
            error_1 = 101,
            error_2 = 102,
            error_3 = 103,

            scene = 200,
            scene_type_not_supported = 201
        };
    };

    class ErrorCategory
        : public std::error_category
    {
        using Base = std::error_category;
    public:
        auto name() const noexcept
            -> char const*
            override
        {
            return "Error";
        }

        auto default_error_condition(int const code) const noexcept
            -> std::error_condition
            override
        {
            (void)code; return {};
        }

        auto equivalent(int const code, std::error_condition const& condition) const noexcept
            -> bool
            override
        {
            (void)code; (void)condition; return false;
        }

        // The intended functionality of this func is pretty unclear.
        // It apparently can't do its job (properly) in the general case.
        auto equivalent(std::error_code const& code, int const condition) const noexcept
            -> bool
            override
        {
            return Base::equivalent(code, condition);
        }

        auto message(int const condition) const
            -> std::string
            override
        {
            return "ERROR: code=" + std::to_string(condition);
        }

        constexpr
            ErrorCategory() : Base{} {}
    };

    auto error_category()
        -> ErrorCategory const&
    {
        static ErrorCategory s_instance;
        return s_instance;
    }

    class Error
        : public std::system_error
    {
        using Base = std::system_error;
    public:
        auto error_code() const
            -> ErrorCode::Enum
        {
            return static_cast<ErrorCode::Enum>(code().value());
        }

        Error(ErrorCode::Enum const code)
            : Base{ code, error_category() }
        {
        }

        Error(ErrorCode::Enum const code, std::string const& description)
            : Base{ code, error_category(), description }
        {
        }
    };
}
