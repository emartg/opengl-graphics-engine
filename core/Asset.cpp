/*
* Asset.h
* This file implements the Asset class, which is an abstract base class
* to manage an asset meant to be used by the engine.
*/

#include "Asset.h"

// Static Private Attributes
// -------------------------
std::uint32_t Asset::nAssets{}; // initialize the number of assets in the scene to 0