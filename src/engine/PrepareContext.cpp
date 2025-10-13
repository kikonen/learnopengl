#include "PrepareContext.h"

#include "util/thread.h"

PrepareContext::PrepareContext(
    Engine& engine)
    : BaseContext{ engine }
{
    //ASSERT_RT();
}
