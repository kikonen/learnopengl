#pragma once

#include <glm/glm.hpp>

#include "asset/Material.h"
#include "Node.h"

class Sprite : public Node
{
public:
	static NodeType* getNodeType(
		const Assets& assets, 
		const std::string& path,
		const std::string& normalMapPath);

	Sprite(NodeType* type, glm::vec2 size);
	~Sprite();

private:
	static Material* getMaterial(
		const Assets& assets, 
		const std::string& path,
		const std::string& normalMapPath);

	static Mesh* getMesh(
		const Assets& assets, 
		Material* material);

public:
	const glm::vec2 size;
};

