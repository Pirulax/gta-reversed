#include "StdInc.h"
#include "Base.h"

fs::path notsa::GetSourceCodeBasePath() {
    return fs::path(__FILE__).parent_path();
}
