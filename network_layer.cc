//network_layer.cc
//Made by Michiel van der Bijl
//Bachelor thesis project 2025 Leiden University

#include "network_layer.h"
#include "engine.h"

using json = nlohmann::json;

clientLayer::clientLayer(int thisDevice)
  : client(thisDevice)
{
  //no further initialization needed
}//clientLayer