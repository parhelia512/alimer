// For conditions of distribution and use, see copyright notice in License.txt

#pragma once

#include "../Base/String.h"
#include "../Base/Vector.h"
#include "../Base/WString.h"

namespace Alimer
{

/// Parse arguments from the command line. First argument is by default assumed to be the executable name and is skipped.
ALIMER_API const Vector<String>& ParseArguments(const String& cmdLine, bool skipFirstArgument = true);
/// Parse arguments from the command line.
ALIMER_API const Vector<String>& ParseArguments(const char* cmdLine);
/// Parse arguments from a wide char command line.
ALIMER_API const Vector<String>& ParseArguments(const WString& cmdLine);
/// Parse arguments from a wide char command line.
ALIMER_API const Vector<String>& ParseArguments(const wchar_t* cmdLine);
/// Parse arguments from argc & argv.
ALIMER_API const Vector<String>& ParseArguments(int argc, char** argv);
/// Return previously parsed arguments.
ALIMER_API const Vector<String>& Arguments();

}
