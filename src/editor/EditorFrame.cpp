#include "EditorFrame.h"

#include <math.h>

EditorFrame::EditorFrame(Window& window)
	: Frame(window)
{
}

void EditorFrame::draw(const RenderContext& ctx)
{
	// render your GUI
	ImGui::Begin("Triangle Position/Color");
	static float rotation = 0.0;
	ImGui::SliderFloat("rotation", &rotation, 0, 2);
	static float translation[] = { 0.0, 0.0 };
	ImGui::SliderFloat2("position", translation, -1.0, 1.0);
	static float color[4] = { 1.0f,1.0f,1.0f,1.0f };
	// pass the parameters to the shader
//	triangle_shader.setUniform("rotation", rotation);
//	triangle_shader.setUniform("translation", translation[0], translation[1]);
	// color picker
	ImGui::ColorEdit3("color", color);
	// multiply triangle's color with this color
	//triangle_shader.setUniform("color", color[0], color[1], color[2]);
	ImGui::End();
}

