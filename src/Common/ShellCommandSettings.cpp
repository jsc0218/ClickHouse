#include <base/EnumReflection.h>
#include <Common/Exception.h>
#include <Common/ShellCommandSettings.h>

#include <Poco/String.h>

namespace DB
{

namespace ErrorCodes
{
    extern const int BAD_ARGUMENTS;
}

ExternalCommandStderrReaction parseExternalCommandStderrReaction(const std::string & config)
{
    auto reaction = magic_enum::enum_cast<ExternalCommandStderrReaction>(Poco::toUpper(config));
    if (!reaction)
        throw Exception(
            ErrorCodes::BAD_ARGUMENTS,
            "Unknown stderr_reaction: {}. Possible values are 'none', 'log', 'log_first', 'log_last' and 'throw'",
            config);

    return *reaction;
}

}
