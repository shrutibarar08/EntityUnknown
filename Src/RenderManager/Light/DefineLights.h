#pragma once

#include "SystemManager/Registry/RegistryLight.h"

#include "DirectionalLight/DirectionalLight.h"
#include "SpotLight/SpotLight.h"
#include "PointLight/PointLight.h"

REGISTER_LIGHT(DirectionalLight);
REGISTER_LIGHT(SpotLight);
REGISTER_LIGHT(PointLight);
