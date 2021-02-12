#include "EditorWindow.h"

#include "Engine.h"

EditorWindow::EditorWindow(Engine& engine)
	: GuiWindow(engine)
{
}

void EditorWindow::draw(const RenderContext& ctx)
{
	ImGui::Begin("Demo window");
	ImGui::Button("Hello!");
	ImGui::End();
}

