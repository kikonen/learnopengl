#include "EditorWindow.h"

#include "Engine.h"

EditorWindow::EditorWindow(Engine& engine)
	: GuiWindow(engine)
{
}

void EditorWindow::draw(const RenderContext& ctx)
{
	// render your GUI
	ImGui::Begin("Demo window");
	ImGui::Button("Hello!");
	ImGui::End();

	// Render dear imgui into screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

