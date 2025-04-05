#include "StdInc.h"

namespace details {
void* CPtrNodeDoubleLink__operator_new(size_t sz) {
    assert(sz == sizeof(CPtrNodeDoubleLink<void*>));
    return CPools::GetPtrNodeDoubleLinkPool()->New();
}

void CPtrNodeDoubleLink__operator_delete(void* data, size_t sz) {
    assert(sz == sizeof(CPtrNodeDoubleLink<void*>));
    CPools::GetPtrNodeDoubleLinkPool()->Delete(reinterpret_cast<CPtrNodeDoubleLink<void*>*>(data));
}
};
