#pragma once
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <iostream>
#include <vector>

#include "Clever/WorldManager/MeshData.h"


std::pair<std::vector<Vertex>, std::vector<uint16_t>> loadModel(std::string modelFilePath);