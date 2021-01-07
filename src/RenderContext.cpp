#include "RenderContext.h"

RenderContext::RenderContext(
	const Engine& engine, 
	const float dt, 
	const glm::mat4& view, 
	const glm::mat4& projection,
	const Light* light)
	: engine(engine),
	dt(dt),
	view(view),
	projection(projection),
	projected(projection * view),
	light(light)
{
}
