#pragma once

namespace ReversibleHooks {
enum class eOverallState {
        Unknown,     //!< This category has no items and no subcategories, hence it has no state
        Mixed,       //!< Some hooks are redirected to GTA, some to Our code, and some are unhooked
        AllUnhooked, //!< All hooks are unhooked
        AllGta,      //!< All hooks are redirected to GTA
        AllOur,      //!< All hooks are redirected to Our code
    };
};
